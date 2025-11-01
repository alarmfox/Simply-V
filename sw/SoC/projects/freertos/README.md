# Simply-V FreeRTOS

The FreeRTOS integration in Simply-V works by linking applications against the FreeRTOS kernel. The
directory structure is:
- kernel: the FreeRTOS-kernel. Provides the `tasks.c` `heap_*.c` `queue.c` etc. and platform specific ports
- app: user applications directory. Each application must live in each own directory. The application
    directory is structured like this:
    - app/<app-name-1>/
    - app/<app-name-2>/
- FreeRTOSConfig.h: FreeRTOS configuration.
- startups.S: startup file to setup `freertos_riscv_trap_handler`, `_reset_handler` and jump to main.

The provided `Makefile` compiles all the applications under `app/` as separated elf in `build/<app-name>/<app-name>.elf.`

## Running an application
The users are expected to run the application in from the `${ROOT_DIR}/sw/SoC/project/freertos` directory. First, 
build all applications:

```sh
make
```

Then load the applications on the target:
```sh
(gdb) file build/basic-two-tasks/basic-two-tasks.elf
A program is being debugged already.
Are you sure you want to change the file? (y or n) y
Reading symbols from build/basic-two-tasks/basic-two-tasks.elf...
(gdb) load
Loading section .vector_table, size 0x80 lma 0x0
Loading section .text, size 0xb560 lma 0x100
Loading section .data, size 0x24 lma 0xc7fc
Loading section .sdata, size 0x10 lma 0xc820
Loading section .rodata, size 0x2b0 lma 0xc830
Loading section .srodata, size 0x18 lma 0xcae0
Start address 0x00000100, load size 47324
Transfer rate: 70 KB/sec, 5915 bytes/write.
(gdb) b main
Breakpoint 1 at 0x548: file app/basic-two-tasks/main.c, line 129.
(gdb) c
Continuing.

Breakpoint 1, main () at app/basic-two-tasks/main.c:129
129         uninasoc_init();
```

## Experimental `uninasoc` linking
The linking with `uninasoc` is experimental and can lead to bigger executable size.

## Development
To ease the development process, users can use the provided `.clangd` file to get completions in 
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
