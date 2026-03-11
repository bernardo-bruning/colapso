#include <idt.h>

idt_entry_t idt[256];
idt_ptr_t   idt_ptr;

extern void idt_load(uint32_t);
extern void isr0(); extern void isr3();
extern void irq0(); extern void irq1();
/* Adicionaremos um handler generico para o Disco (IRQ 14 -> INT 46) */
extern void irq14(); 

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

void pic_remap() {
    outb(0x20, 0x11); outb(0xA0, 0x11);
    outb(0x21, 0x20); outb(0xA1, 0x28);
    outb(0x22, 0x04); outb(0xA2, 0x02);
    outb(0x23, 0x01); outb(0xA3, 0x01);
    outb(0x21, 0x0);  outb(0xA1, 0x0);
}

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
}

void init_idt() {
    idt_ptr.limit = sizeof(idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint32_t)&idt;

    pic_remap();

    /* Preenche todas as portas com zero por seguranca */
    for(int i=0; i<256; i++) {
        idt_set_gate(i, 0, 0, 0); 
    }

    /* Exceções */
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);

    /* IRQs (Hardware) */
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E); /* Timer */
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E); /* Teclado */
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E); /* Disco IDE Primario! */

    idt_load((uint32_t)&idt_ptr);
    inb(0x60); 
}

void isr_handler(uint32_t int_no) {
    (void)int_no;
    uint16_t* vga = (uint16_t*) 0xB8000;
    vga[0] = (uint16_t) 'F' | (uint16_t) 0x4F << 8; 
}

void irq_handler(uint32_t irq_no) {
    /* Se for teclado (IRQ 33) */
    if (irq_no == 33) {
        extern void keyboard_handler();
        keyboard_handler();
    }
    
    /* Se for o Disco (IRQ 46), apenas ignoramos e enviamos o EOI para liberar o barramento */

    /* EOI (Avisa aos PICs que terminamos) */
    if (irq_no >= 40) outb(0xA0, 0x20); /* PIC Escravo */
    outb(0x20, 0x20);                   /* PIC Mestre */
}
