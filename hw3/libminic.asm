    section .text
    global sys_rt_sigaction:function
    global sys_sigprocmask:function
    global sys_write:function
    global sys_pause:function
    global sys_nanosleep:function
    global sys_alarm:function
    global sys_exit:function
    global setjmp:function
    global longjmp:function
    global __myrt:function

sys_write:
    mov     rax, 1      ; write
    syscall             ; syscall
    ret

sys_pause:
    mov     rax, 34     ; pause
    syscall             ; syscall
    ret

sys_nanosleep:
    mov     rax, 35     ; sys_nanosleep
    syscall             ; syscall
    ret

setjmp:
    mov     QWORD [rdi],    rbx     ; reg[0]
    mov     QWORD [rdi+8],  rsp     ; reg[1]
    mov     QWORD [rdi+16], rbp     ; reg[2]
    mov     QWORD [rdi+24], r12     ; reg[3]
    mov     QWORD [rdi+32], r13     ; reg[4]
    mov     QWORD [rdi+40], r14     ; reg[5]
    mov     QWORD [rdi+48], r15     ; reg[6]
    mov     rax, [rsp]
    mov     QWORD [rdi+56], rax     ; reg[7]       /return address
    mov     rsi, 0                  ; nset         /set NULL
    add     rdi, 64                 ; mask
    mov     rdx, rdi                ; oset         /sigmask will be put here[mask]
    mov     r10, 8                  ; sigsetsize   /size of sigset_t
    mov     rdi, 2                  ; how          /SIG_SETMASK
    mov     rax, 14                 ; sys_rt_sigprocmask
    syscall                         ; syscall
    mov     rax, 0                  ; return 0
    ret

longjmp:
    mov     r14, rdi                ; jump_buf
    mov     r15, rsi                ; val
    mov     r10, 8                  ; sigsetsize      /size of sigset_t
    mov     rdx, 0                  ; oset            /set NULL
    mov     rsi, rdi
    add     rsi, 64                 ; nset            /sigmask will be set here[mask]
    mov     rdi, 2                  ; how             /SIG_SETMASK
    mov     rax, 14                 ; sys_rt_sigprocmask
    syscall                         ; syscall
    mov     rdi, r14
    mov     rsi, r15
    mov     rbx, QWORD [rdi]        ; reg[0]
    mov     rsp, QWORD [rdi+8]      ; reg[1]
    mov     rbp, QWORD [rdi+16]     ; reg[2]
    mov     r12, QWORD [rdi+24]     ; reg[3]
    mov     r13, QWORD [rdi+32]     ; reg[4]
    mov     r14, QWORD [rdi+40]     ; reg[5]
    mov     r15, QWORD [rdi+48]     ; reg[6]
    mov     rax, rsi                ; return value
    jmp     QWORD [rdi+56]          ; reg[7]          /jump address

__myrt:
    mov     rax, 15                 ; sys_rt_sigreturn
    syscall                         ; syscall

sys_rt_sigaction:
    mov     r10, rcx                ; sigset_t
    mov     rax, 13                 ; sys_rt_sigaction
    syscall                         ; syscall
    ret

sys_sigprocmask:
    mov     r10, 8                  ; sigsetsize
    mov     rax, 14                 ; sys_rt_sigprocmask
    syscall                         ; syscall
    ret

sys_alarm:
    mov     rax, 37     ; alarm
    syscall             ; syscall
    ret

sys_exit:
    mov     rax, 60     ; exit
    syscall             ; syscall
