; Cooperative/preemptive context switch between kernel threads

; void context_switch(context_t *old, context_t *new_ctx);
;   RDI = pointer to old context (save current state here)
;   RSI = pointer to new context (restore state from here)

; context_t layout (offsets):
;   0x00  rsp
;   0x08  rbp
;   0x10  rbx
;   0x18  r12
;   0x20  r13
;   0x28  r14
;   0x30  r15
;   0x38  rip
;   0x40  rflags
;   0x48  cr3

[bits 64]
section .text
global context_switch

context_switch:
    ; save current context into *old (rdi)
    mov [rdi + 0x08], rbp
    mov [rdi + 0x10], rbx
    mov [rdi + 0x18], r12
    mov [rdi + 0x20], r13
    mov [rdi + 0x28], r14
    mov [rdi + 0x30], r15

    ; Save return address as the RIP we'll resume at
    mov rax, [rsp] ; return address
    mov [rdi + 0x38], rax

    ; Save RSP (after the call pushed return addr)
    mov [rdi + 0x00], rsp

    ; Save RFLAGS
    pushfq
    pop rax
    mov [rdi + 0x40], rax

    ; Save CR3
    mov rax, cr3
    mov [rdi + 0x48], rax

    ; Restore new context from *new_ctx (rsi)

    ; Switch CR3 if different (avoid TLB flush when same)
    mov rax, [rsi + 0x48]
    mov rcx, cr3
    cmp rax, rcx
    je .same_cr3
    mov cr3, rax
.same_cr3:

    ; Restore callee-saved regs
    mov rbp, [rsi + 0x08]
    mov rbx, [rsi + 0x10]
    mov r12, [rsi + 0x18]
    mov r13, [rsi + 0x20]
    mov r14, [rsi + 0x28]
    mov r15, [rsi + 0x30]

    ; Restore RFLAGS
    mov rax, [rsi + 0x40]
    push rax
    popfq

    ; Restore RSP
    mov rsp, [rsi + 0x00]

    ; Push new RIP as return address and return into it
    mov rax, [rsi + 0x38]
    mov [rsp], rax ; overwrite return address on stack

    ret