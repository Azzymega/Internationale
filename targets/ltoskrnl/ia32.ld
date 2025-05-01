OUTPUT_FORMAT(elf32-i386)
ENTRY(PalBoot)

SECTIONS
{
    . = 1M;

    .multiboot : {
        *(.multiboot.data)
        *(.multiboot.text)
    }

    . += 0x80000000;

    PalKernelImageStart = .;

    .text ALIGN(4K) : AT(ADDR(.text) - 0x80000000) {
        *(.text)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - 0x80000000) {
        *(.rodata)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - 0x80000000) {
        *(.data)
    }

    .bss ALIGN(4K) : AT(ADDR(.bss) - 0x80000000) {
        *(COMMON)
        *(.bss)
        *(.bootstrap_stack)
    }

    PalKernelImageEnd = .;
}