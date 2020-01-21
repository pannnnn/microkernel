.global swi
.type   swi, %function
swi:
    @ software interrupt, trapped into swi_handler (enter_kernel)
    swi
    @ return back to caller
    mov     pc, lr

.global enter_kernel
.type   enter_kernel, %function
enter_kernel:
    @ go into system mode with IRQ and FIQ off | 0xDF = 0b11011111
    msr     cpsr_c, #0xDF
    @ store r0-r12 and lr in the user stack
    stmfd   sp!, {r0-r12, lr}
    @ store user stack pointer in unbanked register r1
    mov     r1, sp

    @ go into supervisor mode by only changing mode bits | reference: A4-78
    mrs     r2, cpsr
    bic     r2, r2, #0x1F
    orr     r2, r2, #0x13
    msr     cpsr_c, r2
    @ store user state cpsr(spsr) and swi lr in the user stack
    mrs     r2, spsr
    stmfd   r1!, {r2, lr}
    @ r2 holds the address of Args, let it point to the args we want to pass from swi call
    ldmfd   sp!, {r2}
    str     r0, [r2]
    mov     r0, r1
    mov     r1, r2
    @ load kernel state registers
    ldmfd   sp!, {r2-r12, pc}

.global leave_kernel
.type   leave_kernel, %function
leave_kernel:
    @ store kernel state registers in the kernel stack
    stmfd   sp!, {r1-r12, lr}
    @ restore user state spsr and lr back from user stack
    ldmfd   r0!, {r1, lr}
    msr     spsr, r1
    @ go into system mode | 0xDF = 0b11011111
    msr     cpsr_c, #0xDF
    @ restore user stack pointer from r0
    mov     sp, r0
    @ restore r0-r12 and lr from user stack
    ldmfd   sp!, {r0-r12, lr}
    @ go into supervisor mode | 0xD3 = 0b11010011
    msr     cpsr_c, #0xD3
    @ branch to lr and go into user mode | reference: A2-55
    movs    pc, lr