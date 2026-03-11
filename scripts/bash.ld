/* bash.ld - Garante que o _start seja a primeira coisa executada no Bash */
OUTPUT_FORMAT("binary")
ENTRY(_start)

SECTIONS
{
    /* O Kernel carrega o Bash em 0x20000 */
    . = 0x20000;

    .text :
    {
        /* FORÇA o entry point a vir primeiro */
        build/bash_entry.o(.text)
        *(.text)
    }

    .rodata : { *(.rodata) }
    .data : { *(.data) }
    .bss : { *(.bss) }
}
