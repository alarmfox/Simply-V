# Simply-V FreeRTOS

> [!WARNING]
> `freertos` expects the `source $ROOT_DIR/settings.sh` to be sourced in the current shell.

The FreeRTOS integration in Simply-V works by linking applications against the FreeRTOS kernel. The
directory structure is:
- kernel: the FreeRTOS-kernel. Provides the `tasks.c` `heap_*.c` `queue.c` etc. and platform specific ports
- app: user applications directory. Each application must live in each own directory. The application
    directory is structured like this:
    - app/\<app-name-1\>/
    - app/\<app-name-2\>/
- FreeRTOSConfig.h: FreeRTOS configuration.
- startups.S: startup file to setup `freertos_risc_v_trap_handler`, `_reset_handler` and jump to main.

The provided `Makefile` compiles all the applications under `app/` as separated elf in `build/<app-name>/<app-name>.elf.`

## FreeRTOS integration

### Configuration
The `FreeRTOSConfig.h` contains configuration for the FreeRTOS-Kernel and the applications. Based 
on the platform capabilities user can tune parameters like `configTOTAL_HEAP_SIZE, configMINIMAL_STACK_SIZE`.

Some usefule variables to enable extra debugging are:
```c
#define configUSE_MALLOC_FAILED_HOOK          1
#define configCHECK_FOR_STACK_OVERFLOW        2
```

When enabled they mandate to provide `void vApplicationMallocFailedHook(void)` and `
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)` functions that will get 
called when the malloc fails or when the stack overflows happens.

#### Heap configuration
FreeRTOS has different heap implemenations, you can choose the heap implementation by changing the 
include file in the `Makefile` (default is `heap_1.c`):

```Makefile
KERNEL_SRCS  := $(wildcard $(KERNEL_DIR)/*.c) \
                 $(wildcard $(KERNEL_DIR)/portable/GCC/RISC-V/*.c) \
                 $(wildcard $(KERNEL_DIR)/portable/MemMang/heap_1.c) \
                 $(wildcard $(KERNEL_DIR)/portable/GCC/RISC-V/*.S)
```

To change to `heap_4.c`, just provide the `heap_4.c` file:

```Makefile
KERNEL_SRCS  := $(wildcard $(KERNEL_DIR)/*.c) \
                 $(wildcard $(KERNEL_DIR)/portable/GCC/RISC-V/*.c) \
                 $(wildcard $(KERNEL_DIR)/portable/MemMang/heap_4.c) \
                 $(wildcard $(KERNEL_DIR)/portable/GCC/RISC-V/*.S)
```

### System Tick Managemnet
For now the SoC does not support a system timer. The SystemTick needs to be updated using a custom interrupt 
handler. On the timer ISR we need to implement the logic decribed in the picture from [here](https://rcc.freertos.org/Documentation/02-Kernel/05-RTOS-implementation-tutorial/02-Building-blocks/03-The-RTOS-tick).

[![RTOS Tick](https://rcc.freertos.org/media/2018/TickISR.gif)](https://rcc.freertos.org/media/2018/TickISR.gif)

This is done in the `vExternalTickIncrement()`:
```c
static void vExternalTickIncrement() {
  BaseType_t xSwitchRequired;

  // Clear interrupt flag
  xlnx_tim_clear_int(&timer);

  // Increment RTOS tick count
  xSwitchRequired = xTaskIncrementTick();

  // If a task was unblocked, yield to it
  if (xSwitchRequired != pdFALSE) {
    portYIELD_FROM_ISR(xSwitchRequired);
  }
}
```

### Trap vector configuration
The FreeRTOS integration is developed according to [this documentation page]([https://www.freertos.org/Using-FreeRTOS-on-RISC-V).

The `startup.S` installs the `freertos_risc_v_trap_handler` and configures the reset handler. 

The `_reset_handler` performs the following actions:
- reset registers
- install the trap handler (vectored mode)
- initializes the stack to `__stack_start`
- jumps into main

FreeRTOS vectored mode wants every entry of the vector table to point at the `freertos_risc_v_trap_handler`

> Note: If the RISC-V chip uses a vectored interrupt controller then install freertos_risc_v_trap_handler() 
as the handler for each vector.

```asm
.extern freertos_risc_v_trap_handler
.section .vector_table, "ax"
.align 4
.option norvc
/* In vectored mode all entries must go to -> freertos_risc_v_trap_handler */
/* https://www.freertos.org/Using-FreeRTOS-on-RISC-V#interrupt-system-stack-setup */
.rept 32
jal x0, freertos_risc_v_trap_handler
.endr
```

```asm
/* Set mtvec trap freertos_risc_v_trap_handler VECTORED mode. */
la a0, _vector_table_start  # Load vector table base address
li a1, 1                    # Set vectored mode bit
or a1, a1, a0
csrw mtvec, a1              # Commit on mtvec register
```

The timer peripheral is "TIM0" and it's managed through the PLIC. The "custom" interrupt 
handler can be specified by defining a `void freertos_risc_v_application_interrupt_handler(uint32_t mcause);` 
function (defined as `weak` by the FreeRTOS kernel) which will be called upon any external interrupt.
For example, if the SystemTimer has `interrupt_id = 2`, the trap handler would be like:
```c
void freertos_risc_v_application_interrupt_handler(uint32_t mcause) {
  (void) mcause;

  uint32_t interrupt_id = plic_claim();

  switch (interrupt_id) {
    case 0x2:
      vExternalTickIncrement();
      break;
    // other interrupts...
    default:
      break;
  }
  plic_complete(interrupt_id);
}
```

The trap stack needs to be configured. FreeRTOS supports reusing the initial stack as `xISRStack`.
This is achieved by defining the `__freertos_irq_stack_top` as equal to the `__stack_top` in the 
linkerscript as mentioned [here](https://www.freertos.org/Using-FreeRTOS-on-RISC-V#interrupt-system-stack-setup)

```
__bram_end        = ORIGIN(BRAM) + LENGTH(BRAM);
__stack_top       = __bram_end - 0x10;
__stack_size      = 0x1000; /* 4k */
__stack_bottom    = __stack_top - __stack_size;

/* https://www.freertos.org/Using-FreeRTOS-on-RISC-V#interrupt-system-stack-setup */
__freertos_irq_stack_top = __stack_top;
```

> [!Note]
> Make sure the `configISR_STACK_SIZE_WORDS` is not defined in the `FreeRTOSConfig.h`

### Timer configuration
The SoC does not have the MTIME so we need to start the timer defining the `void vPortSetupTimerInterrupt(void)`.
The function enables the PLIC, configures, starts and enables timer interrupt. Since we do not have 
the MTIME, the timer interrupt will be just enabled as a normal external interrupt (MEI bit in MIE):

```c
void vPortSetupTimerInterrupt(void) {
  // configure and init PLIC
  plic_init();
  // other functions omitted...

  // create a Timer Instance
  xlnx_tim_init(&timer);
  // validate and enable timer interrupts... (not shown here)

  // start the timer
  ret = xlnx_tim_start(&timer);
  if (ret != UNINASOC_OK) {
    printf("Cannot start timer\r\n");
    return;
  }

  // Enable local interrupts lines
  // MEI (External Interrupt)
  __asm volatile ( "csrs mie, %0" ::"r" ( 0x800 ) );
}
```

### Linkerscript

The `linker.ld`  defines `.bss` and  `.data` sections, but relies on the
`${ROOT_DIR}/sw/SoC/common/UninaSoC.ld` for memory and peripheral symbols. The linkerscript asserts
non overlapping memory sections and defines the `__stack_start` and `__stack_size` (4K by default).

## Usage
The users are expected to run the application in from the `${ROOT_DIR}/sw/SoC/project/freertos` directory. 
Users can either build all applications under `app/` (`make`) or a single application (`make <app-name>`). 
Use `DEBUG=1` to enable debug symbols (add `-g` to CFLAGS).

> [!Note]
> Toolchain, architecture, ABI etc. are managed from `${ROOT_DIR}/sw/SoC/common/config.mk`. You can define your 
custom RV_PREFIX by running something `make RV_PREFIX=<path-to-prefix> XLEN=<32-64>`

```sh
# build all applications in app/
make DEBUG=1

# or build single application in app/
make DEBUG=1 timed-prod-cons
```

Load the application on the target:
```sh
?? () at startup.S:9
9           jal x0, freertos_risc_v_trap_handler
Loading section .vector_table, size 0x82 lma 0x0
Loading section .text, size 0x8d60 lma 0x100
Loading section .rodata, size 0x214 lma 0x8e60
Loading section .srodata, size 0x18 lma 0x9078
Loading section .data, size 0xc lma 0xa660
Loading section .sdata, size 0x10 lma 0xa66c
Start address 0x00000100, load size 36906
Transfer rate: 54 KB/sec, 4613 bytes/write.
(gdb) b main
Breakpoint 1 at 0x548: file app/timed-prod-cons/main.c, line 153.
(gdb) c
Continuing.

Breakpoint 1, main () at app/timed-prod-cons/main.c:153
153         uninasoc_init();
```

## Development
To ease the development process, users can use the following `.clangd` file to get completions in 
editors like VSCode.

```
CompileFlags: 
  Add:
    - "-Ikernel/include/"
    - "-Ikernel/portable/GCC/RISC-V/"
    - "-Ikernel/portable/GCC/RISC-V/chip_specific_extensions/RISCV_no_extensions"
    - "-Ikernel/include/"
    - "-I../../lib/uninasoc/inc/"
    - "-I../../lib/tinyio/inc/"
```

More advanced tools can require a `compile_commands.json` which can be generated using 
[bear](https://github.com/rizsotto/Bear):

```sh
make clean
bear -- make DEBUG=1
```

And include the `compile_commands.json` in the `.clangd` (this should be default):

```
CompileFlags: 
  CompilationDatabase: .
```
