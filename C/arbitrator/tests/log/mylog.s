	.file	"add0.5.c"
	.version	"01.01"
gcc2_compiled.:
		.section	.rodata
	.align 8
.LC0:
	.long	0x0,0x3fe00000
.text
	.align 16
.globl add_0_5
	.type	 add_0_5,@function
add_0_5:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	movl	12(%ebp), %edx
	movl	%eax, -8(%ebp)
	movl	%edx, -4(%ebp)
	fldl	-8(%ebp)
	fldl	.LC0
	faddp	%st, %st(1)
	movl	%ebp, %esp
	popl	%ebp
	ret
.Lfe1:
	.size	 add_0_5,.Lfe1-add_0_5
	.ident	"GCC: (GNU) 2.96 20000731 (Linux-Mandrake 8.0 2.96-0.48mdk)"
