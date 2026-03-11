/* bash.ld - Para o Bash em 0x20000 */
OUTPUT_FORMAT("binary")
ENTRY(_start)

SECTIONS
{
    . = 0x20000;

    .text :
    {
        *(.text)
    }

    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}
