#ifndef IDT_H
#define IDT_H

#include <stdint.h>

/* Estrutura de um portão da IDT (Interrupt Gate) */
struct idt_entry_struct {
    uint16_t base_low;  /* Os 16 bits inferiores do endereço da ISR */
    uint16_t sel;       /* Seletor de segmento de código (GDT) */
    uint8_t  always0;   /* Sempre zero */
    uint8_t  flags;     /* Flags (P, DPL, Type) */
    uint16_t base_high; /* Os 16 bits superiores do endereço da ISR */
} __attribute__((packed));

/* Estrutura do ponteiro da IDT para o comando 'lidt' */
struct idt_ptr_struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

typedef struct idt_entry_struct idt_entry_t;
typedef struct idt_ptr_struct idt_ptr_t;

/* Inicializa a IDT */
void init_idt();

/* Define um portão na IDT */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

#endif
