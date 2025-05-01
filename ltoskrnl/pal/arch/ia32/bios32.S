
.section .text

#define REBASE(x) (((x) - PaliX86BiosSwitch) + 0x7c00)

.global PaliX86BiosSwitch
PaliX86BiosSwitch:

    pushl %ebp
    movl %esp, %ebp

    movl 8(%ebp), %eax
    movb %al, REBASE(InterruptNumber)
    movl 12(%ebp), %edi
    movl %edi, REBASE(EdiBuffer)

    pushal
    pushfl

    movl %cr3, %eax
    movl %eax, REBASE(Cr3Buffer)

    movl %cr0, %eax
    movl %eax, REBASE(Cr0Buffer)

    movl %esp, REBASE(EspBuffer)
    sidtl REBASE(IdtrBuffer)
    sgdtl REBASE(GdtrBuffer)

    movl %cr0, %eax
    andl $~(1 << 31), %eax
    movl %eax, %cr0

    xorl %eax,%eax

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

    movl 4(%edi), %esi
    movl 8(%edi), %ebp
    movl 12(%edi), %esp
    movl 16(%edi), %ebx
    movl 20(%edi), %edx
    movl 24(%edi), %ecx
    movl 28(%edi), %eax

    movw 32(%edi), %gs
    movw 34(%edi), %fs
    movw 36(%edi), %es
    movw 38(%edi), %ds
    movw 40(%edi), %ss

    pushl 42(%edi)
    popfl

    movl (%edi), %edi

    int $0x00
InterruptNumber = . - 1

    cli
    pushl %edi
    mov REBASE(EdiBuffer), %edi
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
    movw %ss, 40(%edi)

    pushfl
    pop 42(%edi)

    movl %cr0, %eax
    orl $1, %eax
    movl %eax, %cr0

    ljmp $0x08, $REBASE(ProtectedMode)

ProtectedMode:
    .code32

    lidtl REBASE(IdtrBuffer)
    lgdtl REBASE(GdtrBuffer)

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    movl REBASE(EspBuffer), %esp
    movl REBASE(Cr3Buffer), %eax
    movl %eax, %cr3
    movl REBASE(Cr0Buffer), %eax
    movl %eax, %cr0

    popfl
    popal

    leave
    ret

RealModeIdt:
    .word 0x03FF
    .long 0x00000000

GdtrBuffer: .space 6
IdtrBuffer: .space 6
Cr3Buffer: .space 4
Cr0Buffer: .space 4
EspBuffer: .space 4
EdiBuffer: .space 4

PaliX86StateStorage: .space 100
PaliX86DataStorage: .space 4096

.global PaliX86BiosSwitchEnd
PaliX86BiosSwitchEnd:
