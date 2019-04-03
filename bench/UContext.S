# UContext.S: i386 only
# UContext[0]: %eip
# UContext[1]: %esp
# UContext[2]: %ebx
# UContext[3]: %ecx
# UContext[4]: %edx
# UContext[5]: %edi
# UContext[6]: %esi
# UContext[7]: %ebp
# UContext[8]: %eax

# assume that %edx preserved during procdure callings

# prototype: int save_context(UContext_t* from)
.global save_context
.type save_context, @function
save_context:
    popl %eax
    movl (%esp), %edx
    movl %eax, (%edx)
    movl %ebx, 8(%edx)
    movl %ecx, 12(%edx)
    movl %edi, 20(%edx)
    movl %esi, 24(%edx)
    movl %ebp, 28(%edx)
    movl %esp, 4(%edx)
    pushl %eax
    xorl %eax, %eax
    ret

# prototype: int restore_context(UContext_t* to)
.global restore_context
.type restore_context, @function
restore_context:
    movl 4(%esp), %edx
    movl 8(%edx), %ebx
    movl 12(%edx), %ecx
    movl 20(%edx), %edi
    movl 24(%edx), %esi
    movl 28(%edx), %ebp
    movl 4(%edx), %esp
    movl (%edx), %edx
    movl $1, %eax
    jmp *%edx
