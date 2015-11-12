	.file	"fib-0.c"
	.globl	fib_results
	.data
	.align 64
	.type	fib_results, @object
	.size	fib_results, 164
fib_results:
	.long	0
	.long	1
	.long	1
	.long	2
	.long	3
	.long	5
	.long	8
	.long	13
	.long	21
	.long	34
	.long	55
	.long	89
	.long	144
	.long	233
	.long	377
	.long	610
	.long	987
	.long	1597
	.long	2584
	.long	4181
	.long	6765
	.long	10946
	.long	17711
	.long	28657
	.long	46368
	.long	75025
	.long	121393
	.long	196418
	.long	317811
	.long	514229
	.long	832040
	.long	1346269
	.long	2178309
	.long	3524578
	.long	5702887
	.long	9227465
	.long	14930352
	.long	24157817
	.long	39088169
	.long	63245986
	.long	102334155
	.local	par_res
	.comm	par_res,4,4
	.local	seq_res
	.comm	seq_res,4,4
	.section	.rodata
.LC0:
	.string	"Usage: fibonacci NUMBER\n"
	.text
	.type	_usage, @function
_usage:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movl	%esi, -12(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rcx
	movl	$24, %edx
	movl	$1, %esi
	movl	$.LC0, %edi
	call	fwrite
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	fflush
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	_usage, .-_usage
	.globl	fib
	.type	fib, @function
fib:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$64, %rsp
	movl	%edi, -52(%rbp)
	cmpl	$1, -52(%rbp)
	jg	.L3
	movl	-52(%rbp), %eax
	jmp	.L4
.L3:
	movl	-52(%rbp), %eax
	movl	%eax, -24(%rbp)
	leaq	-36(%rbp), %rax
	movq	%rax, -32(%rbp)
	leaq	-32(%rbp), %rax
	pushq	$0
	pushq	$1
	movl	$1, %r9d
	movl	$8, %r8d
	movl	$16, %ecx
	movl	$0, %edx
	movq	%rax, %rsi
	movl	$fib._omp_fn.0, %edi
	call	GOMP_task
	addq	$16, %rsp
	movl	-52(%rbp), %eax
	movl	%eax, -8(%rbp)
	leaq	-40(%rbp), %rax
	movq	%rax, -16(%rbp)
	leaq	-16(%rbp), %rax
	pushq	$0
	pushq	$1
	movl	$1, %r9d
	movl	$8, %r8d
	movl	$16, %ecx
	movl	$0, %edx
	movq	%rax, %rsi
	movl	$fib._omp_fn.1, %edi
	call	GOMP_task
	addq	$16, %rsp
	call	GOMP_taskwait
	movl	-40(%rbp), %eax
	movl	-36(%rbp), %edx
	addl	%edx, %eax
.L4:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	fib, .-fib
	.section	.rodata
.LC1:
	.string	"h?"
.LC2:
	.string	"\nMissing fib number.\n"
	.align 8
.LC3:
	.string	"Fibonacci result for %d is %d\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movq	%rsi, -32(%rbp)
	movl	$0, -4(%rbp)
	jmp	.L6
.L9:
	movl	-4(%rbp), %eax
	cmpl	$104, %eax
	jne	.L7
	movq	stdout(%rip), %rax
	movl	$1, %esi
	movq	%rax, %rdi
	call	_usage
.L7:
	movq	stderr(%rip), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	_usage
.L6:
	movq	-32(%rbp), %rcx
	movl	-20(%rbp), %eax
	movl	$.LC1, %edx
	movq	%rcx, %rsi
	movl	%eax, %edi
	call	getopt
	movl	%eax, -4(%rbp)
	cmpl	$-1, -4(%rbp)
	jne	.L9
	movl	optind(%rip), %eax
	subl	%eax, -20(%rbp)
	movl	optind(%rip), %eax
	cltq
	salq	$3, %rax
	addq	%rax, -32(%rbp)
	movl	$1, -8(%rbp)
	movl	-20(%rbp), %eax
	testl	%eax, %eax
	je	.L11
	cmpl	$1, %eax
	je	.L12
	jmp	.L10
.L11:
	movq	stderr(%rip), %rax
	movq	%rax, %rcx
	movl	$21, %edx
	movl	$1, %esi
	movl	$.LC2, %edi
	call	fwrite
.L10:
	movq	stderr(%rip), %rax
	movl	$0, %esi
	movq	%rax, %rdi
	call	_usage
.L12:
	movq	-32(%rbp), %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	atoi
	movl	%eax, -8(%rbp)
	nop
	movl	-8(%rbp), %eax
	movl	%eax, -16(%rbp)
	leaq	-16(%rbp), %rax
	movl	$0, %ecx
	movl	$0, %edx
	movq	%rax, %rsi
	movl	$main._omp_fn.2, %edi
	call	GOMP_parallel
	movl	-16(%rbp), %eax
	movl	%eax, -8(%rbp)
	movl	par_res(%rip), %edx
	movl	-8(%rbp), %eax
	movl	%eax, %esi
	movl	$.LC3, %edi
	movl	$0, %eax
	call	printf
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	main, .-main
	.type	fib._omp_fn.1, @function
fib._omp_fn.1:
.LFB4:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$40, %rsp
	.cfi_offset 3, -24
	movq	%rdi, -40(%rbp)
	movq	-40(%rbp), %rax
	movl	8(%rax), %eax
	movl	%eax, -20(%rbp)
	movl	-20(%rbp), %eax
	leal	-2(%rax), %edx
	movq	-40(%rbp), %rax
	movq	(%rax), %rbx
	movl	%edx, %edi
	call	fib
	movl	%eax, (%rbx)
	addq	$40, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	fib._omp_fn.1, .-fib._omp_fn.1
	.type	fib._omp_fn.0, @function
fib._omp_fn.0:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$40, %rsp
	.cfi_offset 3, -24
	movq	%rdi, -40(%rbp)
	movq	-40(%rbp), %rax
	movl	8(%rax), %eax
	movl	%eax, -20(%rbp)
	movl	-20(%rbp), %eax
	leal	-1(%rax), %edx
	movq	-40(%rbp), %rax
	movq	(%rax), %rbx
	movl	%edx, %edi
	call	fib
	movl	%eax, (%rbx)
	addq	$40, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	fib._omp_fn.0, .-fib._omp_fn.0
	.section	.rodata
.LC4:
	.string	"OpenMP Work took %f sec.\n"
	.text
	.type	main._omp_fn.2, @function
main._omp_fn.2:
.LFB5:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	call	GOMP_single_start
	cmpb	$1, %al
	jne	.L15
	call	omp_get_wtime
	movq	%xmm0, %rax
	movq	%rax, -8(%rbp)
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	movl	%eax, %edi
	call	fib
	movl	%eax, par_res(%rip)
	call	omp_get_wtime
	subsd	-8(%rbp), %xmm0
	movq	%xmm0, %rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	movq	%rax, -32(%rbp)
	movsd	-32(%rbp), %xmm0
	movl	$.LC4, %edi
	movl	$1, %eax
	call	printf
.L15:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	main._omp_fn.2, .-main._omp_fn.2
	.ident	"GCC: (GNU) 4.9.1"
	.section	.note.GNU-stack,"",@progbits
