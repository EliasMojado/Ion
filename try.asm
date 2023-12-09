format PE64 console
entry start

section '.text' executable

start:
    ; write 'Hello, World!' to the console
    mov rcx, -11             ; file handle (STD_OUTPUT_HANDLE)
    call [GetStdHandle]
    mov rdx, msg             ; pointer to the message
    mov r8, msg.len          ; message length
    lea r9, [rsp+8]          ; buffer to receive number of chars written
    mov qword [rsp+8], 0     ; clear buffer
    call [WriteConsoleA]
    
    ; exit the process
    mov ecx, 0               ; exit code
    call [ExitProcess]

section '.data' data readable writeable

    msg db 'Hello, World!',0
    .len equ $ - msg

section '.idata' import data readable writeable

    library kernel32,'KERNEL32.DLL'

    import kernel32,\
           GetStdHandle,'GetStdHandle',\
           WriteConsoleA,'WriteConsoleA',\
           ExitProcess,'ExitProcess'
