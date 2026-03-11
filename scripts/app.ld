/* app.ld - Para Aplicativos em 0x40000 */
OUTPUT_FORMAT("binary")
ENTRY(_start)

SECTIONS
{
    . = 0x40000;

    .text :
    {
        *(.text)
    }

    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}
