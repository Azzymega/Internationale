.macro ISR_NOERRCODE num
.global HaliIsr\num
HaliIsr\num:
    pushl $0
    pushl $\num
    jmp isr_common_stub
.endm

.macro ISR_ERRCODE num
.global HaliIsr\num
HaliIsr\num:
    pushl $\num
    jmp isr_common_stub
.endm

.macro IRQ num irq_num
.global HaliIrq\num
HaliIrq\num:
    pushl $0
    pushl $\irq_num
    jmp irq_common_stub
.endm

.global HaliDispatch
HaliDispatch:
    pushl $0xCAFE
    pushl $0xFE
    jmp irq_common_stub

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

.global HaliX86FlushGdt
HaliX86FlushGdt:
	mov $0x10,%ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	jmp $0x08,$FlushEnd
FlushEnd:
    ret

.global HaliX86IretJump
HaliX86IretJump:
    pop %eax

    pop %eax
    pop %ecx
    pop %edx

    pop %ebp
    pop %esp

    push %edx
    push %ecx
    push %eax

    iret

.global HaliX86SetCrs
HaliX86SetCrs:
    pop %eax

    pop %ecx
    mov %ecx, %cr3

    pop %ecx
    mov %ecx, %cr0

    pop %ecx
    mov %ecx, %cr4

    push %eax
    ret




isr_common_stub:
    push %esp
    push %eax
    push %ecx
    push %edx
    push %ebx
    push %ebp
    push %esi
    push %edi

    subl $108, %esp
    fnsave (%esp)

    push %ds
    push %es
    push %fs
    push %gs

    mov %cr0, %eax
    push %eax
    mov %cr2, %eax
    push %eax
    mov %cr3, %eax
    push %eax
    mov %cr4, %eax
    push %eax

    mov %dr0, %eax
    push %eax
    mov %dr1, %eax
    push %eax
    mov %dr2, %eax
    push %eax
    mov %dr3, %eax
    push %eax
    mov %dr6, %eax
    push %eax
    mov %dr7, %eax
    push %eax

    push %esp
    call HaliIsrHandler
    cmpl $1, %eax
    jl IsrUserRet

    IsrKernelRet:

    add $4, %esp
    pop %eax
    mov %eax, %dr7
    pop %eax
    mov %eax, %dr6
    pop %eax
    mov %eax, %dr3
    pop %eax
    mov %eax, %dr2
    pop %eax
    mov %eax, %dr1
    pop %eax
    mov %eax, %dr0
    pop %eax
    mov %eax, %cr4
    pop %eax
    mov %eax, %cr3
    pop %eax
    mov %eax, %cr2
    pop %eax
    mov %eax, %cr0
    pop %gs
    pop %fs
    pop %es
    pop %ds

    frstor (%esp)
    addl $108, %esp

    pop %edi
    pop %esi
    pop %ebp
    pop %ebx
    pop %edx
    pop %ecx
    pop %eax
    pop %esp

    iret

    IsrUserRet:
    rsm

irq_common_stub:
    push %esp
    push %eax
    push %ecx
    push %edx
    push %ebx
    push %ebp
    push %esi
    push %edi

    subl $108, %esp
    fnsave (%esp)

    push %ds
    push %es
    push %fs
    push %gs

    mov %cr0, %eax
    push %eax
    mov %cr2, %eax
    push %eax
    mov %cr3, %eax
    push %eax
    mov %cr4, %eax
    push %eax

    mov %dr0, %eax
    push %eax
    mov %dr1, %eax
    push %eax
    mov %dr2, %eax
    push %eax
    mov %dr3, %eax
    push %eax
    mov %dr6, %eax
    push %eax
    mov %dr7, %eax
    push %eax

    push %esp
    call HaliIrqHandler
    cmpl $1, %eax
    jl IrqUserRet

    IrqKernelRet:

    add $4, %esp
    pop %eax
    mov %eax, %dr7
    pop %eax
    mov %eax, %dr6
    pop %eax
    mov %eax, %dr3
    pop %eax
    mov %eax, %dr2
    pop %eax
    mov %eax, %dr1
    pop %eax
    mov %eax, %dr0
    pop %eax
    mov %eax, %cr4
    pop %eax
    mov %eax, %cr3
    pop %eax
    mov %eax, %cr2
    pop %eax
    mov %eax, %cr0
    pop %gs
    pop %fs
    pop %es
    pop %ds

    frstor (%esp)
    addl $108, %esp

    pop %edi
    pop %esi
    pop %ebp
    pop %ebx
    pop %edx
    pop %ecx
    pop %eax
    pop %esp

    iret

    IrqUserRet:
    rsm