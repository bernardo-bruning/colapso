[bits 32]

global idt_load
idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

; ISR Stubs (Exceções)
global isr0
global isr3
extern isr_handler

isr0:
    push dword 0
    jmp isr_common_stub

isr3:
    push dword 3
    jmp isr_common_stub

isr_common_stub:
    pusha
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push dword [esp + 36] ; Pega o número da interrupção (pushed 0 ou 3)
    call isr_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 4
    iret

; IRQ Stubs (Hardware)
global irq0
global irq1
global irq14
extern irq_handler

irq0:
    push dword 32
    jmp irq_common_stub

irq1:
    push dword 33
    jmp irq_common_stub

irq14:
    push dword 46
    jmp irq_common_stub

irq_common_stub:
    pusha
    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push dword [esp + 36] ; Pega o número da IRQ (pushed 32 ou 33)
    call irq_handler
    add esp, 4

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 4
    iret
