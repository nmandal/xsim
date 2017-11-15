.globl function
function:
mov $300, %eax
mov $300, %ecx
mov $300, %edx
mov $300, %ebx 
mov $300, %esp
mov $300, %ebp
mov $300, %esi
mov $300, %edi
# 
mov $-300, %eax
mov $-300, %ecx
mov $-300, %edx
mov $-300, %ebx 
mov $-300, %esp
mov $-300, %ebp
mov $-300, %esi
mov $-300, %edi
# 
mov %eax, %eax
mov %eax, %ecx
mov %eax, %edx
mov %eax, %ebx
mov %eax, %esp
mov %eax, %ebp
mov %eax, %esi
mov %eax, %edi
#
mov %ecx, %eax
mov %ecx, %ecx
mov %ecx, %edx
mov %ecx, %ebx
mov %ecx, %esp
mov %ecx, %ebp
mov %ecx, %esi
mov %ecx, %edi
#
mov %edx, %eax
mov %edx, %ecx
mov %edx, %edx
mov %edx, %ebx
mov %edx, %esp
mov %edx, %ebp
mov %edx, %esi
mov %edx, %edi
#
mov %esp, %eax
mov %esp, %ecx
mov %esp, %edx
mov %esp, %ebx
mov %esp, %esp
mov %esp, %ebp
mov %esp, %esi
mov %esp, %edi
#
mov %ebp, %eax
mov %ebp, %ecx
mov %ebp, %edx
mov %ebp, %ebx
mov %ebp, %esp
mov %ebp, %ebp
mov %ebp, %esi
mov %ebp, %edi
#
mov %esi, %eax
mov %esi, %ecx
mov %esi, %edx
mov %esi, %ebx
mov %esi, %esp
mov %esi, %ebp
mov %esi, %esi
mov %esi, %edi
#
mov %edi, %eax
mov %edi, %ecx
mov %edi, %edx
mov %edi, %ebx
mov %edi, %esp
mov %edi, %ebp
mov %edi, %esi
mov %edi, %edi
#
addl %eax, %eax
addl %ebx, %ebx
addl %ecx, %ecx
addl %edx, %edx
addl %esi, %esi
addl %edi, %edi
addl %esp, %esp
addl %ebp, %ebp
#
subl %eax, %eax
subl %ebx, %ebx
subl %ecx, %ecx
subl %edx, %edx
subl %esi, %esi
subl %edi, %edi
subl %esp, %esp
subl %ebp, %ebp
#
mov $2, %eax
mov $2, %ebx
mov $200000000, %eax
mov $200000000, %ebx
mov $-20000000, %eax
mov $-20000000, %ebx
#
cmpl $1, %eax
cmpl $1, %ecx
cmpl $1, %edx
cmpl $1, %ebx
cmpl $1, %esp
cmpl $1, %ebp
cmpl $1, %esi
cmpl $1, %edi
#
cmpl $4000000, %eax
cmpl $4000000, %ecx
cmpl $4000000, %edx
cmpl $4000000, %ebx
cmpl $4000000, %esp
cmpl $4000000, %ebp
cmpl $4000000, %ebp
cmpl $4000000, %esi
cmpl $4000000, %edi
#
cmpl $-4000000, %eax
cmpl $-4000000, %ecx
cmpl $-4000000, %edx
cmpl $-4000000, %ebx
cmpl $-4000000, %esp
cmpl $-4000000, %ebp
cmpl $-4000000, %ebp
cmpl $-4000000, %esi
cmpl $-4000000, %edi
#
target:
cmpl %eax,%ebx
cmpl %eax,%ecx
cmpl %eax,%edx
cmpl %eax,%ebx
cmpl %eax,%esi
cmpl %eax,%edi
cmpl %eax,%ebp
cmpl %eax,%esp
#
jmp target
je target
jne target
jge target
jle target
jg target
jl target
#
jmp target2
je target2
jne target2
jge target2
jle target2
jg target2
jl target2
#
call target2
cmpl %eax,%ebx
cmpl %eax,%ecx
cmpl %eax,%edx
cmpl %eax,%ebx
cmpl %eax,%esi
cmpl %eax,%edi
cmpl %eax,%ebp
cmpl %eax,%esp
#
target2:
call target
ret
# fancy mov
mov 4(%ebp), %ecx
mov 8(%ebp), %ecx
mov -4(%ebp), %ecx
mov %edx, 4(%ebp)
mov %edx, 8(%ebp)
mov %edx, -4(%ebp)
nop
