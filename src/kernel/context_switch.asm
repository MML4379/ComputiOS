; src/kernel/context_switch.asm
; extern "C" void context_switch(ThreadContext* old_ctx, ThreadContext* new_ctx);
;
; ThreadContext layout (must match C++):
;   0x00: r15
;   0x08: r14
;   0x10: r13
;   0x18: r12
;   0x20: rbx
;   0x28: rbp
;   0x30: rsp

[BITS 64]

global context_switch

section .text

; void context_switch(ThreadContext* old_ctx, ThreadContext* new_ctx)
; SysV AMD64 ABI: old_ctx in RDI, new_ctx in RSI
context_switch:
    ; Save callee-saved registers into old_ctx
    mov     [rdi + 0x00], r15
    mov     [rdi + 0x08], r14
    mov     [rdi + 0x10], r13
    mov     [rdi + 0x18], r12
    mov     [rdi + 0x20], rbx
    mov     [rdi + 0x28], rbp
    mov     [rdi + 0x30], rsp

    ; Load callee-saved registers from new_ctx
    mov     r15, [rsi + 0x00]
    mov     r14, [rsi + 0x08]
    mov     r13, [rsi + 0x10]
    mov     r12, [rsi + 0x18]
    mov     rbx, [rsi + 0x20]
    mov     rbp, [rsi + 0x28]
    mov     rsp, [rsi + 0x30]

    ; Return to whatever the new stack's top return address is
    ret
