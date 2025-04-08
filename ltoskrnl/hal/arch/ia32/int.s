.macro ISR_NOERRCODE num
.global HaliIsr\num
HaliIsr\num:
    cli
    pushl $0
    pushl $\num
    jmp isr_common_stub
.endm

.macro ISR_ERRCODE num
.global HaliIsr\num
HaliIsr\num:
    cli
    pushl $\num
    jmp isr_common_stub
.endm

.macro IRQ num irq_num
.global HaliIrq\num
HaliIrq\num:
    cli
    pushl $0
    pushl $\irq_num
    jmp irq_common_stub
.endm

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

isr_common_stub:
    subl $108, %esp
    fnsave (%esp)
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
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
    popa
    frstor (%esp)
    addl $108, %esp
    add $8, %esp
    iret

irq_common_stub:
    subl $108, %esp
    fnsave (%esp)
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
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
    popa
    frstor (%esp)
    addl $108, %esp
    add $8, %esp
    mov $0x20, %al
    out %al, $0x20
    cmpl $40, 8(%esp)
    jl eoi_done
    out %al, $0xA0
eoi_done:
    iret
