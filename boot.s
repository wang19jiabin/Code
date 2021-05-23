.code16
.globl _start

_start:
	movw	$0x07c0, %ax
	movw	%ax, %es
	movw	$s, %bp
	movw	$5, %cx
	movw	$0x1301, %ax
	movw	$0x000c, %bx
	int	$0x10

loop:
	jmp	loop

s:
	.ascii	"hello"
	.org	510
	.word	0xaa55
