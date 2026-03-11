/* kernel.ld - Layout para Kernel Binário Flat */
OUTPUT_FORMAT("binary")

SECTIONS
{
    . = 0x1000;

    .text :
    {
        /* Garante que o kernel_entry venha primeiro no arquivo */
        build/kernel_entry.o(.text)
        *(.text)
    }

    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}
