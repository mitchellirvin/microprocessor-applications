/*
 * Lab4Quiz.asm
 *
 *  Created: 10/17/2016 1:39:39 PM
 *   Author: Mitch
 */ 

 ;set baud rate to be 28,800 bps
 .equ BSEL = 427
 .equ BSCALE = -7
 ;bits necessary for new line
 .equ CR = 0x0D 
 .equ LF = 0x0A 

.org 0x0000
	rjmp MAIN

.org 0x0200
MAIN:
	;init GPIO and USART regs
	rcall INIT_GPIO
	rcall INIT_USART
	jmp LOOP

INIT_USART:
	;set receive and transmit bits
	ldi R16, 0x18
	sts USARTD0_CTRLB, R16		

	;set parity to none, 8 bit character size, 1 stop bit
	ldi R16, 0x03				
	sts USARTD0_CTRLC, R16		

	;ctrla gets lower 8 bits of BSel
	ldi R16, (BSEL & 0xFF)		
	sts USARTD0_BAUDCTRLA, R16	

	;lower 4 bits = upper 4 of BSel, upper 4 bits = Bscale
	ldi R16, ((BSCALE << 4) & 0xF0) | ((BSEL >> 8) & 0x0F)							
	sts USARTD0_BAUDCTRLB, R16	 

	ret

INIT_GPIO:
	ldi R16, 0x04	;bit 2 needs to be input
	sts PORTD_DIRCLR, R16

	ldi R16, 0x08	;bit 3 needs to be output, set high true
	sts PORTD_DIRSET, R16
	sts PORTD_OUTSET, R16

	ldi R16, 0x0A			;set pins 3 and 1 low, enable and sel
	sts PORTQ_DIRSET, R16
	sts PORTQ_OUTCLR, R16

	ret

LOOP:
	;print "Hello\n"
	ldi R16, 'H'
	rcall OUT_CHAR
	ldi R16, 'e'
	rcall OUT_CHAR
	ldi R16, 'l'
	rcall OUT_CHAR
	ldi R16, 'l'
	rcall OUT_CHAR
	ldi R16, 'o'
	rcall OUT_CHAR
	ldi R16, CR
	rcall OUT_CHAR
	ldi R16, LF
	rcall OUT_CHAR

	rjmp LOOP

OUT_CHAR:

TXPOLL:
	lds R17, USARTD0_STATUS	;load R17 with status register
	sbrs R17, 5			;check if transmit bit is set (so we can transmit the data)
	rjmp TXPOLL			;if it's not, try again
	sts USARTD0_DATA, R16		;if it is, store R16 in the data reg

	ret
	