; syscall_stub.asm
; Assembly stub for performing direct syscalls on Windows x64
; Handles up to 4 arguments via va_list

.code

DoSyscall PROC
    ; rcx = ssn
    ; rdx = va_list (pointer to args)
    
    mov r10, rdx        ; save va_list in r10
    mov rax, rcx        ; SSN in rax
    
    ; Load first argument into rcx
    mov rcx, [r10]
    add r10, 8
    
    ; Load second argument into rdx
    mov rdx, [r10]
    add r10, 8
    
    ; Load third argument into r8
    mov r8, [r10]
    add r10, 8
    
    ; Load fourth argument into r9
    mov r9, [r10]
    ; For more arguments, they would go on the stack, but omitted for simplicity
    
    syscall
    ret
DoSyscall ENDP

END