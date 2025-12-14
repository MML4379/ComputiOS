ENTRY(kernel_entry)

SECTIONS
{
    /* Load kernel at physical (and virtual, for now) 0x00100000 */
    . = 0x00100000;

    .text :
    {
        /* 1st: our special .start section with kernel_entry */
        *(.start)

        /* then all other code */
        *(.text*)
    }

    .rodata :
    {
        *(.rodata*)
    }

    .data :
    {
        *(.data*)
    }

    .bss :
    {
        *(.bss*)
        *(COMMON)
    }
}
