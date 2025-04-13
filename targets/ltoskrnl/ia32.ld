OUTPUT_FORMAT(elf32-i386)

ENTRY(FwBoot)


SECTIONS
{
    . = 1M;

    .interp :
    {
        *(.multiboot)
        *(.interp)
    }

	.text ALIGN(4K) :
	{
        *(.multiboot)
		*(.text)
	}

	.rodata ALIGN(4K) :
	{
	    *(.multiboot)
	    *(.rodata.str1.1)
		*(.rodata)
	}

	.data ALIGN(4K) :
	{
	    *(.multiboot)
		*(.data)
	}

	.bss ALIGN(4K) :
	{
		*(COMMON)
		*(.bss)
	}
}