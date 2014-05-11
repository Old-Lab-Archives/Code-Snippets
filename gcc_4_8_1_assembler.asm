.data

x:

.long	0

s:

.string "%d\n\0"

.text

.global main

main:				

loop:			

pushl	$x

pushl	$s

call	scanf

addl	$8, %esp

movl	x, %eax

subl	$42, %eax

jz	break

pushl	x

pushl	$s

call	

addl	$8, %esp

jmp	loop	

break:

xor	%eax, %eax

ret



