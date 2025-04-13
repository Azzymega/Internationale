
.section .text

# LOADING BASE - 0x7C00
# CR0 - 0x520
# CR3 - 0x500
# IDTR - 0x530
# GDTR - 0x540
# ESP - 0x550
# EDI - 0x560

#define REBASE(x) (((x) - PaliX86BiosSwitch) + 0x7c00)

.global PaliX86BiosSwitch
PaliX86BiosSwitch:

    pushl %ebp
    movl %esp, %ebp

    movl 8(%ebp), %eax
    movb %al, REBASE(InterruptNumber)
    movl 12(%ebp), %edi

    pushal
    pushfl

    movl %cr3, %eax
    movl %eax, 0x500
    movl %cr0, %eax
    movl %eax, 0x520
    movl %esp, 0x550
    movl %edi, 0x560
    sidtl 0x530
    sgdtl 0x540

    movl %cr0, %eax
    andl $~(1 << 31), %eax
    movl %eax, %cr0

    xorl %eax, %eax
    movl %eax, %cr3

    ljmp $0x30, $REBASE(Reload16Bit)

Reload16Bit:

    .code16

    mov %cr0, %eax
    and $~1, %eax
    mov %eax, %cr0

    lidtw REBASE(RealModeIdt)
    ljmp $0x0, $REBASE(RealMode)

RealMode:

    xor %eax, %eax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %ss
    mov %ax, %fs
    mov %ax, %gs

    mov 0x560, %edi

    movl 4(%edi), %esi
    movl 8(%edi), %ebp
    movw $0xF000, %sp
    movl 16(%edi), %ebx
    movl 20(%edi), %edx
    movl 24(%edi), %ecx
    movl 28(%edi), %eax

    movw 32(%edi), %gs
    movw 34(%edi), %fs
    movw 36(%edi), %es
    movw 38(%edi), %ds

    pushl 40(%edi)
    popfl

    movl (%edi), %edi

    clc
    cld
    int $0x00
InterruptNumber = . - 1

    pushl %edi
    mov 0x560, %edi
    pop (%edi)

    movl %esi, 4(%edi)
    movl %ebp, 8(%edi)
    movl %esp, 12(%edi)
    movl %ebx, 16(%edi)
    movl %edx, 20(%edi)
    movl %ecx, 24(%edi)
    movl %eax, 28(%edi)

    movw %gs, 32(%edi)
    movw %fs, 34(%edi)
    movw %es, 36(%edi)
    movw %ds, 38(%edi)

    pushfl
    pop 40(%edi)

    movl %cr0, %eax
    orl $1, %eax
    movl %eax, %cr0

    ljmp $0x08, $REBASE(ProtectedMode)

ProtectedMode:
    .code32

    lidtl 0x530
    lgdtl 0x540

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    movl 0x550, %esp
    movl 0x500, %eax
    movl %eax, %cr3
    movl 0x520, %eax
    movl %eax, %cr0

    popfl
    popal

    leave
    ret

RealModeIdt:
    .word 0x03FF
    .long 0x00000000

.global PaliX86BiosSwitchEnd
PaliX86BiosSwitchEnd:
