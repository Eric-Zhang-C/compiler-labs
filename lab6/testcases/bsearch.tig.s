.text
.globl main
.type main, @function
main:
BEGIN function
main:L12:
 movq $16, %rax
 movq %rax, -8(%rbp)
 movq %rbp, %rdi
 addq $-16, %rdi
 movq -8(%rbp), %rdi
 movq $0, %rsi
 call initArray
 movq %rax, (%rdi)
 movq %rbp, %rdi
 call try
 jmp  L11
L11:

.cfi_endprocEND function
.text
.globl try
.type try, @function
try:
BEGIN function
try:L14:
 movq (%rbp), %rdi
 call init
 movq (%rbp), %rax
 movq (%rax), %rax
 movq (%rax), %rdi
 movq (%rbp), %rdi
 movq $0, %rsi
 movq (%rbp), %rax
 movq -8(%rax), %rdx
 subq $1, %rdx
 movq $7, %rcx
 call bsearch
 movq %rax, %rsi
 call printi
 movq (%rbp), %rax
 movq (%rax), %rax
 movq (%rax), %rdi
 movq L10, %rsi
 call print
 jmp  L13
L13:

.cfi_endprocEND function
.section .rodata
.L10:
.string "   
"
.text
.globl bsearch
.type bsearch, @function
bsearch:
BEGIN function
bsearch:L16:
 cmpq %rdi, %rsi
 je  L7
L8:
 movq %rdi, %rax
 addq %rsi, %rax
 movq $2, %rax
 divq %rax
 movq %rax, -8(%rbp)
 movq (%rbp), %rax
 movq -16(%rax), %rdi
 movq -8(%rbp), %rax
 movq $8, %rax
 mulq %rax
 addq %rax, %rdi
 movq (%rdi), %rax
 cmpq %rax, %rdx
 jl  L4
L5:
 movq (%rbp), %rdi
 movq %rdi, %rsi
 movq -8(%rbp), %rdx
 movq %rdx, %rcx
 call bsearch
L6:
 movq %rax, %rdi
L9:
 movq %rdi, %rax
 jmp  L15
L7:
 jmp  L9
L4:
 movq (%rbp), %rdi
 movq -8(%rbp), %rsi
 addq $1, %rsi
 movq %rsi, %rdx
 movq %rdx, %rcx
 call bsearch
 jmp  L6
L15:

.cfi_endprocEND function
.text
.globl init
.type init, @function
init:
BEGIN function
init:L18:
 movq $0, %rax
 movq (%rbp), %rax
 movq -8(%rax), %rax
 subq $1, %rax
 cmpq %rax, %rax
 jg  L1
L3:
 movq (%rbp), %rax
 movq -16(%rax), %rdi
 movq $8, %rax
 mulq %rax
 addq %rax, %rdi
 movq (%rdi), %rax
 movq (%rbp), %rdi
 call nop
 movq (%rbp), %rax
 movq -8(%rax), %rax
 subq $1, %rax
 cmpq %rax, %rax
 je  L1
L2:
 addq $1, %rax
 movq $2, %rax
 mulq %rax
 addq $1, %rax
 movq (%rbp), %rax
 movq -16(%rax), %rdi
 movq $8, %rax
 mulq %rax
 addq %rax, %rdi
 movq %rax, (%rdi)
 movq (%rbp), %rax
 movq -16(%rax), %rdi
 movq $8, %rax
 mulq %rax
 addq %rax, %rdi
 movq (%rdi), %rax
 movq (%rbp), %rdi
 call nop
 movq (%rbp), %rax
 movq -8(%rax), %rax
 subq $1, %rax
 cmpq %rax, %rax
 jl  L2
L1:
 movq $0, %rax
 jmp  L17
L17:

.cfi_endprocEND function
.text
.globl nop
.type nop, @function
nop:
BEGIN function
nop:L20:
 movq (%rbp), %rax
 movq (%rax), %rax
 movq (%rax), %rdi
 movq L0, %rsi
 call print
 jmp  L19
L19:

.cfi_endprocEND function
.section .rodata
.L0:
.string "    "
