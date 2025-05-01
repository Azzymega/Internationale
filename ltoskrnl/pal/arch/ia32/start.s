.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot.data, "aw"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 1048576
stack_top:

.section .bss, "aw", @nobits
.align 4096
boot_page_directory:
.skip 4096
boot_page_table1:
.skip 4096

.section .multiboot.text, "a"
.global PalBoot
.type PalBoot, @function
PalBoot:
    movl $(boot_page_table1 - 0x80000000), %edi
    movl $0, %esi
    movl $1023, %ecx

1:
    movl %esi, %edx
    orl $0x003, %edx
    movl %edx, (%edi)
    addl $4096, %esi
    addl $4, %edi
    loop 1b

    movl $(0x000B8000 | 0x003), boot_page_table1 - 0x80000000 + 1023 * 4

    movl $(boot_page_table1 - 0x80000000 + 0x003), boot_page_directory - 0x80000000 + 0
    movl $(boot_page_table1 - 0x80000000 + 0x003), boot_page_directory - 0x80000000 + 512 * 4

    movl $(boot_page_directory - 0x80000000), %ecx
    movl %ecx, %cr3

    movl %cr0, %ecx
    orl $0x80010000, %ecx
    movl %ecx, %cr0

    lea 4f, %ecx
    jmp *%ecx

.section .text
4:
    mov $stack_top, %esp
    add $0x80000000, %ebx
    push %ebx
    push %eax
    call PalInitialize

    cli
1:  hlt
    jmp 1b