;;; Alex Pollock
;;; 115332645
;;; apolloc1

;;; Implementation that computes the nth fibonacci number.

    .global fib                 ; exposes fib as a label for the linker
fib:                            ; the label: the next instruction starts fib() function.
	;; Add your code here.

	cpi r24, 0		;Compare value to 0
	breq exit0		;Break to return if 0
	cpi r24, 1		;Compare value to 1
	breq exit1		;Break to return if 1

	
	mov r20, r24		;Copy value into r20
	push r20		;Push r20 onto the stack to save value
	dec r24 		;Decrement r24 by 1
	call fib		;Recursively call function for value - 1
	movw r22, r24		;Copy value to r22
	pop r20			;Pop r20 off stack 
	push r22		;Push r22 and r23 onto the stack
	push r23		
	movw r24, r20		;Copy r20 to r24
	dec r24			;Decrement value by 2
	dec r24
	call fib		;Recursively call function for value - 2
	pop r23 		;Pop r23 and r22 off stack
	pop r22
	adc r24, r22		;Add r22 and r24
	adc r25, r23		;Add r25 and 23
	ret			;Return
	
exit0:
	ret                     ;Return 0

exit1:
	ret			;Return 1
