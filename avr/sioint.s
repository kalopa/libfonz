;
;
UDR=		0x0c
UCSRA=		0x0b
UCSRB=		0x0a
UCSRC=		0x20			;  Note! UCSRC equals UBRRH
UBRRL=		0x09
SREG=		0x3f
;
; void _fonz_in();
;
; Serial I/O input interrupt routine (RX).
	.section .init6,"ax",@progbits
	.global	_fonz_in
	.func	_fonz_in
_fonz_in:
	push	r24					; Save the status register
	in		r24,SREG
	push	r24

	push	r18
	push	r19
	push	r20
	push	r21
	push	r22
	push	r23
	push	r25
	push	r26
	push	r27
	push	r30
	push	r31
;
	in		r24,UDR				; Pull the character
	rcall	fp_indata			; Process it
;
	rjmp	fonz2				; We're done
	.endfunc
;
; void _fonz_out();
;
; Serial I/O output interrupt routine (TX).
	.global	_fonz_out
	.func	_fonz_out
_fonz_out:
	push	r24					; Save the status register
	in		r24,SREG
	push	r24

	push	r18
	push	r19
	push	r20
	push	r21
	push	r22
	push	r23
	push	r25
	push	r26
	push	r27
	push	r30
	push	r31
;
	rcall	fp_outdata			; Is there data to send?
	out	UDR,r24
	sbrs	r25,7
	rjmp	fonz1				; Yes - Go do that
	cbi		UCSRB,5				; NO - disable TX ints
	rjmp	fonz2
;
fonz1:	out		UDR,r24				; Transmit the character
;
; Common return routine from interrupts.
fonz2:	pop		r31
	pop		r30
	pop		r27
	pop		r26
	pop		r25
	pop		r23
	pop		r22
	pop		r21
	pop		r20
	pop		r19
	pop		r18
;
	pop		r24					; Restore the status register
	out		SREG,r24
	pop		r24
	reti						; Return from interrupt
	.endfunc
;
; void _fp_rxinton();
;
; Turn on serial receive interrupts.
	.global	_fp_rxinton
	.func	_fp_rxinton
_fp_rxinton:
	sbi		UCSRB,7				; Enable RX ints
	ret
	.endfunc
;
; void _fp_rxintoff();
;
; Turn off serial receive interrupts.
	.global	_fp_rxintoff
	.func	_fp_rxintoff
_fp_rxintoff:
	cbi		UCSRB,7				; Disable RX ints
	ret
	.endfunc
;
; void _fp_txinton();
;
; Turn on serial transmit interrupts.
	.global	_fp_txinton
	.func	_fp_txinton
_fp_txinton:
	sbi		UCSRB,5				; Enable TX ints
	ret
	.endfunc
;
; void _fp_txintoff();
;
; Turn off serial transmit interrupts.
	.global	_fp_txintoff
	.func	_fp_txintoff
_fp_txintoff:
	cbi		UCSRB,5				; Disable TX ints
	ret
	.endfunc
;
; Fin.
