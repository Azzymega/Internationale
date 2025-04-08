
.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bss
_boot_stack_bottom:
.skip 1048576
_boot_stack_top:
.align 16

.section .text

.global HaliFlushGdt
HaliFlushGdt:
	mov $0x10,%ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	jmp $0x08,$FlushEnd
FlushEnd:
    ret

.global FwBoot
.type FwBoot, @function

FwBoot:
	mov $_boot_stack_top, %esp
	push %ebx
	push %eax

	call FwInitialize

	cli
1:	hlt
	jmp 1b
