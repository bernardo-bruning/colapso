#include <idt.h>

idt_entry_t idt[256];
idt_ptr_t   idt_ptr;

extern void idt_load(uint32_t);
extern void isr0(); extern void isr3();
extern void irq0(); extern void irq1(); extern void irq14(); 
extern void isr80();

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static void serial_write(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        outb(0x3F8, (uint8_t)str[i]);
    }
}

static void serial_write_hex8(uint8_t value) {
    static const char hex[] = "0123456789ABCDEF";
    char msg[3];
    msg[0] = hex[(value >> 4) & 0x0F];
    msg[1] = hex[value & 0x0F];
    msg[2] = '\0';
    serial_write(msg);
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

    for(int i=0; i<256; i++) idt_set_gate(i, 0, 0, 0); 

    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    
    /* Syscall Interruption (int 0x80) - Gate Type 0xEE (Allow User Space Calls) */
    idt_set_gate(128, (uint32_t)isr80, 0x08, 0xEE);

    idt_load((uint32_t)&idt_ptr);
}

void isr_handler(struct regs *r) {
    serial_write("[EXC ");
    serial_write_hex8((uint8_t)r->int_no);
    serial_write("]");
    for (;;) {
        asm volatile ("cli; hlt");
    }
}

void irq_handler(struct regs *r) {
    if (r->int_no == 33) {
        extern void keyboard_handler();
        keyboard_handler();
    }
    if (r->int_no >= 40) outb(0xA0, 0x20);
    outb(0x20, 0x20);
}
