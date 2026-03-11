[bits 32]

global idt_load
idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

; --- STUB COMUM ---
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    push dword 0 ; Dummy error code
    push dword %1 ; Interrupt number
    jmp common_stub
%endmacro

%macro IRQ 2
global irq%1
irq%1:
    push dword 0 ; Dummy error code
    push dword %2 ; Interrupt number (32 + irq)
    jmp common_stub
%endmacro

common_stub:
    pusha
    mov ax, ds
    push eax
    
    mov ax, 0x10 ; DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp ; Passa struct regs* como argumento
    
    ; O número da interrupção agora está em [esp + 40] 
    ; (32 bytes de pusha + 4 de ds + 4 de push esp)
    mov eax, [esp + 40] 
    
    cmp eax, 128
    je .call_syscall
    cmp eax, 32
    jae .call_irq
    
    extern isr_handler
    call isr_handler
    jmp .done

.call_irq:
    extern irq_handler
    call irq_handler
    jmp .done

.call_syscall:
    extern syscall_handler
    call syscall_handler

.done:
    add esp, 4 ; Limpa o 'push esp'
    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    popa
    add esp, 8 ; Limpa int_no e error_code
    iret

; --- ENTRADAS ---
ISR_NOERRCODE 0
ISR_NOERRCODE 3
IRQ 0, 32
IRQ 1, 33
IRQ 14, 46

global isr80
isr80:
    push dword 0
    push dword 128
    jmp common_stub
