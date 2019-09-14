# 1 "isqrt.S"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "isqrt.S"
;;; Alex Pollock
;;; 115332645
;;; apolloc1

;;; Implementation that computes the square root of an integer.

 .global isqrt ;exposes isqrt as a label for the linker
isqrt:

 cpi r24, 2 ;Compare integer to 2
 brlt 1f ;Branch if less than 2

 lsr r24 ;Shift bits to the right 1 position
 lsr r24 ;Shift bits to the right 1 position
 movw r20, r24 ;
 cpi r24, 2 ;
 brlt 1f ;

 movw r18, r20 ;Copy small to r18
 movw r26, r20 ;Create the large integer
 inc r26 ;Increment large value by 1
 movw r30, r26 ;Copy small + 1 to large
 movw r20, r30
 mul r26, r30 ;Multiply large by large
 movw r30, r0 ;Copy result back to r30
 cp r30, r18 ;Compare large * large to parameter
 brge 2f ;Break if greater than parameter

 movw r24, r26 ;Copy large value to return register
 ret ;Return large


1: ret ;Return integer

2: clr r25
 mov r24, r18 ;Copy small to return register
 ret ;Return small
