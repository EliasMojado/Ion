format pe64 console
entry start

STD_OUTPUT_HANDLE       = -11

section '.data' data readable writeable
    ; Data section goes here
    dummy db 0  ; Placeholder to keep the section

section '.text' code readable executable
start:
    sub rsp, 16  ; Allocate stack space for program
    add rsp, 16  ; Deallocate stack space for program
    mov ecx, 0  ; Exit code
    call [ExitProcess]

section '.idata' import data readable writeable
    dd      0,0,0,RVA kernel_name,RVA kernel_table
    dd      0,0,0,0,0

kernel_table:
    ExitProcess     dq RVA _ExitProcess
    dq 0

kernel_name     db 'KERNEL32.DLL',0

_ExitProcess    db 0,0,'ExitProcess',0
