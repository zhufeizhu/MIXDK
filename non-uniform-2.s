	.file	"non-uniform-2.c"
	.text
	.type	kfifo_unused, @function
kfifo_unused:
.LFB5:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movl	8(%rax), %edx
	movq	-8(%rbp), %rax
	movl	4(%rax), %ecx
	movq	-8(%rbp), %rax
	movl	(%rax), %eax
	subl	%eax, %ecx
	movl	%ecx, %eax
	addl	%edx, %eax
	addl	$1, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	kfifo_unused, .-kfifo_unused
	.type	__kfifo_init, @function
__kfifo_init:
.LFB6:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movl	%edx, -20(%rbp)
	movq	%rcx, -32(%rbp)
	movl	-20(%rbp), %eax
	movl	$0, %edx
	divq	-32(%rbp)
	movl	%eax, -20(%rbp)
	movq	-8(%rbp), %rax
	movl	$0, (%rax)
	movq	-8(%rbp), %rax
	movl	$0, 4(%rax)
	movq	-32(%rbp), %rax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	movl	%edx, 12(%rax)
	movq	-8(%rbp), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, 16(%rax)
	cmpl	$1, -20(%rbp)
	ja	.L4
	movq	-8(%rbp), %rax
	movl	$0, 8(%rax)
	movl	$1, %eax
	jmp	.L5
.L4:
	movl	-20(%rbp), %eax
	leal	-1(%rax), %edx
	movq	-8(%rbp), %rax
	movl	%edx, 8(%rax)
	movl	$0, %eax
.L5:
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	__kfifo_init, .-__kfifo_init
	.type	min, @function
min:
.LFB7:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	movl	-8(%rbp), %eax
	cmpl	%eax, -4(%rbp)
	cmovbe	-4(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	min, .-min
	.type	__kfifo_max_r, @function
__kfifo_max_r:
.LFB8:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	movl	-4(%rbp), %eax
	cmpl	%eax, -8(%rbp)
	cmovnb	-8(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	__kfifo_max_r, .-__kfifo_max_r
	.type	kfifo_copy_in, @function
kfifo_copy_in:
.LFB9:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movl	%edx, -36(%rbp)
	movl	%ecx, -40(%rbp)
	movq	-24(%rbp), %rax
	movl	8(%rax), %eax
	addl	$1, %eax
	movl	%eax, -12(%rbp)
	movq	-24(%rbp), %rax
	movl	12(%rax), %eax
	movl	%eax, -8(%rbp)
	movq	-24(%rbp), %rax
	movl	8(%rax), %eax
	andl	%eax, -40(%rbp)
	cmpl	$1, -8(%rbp)
	je	.L11
	movl	-40(%rbp), %eax
	imull	-8(%rbp), %eax
	movl	%eax, -40(%rbp)
	movl	-12(%rbp), %eax
	imull	-8(%rbp), %eax
	movl	%eax, -12(%rbp)
	movl	-36(%rbp), %eax
	imull	-8(%rbp), %eax
	movl	%eax, -36(%rbp)
.L11:
	movl	-12(%rbp), %eax
	subl	-40(%rbp), %eax
	movl	%eax, %edx
	movl	-36(%rbp), %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	min
	movl	%eax, -4(%rbp)
	movl	-4(%rbp), %edx
	movq	-24(%rbp), %rax
	movq	16(%rax), %rcx
	movl	-40(%rbp), %eax
	addq	%rax, %rcx
	movq	-32(%rbp), %rax
	movq	%rax, %rsi
	movq	%rcx, %rdi
	call	memcpy@PLT
	movl	-36(%rbp), %eax
	subl	-4(%rbp), %eax
	movl	%eax, %esi
	movl	-4(%rbp), %edx
	movq	-32(%rbp), %rax
	leaq	(%rdx,%rax), %rcx
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rsi, %rdx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	memcpy@PLT
	mfence
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	kfifo_copy_in, .-kfifo_copy_in
	.type	__kfifo_in, @function
__kfifo_in:
.LFB10:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movl	%edx, -36(%rbp)
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	kfifo_unused
	movl	%eax, -4(%rbp)
	movl	-36(%rbp), %eax
	cmpl	-4(%rbp), %eax
	jbe	.L13
	movl	-4(%rbp), %eax
	movl	%eax, -36(%rbp)
.L13:
	movq	-24(%rbp), %rax
	movl	(%rax), %ecx
	movl	-36(%rbp), %edx
	movq	-32(%rbp), %rsi
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	kfifo_copy_in
	movq	-24(%rbp), %rax
	movl	(%rax), %edx
	movl	-36(%rbp), %eax
	addl	%eax, %edx
	movq	-24(%rbp), %rax
	movl	%edx, (%rax)
	movl	-36(%rbp), %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE10:
	.size	__kfifo_in, .-__kfifo_in
	.type	__kfifo_peek_n, @function
__kfifo_peek_n:
.LFB11:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	-24(%rbp), %rax
	movl	8(%rax), %eax
	movl	%eax, -12(%rbp)
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rax, -8(%rbp)
	movq	-24(%rbp), %rax
	movl	4(%rax), %eax
	andl	-12(%rbp), %eax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	movzbl	%al, %eax
	movl	%eax, -16(%rbp)
	subq	$1, -32(%rbp)
	cmpq	$0, -32(%rbp)
	je	.L16
	movq	-24(%rbp), %rax
	movl	4(%rax), %eax
	addl	$1, %eax
	andl	-12(%rbp), %eax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	addq	%rdx, %rax
	movzbl	(%rax), %eax
	movzbl	%al, %eax
	sall	$8, %eax
	orl	%eax, -16(%rbp)
.L16:
	movl	-16(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE11:
	.size	__kfifo_peek_n, .-__kfifo_peek_n
	.type	__kfifo_poke_n, @function
__kfifo_poke_n:
.LFB12:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	movl	%esi, -28(%rbp)
	movq	%rdx, -40(%rbp)
	movq	-24(%rbp), %rax
	movl	8(%rax), %eax
	movl	%eax, -12(%rbp)
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	movq	%rax, -8(%rbp)
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	andl	-12(%rbp), %eax
	movl	%eax, %edx
	movq	-8(%rbp), %rax
	addq	%rdx, %rax
	movl	-28(%rbp), %edx
	movb	%dl, (%rax)
	cmpq	$1, -40(%rbp)
	jbe	.L20
	movl	-28(%rbp), %eax
	shrl	$8, %eax
	movl	%eax, %edx
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	addl	$1, %eax
	andl	-12(%rbp), %eax
	movl	%eax, %ecx
	movq	-8(%rbp), %rax
	addq	%rcx, %rax
	movb	%dl, (%rax)
.L20:
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE12:
	.size	__kfifo_poke_n, .-__kfifo_poke_n
	.globl	__kfifo_len_r
	.type	__kfifo_len_r, @function
__kfifo_len_r:
.LFB13:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-16(%rbp), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	__kfifo_peek_n
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE13:
	.size	__kfifo_len_r, .-__kfifo_len_r
	.globl	__kfifo_in_r
	.type	__kfifo_in_r, @function
__kfifo_in_r:
.LFB14:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$40, %rsp
	.cfi_offset 3, -24
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movl	%edx, -36(%rbp)
	movq	%rcx, -48(%rbp)
	movl	-36(%rbp), %edx
	movq	-48(%rbp), %rax
	leaq	(%rdx,%rax), %rbx
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	kfifo_unused
	movl	%eax, %eax
	cmpq	%rax, %rbx
	jbe	.L24
	movl	$0, %eax
	jmp	.L25
.L24:
	movq	-48(%rbp), %rdx
	movl	-36(%rbp), %ecx
	movq	-24(%rbp), %rax
	movl	%ecx, %esi
	movq	%rax, %rdi
	call	__kfifo_poke_n
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	movq	-48(%rbp), %rdx
	leal	(%rax,%rdx), %ecx
	movl	-36(%rbp), %edx
	movq	-32(%rbp), %rsi
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	kfifo_copy_in
	movq	-24(%rbp), %rax
	movl	(%rax), %eax
	movq	-48(%rbp), %rdx
	movl	%edx, %ecx
	movl	-36(%rbp), %edx
	addl	%ecx, %edx
	addl	%eax, %edx
	movq	-24(%rbp), %rax
	movl	%edx, (%rax)
	movl	-36(%rbp), %eax
.L25:
	addq	$40, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE14:
	.size	__kfifo_in_r, .-__kfifo_in_r
	.type	kfifo_copy_out, @function
kfifo_copy_out:
.LFB15:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movl	%edx, -36(%rbp)
	movl	%ecx, -40(%rbp)
	movq	-24(%rbp), %rax
	movl	8(%rax), %eax
	addl	$1, %eax
	movl	%eax, -12(%rbp)
	movq	-24(%rbp), %rax
	movl	12(%rax), %eax
	movl	%eax, -8(%rbp)
	movq	-24(%rbp), %rax
	movl	8(%rax), %eax
	andl	%eax, -40(%rbp)
	cmpl	$1, -8(%rbp)
	je	.L27
	movl	-40(%rbp), %eax
	imull	-8(%rbp), %eax
	movl	%eax, -40(%rbp)
	movl	-12(%rbp), %eax
	imull	-8(%rbp), %eax
	movl	%eax, -12(%rbp)
	movl	-36(%rbp), %eax
	imull	-8(%rbp), %eax
	movl	%eax, -36(%rbp)
.L27:
	movl	-12(%rbp), %eax
	subl	-40(%rbp), %eax
	movl	%eax, %edx
	movl	-36(%rbp), %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	min
	movl	%eax, -4(%rbp)
	movl	-4(%rbp), %edx
	movq	-24(%rbp), %rax
	movq	16(%rax), %rcx
	movl	-40(%rbp), %eax
	addq	%rax, %rcx
	movq	-32(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	memcpy@PLT
	movl	-36(%rbp), %eax
	subl	-4(%rbp), %eax
	movl	%eax, %esi
	movq	-24(%rbp), %rax
	movq	16(%rax), %rax
	movl	-4(%rbp), %ecx
	movq	-32(%rbp), %rdx
	addq	%rdx, %rcx
	movq	%rsi, %rdx
	movq	%rax, %rsi
	movq	%rcx, %rdi
	call	memcpy@PLT
	mfence
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE15:
	.size	kfifo_copy_out, .-kfifo_copy_out
	.globl	__kfifo_out_peek
	.type	__kfifo_out_peek, @function
__kfifo_out_peek:
.LFB16:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movl	%edx, -36(%rbp)
	movq	-24(%rbp), %rax
	movl	(%rax), %edx
	movq	-24(%rbp), %rax
	movl	4(%rax), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -4(%rbp)
	movl	-36(%rbp), %eax
	cmpl	-4(%rbp), %eax
	jbe	.L29
	movl	-4(%rbp), %eax
	movl	%eax, -36(%rbp)
.L29:
	movq	-24(%rbp), %rax
	movl	4(%rax), %ecx
	movl	-36(%rbp), %edx
	movq	-32(%rbp), %rsi
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	kfifo_copy_out
	movl	-36(%rbp), %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE16:
	.size	__kfifo_out_peek, .-__kfifo_out_peek
	.globl	__kfifo_out_peek_offset
	.type	__kfifo_out_peek_offset, @function
__kfifo_out_peek_offset:
.LFB17:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movl	%edx, -36(%rbp)
	movl	%ecx, -40(%rbp)
	movq	-24(%rbp), %rax
	movl	(%rax), %edx
	movq	-24(%rbp), %rax
	movl	4(%rax), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -4(%rbp)
	movl	-36(%rbp), %eax
	cmpl	-4(%rbp), %eax
	jbe	.L32
	movl	-4(%rbp), %eax
	movl	%eax, -36(%rbp)
.L32:
	movq	-24(%rbp), %rax
	movl	4(%rax), %edx
	movl	-40(%rbp), %eax
	leal	(%rdx,%rax), %ecx
	movl	-36(%rbp), %edx
	movq	-32(%rbp), %rsi
	movq	-24(%rbp), %rax
	movq	%rax, %rdi
	call	kfifo_copy_out
	movl	-36(%rbp), %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE17:
	.size	__kfifo_out_peek_offset, .-__kfifo_out_peek_offset
	.globl	__kfifo_out
	.type	__kfifo_out, @function
__kfifo_out:
.LFB18:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movl	%edx, -20(%rbp)
	movl	-20(%rbp), %edx
	movq	-16(%rbp), %rcx
	movq	-8(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	__kfifo_out_peek
	movl	%eax, -20(%rbp)
	movq	-8(%rbp), %rax
	movl	4(%rax), %edx
	movl	-20(%rbp), %eax
	addl	%eax, %edx
	movq	-8(%rbp), %rax
	movl	%edx, 4(%rax)
	movl	-20(%rbp), %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE18:
	.size	__kfifo_out, .-__kfifo_out
	.type	kfifo_out_copy_r, @function
kfifo_out_copy_r:
.LFB19:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movl	%edx, -20(%rbp)
	movq	%rcx, -32(%rbp)
	movq	%r8, -40(%rbp)
	movq	-32(%rbp), %rdx
	movq	-8(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	__kfifo_peek_n
	movl	%eax, %edx
	movq	-40(%rbp), %rax
	movl	%edx, (%rax)
	movq	-40(%rbp), %rax
	movl	(%rax), %eax
	cmpl	%eax, -20(%rbp)
	jbe	.L37
	movq	-40(%rbp), %rax
	movl	(%rax), %eax
	movl	%eax, -20(%rbp)
.L37:
	movq	-8(%rbp), %rax
	movl	4(%rax), %eax
	movq	-32(%rbp), %rdx
	leal	(%rax,%rdx), %ecx
	movl	-20(%rbp), %edx
	movq	-16(%rbp), %rsi
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	kfifo_copy_out
	movl	-20(%rbp), %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE19:
	.size	kfifo_out_copy_r, .-kfifo_out_copy_r
	.globl	__kfifo_out_peek_r
	.type	__kfifo_out_peek_r, @function
__kfifo_out_peek_r:
.LFB20:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movl	%edx, -36(%rbp)
	movq	%rcx, -48(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	movq	-24(%rbp), %rax
	movl	(%rax), %edx
	movq	-24(%rbp), %rax
	movl	4(%rax), %eax
	cmpl	%eax, %edx
	jne	.L40
	movl	$0, %eax
	jmp	.L42
.L40:
	leaq	-12(%rbp), %rdi
	movq	-48(%rbp), %rcx
	movl	-36(%rbp), %edx
	movq	-32(%rbp), %rsi
	movq	-24(%rbp), %rax
	movq	%rdi, %r8
	movq	%rax, %rdi
	call	kfifo_out_copy_r
.L42:
	movq	-8(%rbp), %rcx
	xorq	%fs:40, %rcx
	je	.L43
	call	__stack_chk_fail@PLT
.L43:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE20:
	.size	__kfifo_out_peek_r, .-__kfifo_out_peek_r
	.globl	__kfifo_out_r
	.type	__kfifo_out_r, @function
__kfifo_out_r:
.LFB21:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movl	%edx, -36(%rbp)
	movq	%rcx, -48(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	movq	-24(%rbp), %rax
	movl	(%rax), %edx
	movq	-24(%rbp), %rax
	movl	4(%rax), %eax
	cmpl	%eax, %edx
	jne	.L45
	movl	$0, %eax
	jmp	.L47
.L45:
	leaq	-12(%rbp), %rdi
	movq	-48(%rbp), %rcx
	movl	-36(%rbp), %edx
	movq	-32(%rbp), %rsi
	movq	-24(%rbp), %rax
	movq	%rdi, %r8
	movq	%rax, %rdi
	call	kfifo_out_copy_r
	movl	%eax, -36(%rbp)
	movq	-24(%rbp), %rax
	movl	4(%rax), %eax
	movq	-48(%rbp), %rdx
	movl	%edx, %ecx
	movl	-12(%rbp), %edx
	addl	%ecx, %edx
	addl	%eax, %edx
	movq	-24(%rbp), %rax
	movl	%edx, 4(%rax)
	movl	-36(%rbp), %eax
.L47:
	movq	-8(%rbp), %rsi
	xorq	%fs:40, %rsi
	je	.L48
	call	__stack_chk_fail@PLT
.L48:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE21:
	.size	__kfifo_out_r, .-__kfifo_out_r
	.globl	__kfifo_skip_r
	.type	__kfifo_skip_r, @function
__kfifo_skip_r:
.LFB22:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	-32(%rbp), %rdx
	movq	-24(%rbp), %rax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	__kfifo_peek_n
	movl	%eax, -4(%rbp)
	movq	-24(%rbp), %rax
	movl	4(%rax), %eax
	movq	-32(%rbp), %rdx
	movl	%edx, %ecx
	movl	-4(%rbp), %edx
	addl	%ecx, %edx
	addl	%eax, %edx
	movq	-24(%rbp), %rax
	movl	%edx, 4(%rax)
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE22:
	.size	__kfifo_skip_r, .-__kfifo_skip_r
	.type	__kfifo_uint_must_check_helper, @function
__kfifo_uint_must_check_helper:
.LFB23:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, -4(%rbp)
	movl	-4(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE23:
	.size	__kfifo_uint_must_check_helper, .-__kfifo_uint_must_check_helper
	.section	.rodata
.LC0:
	.string	"The size of fifo: %u bytes\n"
	.align 8
.LC1:
	.string	"After insert 3 string, the available bytes of fifo: %u bytes\n"
	.align 8
.LC2:
	.string	"\t used  bytes of fifo: %u bytes\n"
.LC3:
	.string	"get a string: %s, len: %d\n"
	.align 8
.LC4:
	.string	"\tAfter out 1 value, used  bytes of fifo: %u bytes\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB25:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$400, %rsp
	movl	%edi, -388(%rbp)
	movq	%rsi, -400(%rbp)
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	movl	$1048576, %edi
	call	malloc@PLT
	movq	%rax, -368(%rbp)
	leaq	-144(%rbp), %rax
	movq	%rax, -360(%rbp)
	movq	-360(%rbp), %rax
	movq	%rax, -352(%rbp)
	movq	-368(%rbp), %rsi
	movq	-352(%rbp), %rax
	movl	$1, %ecx
	movl	$1048576, %edx
	movq	%rax, %rdi
	call	__kfifo_init
	movl	-136(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -376(%rbp)
	movl	-376(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC0(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	leaq	-112(%rbp), %rax
	movl	$1819043176, (%rax)
	movw	$111, 4(%rax)
	leaq	-144(%rbp), %rax
	movq	%rax, -344(%rbp)
	leaq	-112(%rbp), %rax
	movq	%rax, -336(%rbp)
	leaq	-112(%rbp), %rax
	movq	%rax, %rdi
	call	strlen@PLT
	addq	$1, %rax
	movq	%rax, -328(%rbp)
	movq	$2, -320(%rbp)
	movq	-344(%rbp), %rax
	movq	%rax, -312(%rbp)
	cmpq	$0, -320(%rbp)
	je	.L53
	movq	-328(%rbp), %rax
	movl	%eax, %edi
	movq	-320(%rbp), %rdx
	movq	-336(%rbp), %rsi
	movq	-312(%rbp), %rax
	movq	%rdx, %rcx
	movl	%edi, %edx
	movq	%rax, %rdi
	call	__kfifo_in_r
	jmp	.L54
.L53:
	movq	-328(%rbp), %rax
	movl	%eax, %edx
	movq	-336(%rbp), %rcx
	movq	-312(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	__kfifo_in
.L54:
	leaq	-112(%rbp), %rax
	movw	$26984, (%rax)
	movb	$0, 2(%rax)
	leaq	-144(%rbp), %rax
	movq	%rax, -304(%rbp)
	leaq	-112(%rbp), %rax
	movq	%rax, -296(%rbp)
	leaq	-112(%rbp), %rax
	movq	%rax, %rdi
	call	strlen@PLT
	addq	$1, %rax
	movq	%rax, -288(%rbp)
	movq	$2, -280(%rbp)
	movq	-304(%rbp), %rax
	movq	%rax, -272(%rbp)
	cmpq	$0, -280(%rbp)
	je	.L55
	movq	-288(%rbp), %rax
	movl	%eax, %edi
	movq	-280(%rbp), %rdx
	movq	-296(%rbp), %rsi
	movq	-272(%rbp), %rax
	movq	%rdx, %rcx
	movl	%edi, %edx
	movq	%rax, %rdi
	call	__kfifo_in_r
	jmp	.L56
.L55:
	movq	-288(%rbp), %rax
	movl	%eax, %edx
	movq	-296(%rbp), %rcx
	movq	-272(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	__kfifo_in
.L56:
	leaq	-112(%rbp), %rax
	movabsq	$2338328219631577204, %rsi
	movabsq	$7089063228440191073, %rdi
	movq	%rsi, (%rax)
	movq	%rdi, 8(%rax)
	movabsq	$8387794212885652844, %rcx
	movq	%rcx, 16(%rax)
	movl	$1702109288, 24(%rax)
	movw	$29811, 28(%rax)
	movb	$0, 30(%rax)
	leaq	-144(%rbp), %rax
	movq	%rax, -264(%rbp)
	leaq	-112(%rbp), %rax
	movq	%rax, -256(%rbp)
	leaq	-112(%rbp), %rax
	movq	%rax, %rdi
	call	strlen@PLT
	addq	$1, %rax
	movq	%rax, -248(%rbp)
	movq	$2, -240(%rbp)
	movq	-264(%rbp), %rax
	movq	%rax, -232(%rbp)
	cmpq	$0, -240(%rbp)
	je	.L57
	movq	-248(%rbp), %rax
	movl	%eax, %edi
	movq	-240(%rbp), %rdx
	movq	-256(%rbp), %rsi
	movq	-232(%rbp), %rax
	movq	%rdx, %rcx
	movl	%edi, %edx
	movq	%rax, %rdi
	call	__kfifo_in_r
	jmp	.L58
.L57:
	movq	-248(%rbp), %rax
	movl	%eax, %edx
	movq	-256(%rbp), %rcx
	movq	-232(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	__kfifo_in
.L58:
	leaq	-144(%rbp), %rax
	movq	%rax, -224(%rbp)
	movq	$2, -216(%rbp)
	movq	-224(%rbp), %rax
	movl	8(%rax), %edx
	movq	-224(%rbp), %rax
	movq	%rax, -208(%rbp)
	movq	-208(%rbp), %rax
	movl	(%rax), %ecx
	movq	-208(%rbp), %rax
	movl	4(%rax), %eax
	subl	%eax, %ecx
	movl	%ecx, %eax
	subl	%eax, %edx
	movl	%edx, %eax
	addl	$1, %eax
	movl	%eax, -372(%rbp)
	cmpq	$0, -216(%rbp)
	je	.L59
	movl	-372(%rbp), %eax
	cmpq	%rax, -216(%rbp)
	jnb	.L60
	movq	-216(%rbp), %rax
	movl	%eax, %edx
	movq	-216(%rbp), %rax
	movl	%eax, %ecx
	movl	-372(%rbp), %eax
	subl	%ecx, %eax
	movl	%edx, %esi
	movl	%eax, %edi
	call	__kfifo_max_r
	jmp	.L62
.L60:
	movl	$0, %eax
	jmp	.L62
.L59:
	movl	-372(%rbp), %eax
.L62:
	movl	%eax, %edi
	call	__kfifo_uint_must_check_helper
	movl	%eax, -376(%rbp)
	movl	-376(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC1(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	leaq	-144(%rbp), %rax
	movq	%rax, -200(%rbp)
	movq	-200(%rbp), %rax
	movl	(%rax), %edx
	movq	-200(%rbp), %rax
	movl	4(%rax), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -376(%rbp)
	movl	-376(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC2(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	jmp	.L63
.L66:
	leaq	-144(%rbp), %rax
	movq	%rax, -192(%rbp)
	leaq	-112(%rbp), %rax
	movq	%rax, -184(%rbp)
	movq	$100, -176(%rbp)
	movq	$2, -168(%rbp)
	movq	-192(%rbp), %rax
	movq	%rax, -160(%rbp)
	cmpq	$0, -168(%rbp)
	je	.L64
	movq	-176(%rbp), %rax
	movl	%eax, %edi
	movq	-168(%rbp), %rdx
	movq	-184(%rbp), %rsi
	movq	-160(%rbp), %rax
	movq	%rdx, %rcx
	movl	%edi, %edx
	movq	%rax, %rdi
	call	__kfifo_out_r
	jmp	.L65
.L64:
	movq	-176(%rbp), %rax
	movl	%eax, %edx
	movq	-184(%rbp), %rcx
	movq	-160(%rbp), %rax
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	__kfifo_out
.L65:
	movl	%eax, %edi
	call	__kfifo_uint_must_check_helper
	movl	%eax, -376(%rbp)
	movl	-376(%rbp), %edx
	leaq	-112(%rbp), %rax
	movq	%rax, %rsi
	leaq	.LC3(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	leaq	-144(%rbp), %rax
	movq	%rax, -152(%rbp)
	movq	-152(%rbp), %rax
	movl	(%rax), %edx
	movq	-152(%rbp), %rax
	movl	4(%rax), %eax
	subl	%eax, %edx
	movl	%edx, %eax
	movl	%eax, -376(%rbp)
	movl	-376(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC4(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
.L63:
	cmpl	$0, -376(%rbp)
	jne	.L66
	movl	$0, %eax
	movq	-8(%rbp), %rcx
	xorq	%fs:40, %rcx
	je	.L68
	call	__stack_chk_fail@PLT
.L68:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE25:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 7.5.0-3ubuntu1~18.04) 7.5.0"
	.section	.note.GNU-stack,"",@progbits
