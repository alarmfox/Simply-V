    /* ultra-minimal startup.S for RISC-V */
    .extern freertos_risc_v_trap_handler
    .section .vector_table, "ax"
    .align 4
    .option norvc
    /* In vectored mode all entries must go to -> freertos_risc_v_trap_handler */
    /* https://www.freertos.org/Using-FreeRTOS-on-RISC-V#interrupt-system-stack-setup */
    .rept 32
    jal x0, freertos_risc_v_trap_handler
    .endr

    .section .text.handlers
    .global _reset_handler
_reset_handler:
    /* Reset registers */
    mv ra, zero
    mv gp, zero
    mv tp, zero
    mv t0, zero
    mv t1, zero
    mv t2, zero
    mv s0, zero
    mv s1, zero
    mv a0, zero
    mv a1, zero
    mv a2, zero
    mv a3, zero
    mv a4, zero
    mv a5, zero
    mv a6, zero
    mv a7, zero
    mv s2, zero
    mv s3, zero
    mv s4, zero
    mv s5, zero
    mv s6, zero
    mv s7, zero
    mv s8, zero
    mv s9, zero
    mv s10, zero
    mv s11, zero
    mv t3, zero
    mv t4, zero
    mv t5, zero
    mv t6, zero

    /* Initialize stack pointer first */
    la    sp, __stack_top

    /* Set mtvec trap freertos_risc_v_trap_handler VECTORED mode. */
    la a0, _vector_table_start  # Load vector table base address
    li a1, 1                    # Set vectored mode bit
    or a1, a1, a0
    csrw mtvec, a1              # Commit on mtvec register

    /* Jump to C entry (_start). Keep interrupts disabled here; enable later when ready. */
    j     _start

_default_handler:
    /* simple infinite loop for unexpected traps */
    j     _default_handler

    .global _start
_start:
    /* call main(argc=0, argv=0) */
    mv    a0, zero
    mv    a1, zero
    call  main

    /* If main returns, hang */
1:  j 1b
