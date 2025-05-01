.global NgAsmNativeInvokeTrampolineIntPtr
NgAsmNativeInvokeTrampolineIntPtr:
    push %ebp
    mov %esp,%ebp

    push %edi
    push %esi

    mov %esp, %edi
    mov 12(%ebp),%esi
    mov 8(%ebp),%esp

    call *%esi

    mov %edi, %esp

    pop %esi
    pop %edi

    mov %ebp,%esp
    pop %ebp
    ret

.global NgAsmNativeInvokeTrampolineInt32
NgAsmNativeInvokeTrampolineInt32:
    push %ebp
    mov %esp,%ebp

    push %edi
    push %esi

    mov %esp, %edi
    mov 12(%ebp),%esi
    mov 8(%ebp),%esp

    call *%esi

    mov %edi, %esp

    pop %esi
    pop %edi

    mov %ebp,%esp
    pop %ebp
    ret

.global NgAsmNativeInvokeTrampolineInt64
NgAsmNativeInvokeTrampolineInt64:
    push %ebp
    mov %esp,%ebp

    push %edi
    push %esi

    mov %esp, %edi
    mov 12(%ebp),%esi
    mov 8(%ebp),%esp

    call *%esi

    mov %edi, %esp

    pop %esi
    pop %edi

    mov %ebp,%esp
    pop %ebp
    ret


.global NgAsmNativeInvokeTrampolineSingle
NgAsmNativeInvokeTrampolineSingle:
    push %ebp
    mov %esp,%ebp

    push %edi
    push %esi

    mov %esp, %edi
    mov 12(%ebp),%esi
    mov 8(%ebp),%esp

    call *%esi

    mov %edi, %esp

    pop %esi
    pop %edi

    mov %ebp,%esp
    pop %ebp
    ret


.global NgAsmNativeInvokeTrampolineDouble
NgAsmNativeInvokeTrampolineDouble:
    push %ebp
    mov %esp,%ebp

    push %edi
    push %esi

    mov %esp, %edi
    mov 12(%ebp),%esi
    mov 8(%ebp),%esp

    call *%esi

    mov %edi, %esp

    pop %esi
    pop %edi

    mov %ebp,%esp
    pop %ebp
    ret


.global NgAsmNativeInvokeTrampolinePointer
NgAsmNativeInvokeTrampolinePointer:
    push %ebp
    mov %esp,%ebp

    push %edi
    push %esi

    mov %esp, %edi
    mov 12(%ebp),%esi
    mov 8(%ebp),%esp

    call *%esi

    mov %edi, %esp

    pop %esi
    pop %edi

    mov %ebp,%esp
    pop %ebp
    ret
