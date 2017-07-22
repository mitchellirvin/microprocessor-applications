; Lab4_serial_baud_test.asm
; Lab 4 Part D
; Name: Mitchell Irvin
; Section: 1E83
; TA Name: Khaled Hassan 
; Description: this program will send and receive data between
; the uPad and the user's machine using USARTE0, and test to confirm	
; that the correct baud rate (38400) is being used

;Definitions for all the registers in the processor. ALWAYS REQUIRED.
.include "ATxmega128A1Udef.inc"

.equ BSel = 9			; these values for 38400Hz yield error = 0.16
.equ BScale = -2		; 38400 Hz

.org 0x0000	
	rjmp MAIN

.cseg
.org 0x0200
MAIN:
	ldi YL, 0xFF	;initialize low byte of stack pointer
	out CPU_SPL, YL
	ldi YL, 0x3F
	out CPU_SPH, YL	
	rjmp INIT

INIT:
	call INIT_GPIO
	call USART_INIT
	rjmp LOOP

LOOP:
	ldi R16, 'U'	;load R16 for OUT_CHAR b/c it expects arguments on the stack
	call OUT_CHAR

	ldi R21, 0x02	;set delay for 2 * 500us
	call DELAYx500us	;delay 2 * 500us

	rjmp LOOP			;repeat

USART_INIT:
	ldi R16, 0x18
	sts USARTE0_CTRLB, R16		;turn on TXEN, RXEN lines, bits 4 and 3

	ldi R16, 0x03				
	sts USARTE0_CTRLC, R16		;Set Parity to none, 8 bit frame, 1 stop bit

	ldi R16, (BSel & 0xFF)		;select only the lower 8 bits of BSel
	sts USARTE0_BAUDCTRLA, R16	;set baudctrla to lower 8 bites of BSel 

	ldi R16, ((BScale << 4) & 0xF0) | ((BSel >> 8) & 0x0F)							
	sts USARTE0_BAUDCTRLB, R16	;set baudctrlb to BScale | BSel. Lower 
								;  4 bits are upper 4 bits of BSel 
								;  and upper 4 bits are the BScale. 
	ret

INIT_GPIO:
	; portE bits 3:2 are TXD, RXD respectively
	; TXD is transmit data buffer, RXD is receive data buffer
	ldi R16, 0x04				; pin 2 is receive = input
	sts PORTE_DIRCLR, R16		; set pin 2 as input

	ldi R16, 0x08				;pin 3 is transmit = output
	sts PORTE_DIRSET, R16		; set pin 3 as output
	sts PORTE_OUTSET, R16		; set activation level as high (pin3)

	; PORTQ pins 3 and 1 are USB switch SEL and USB switch ENable respectively
	; enable is low true, SEL is high true. we want to set both to 0, to enable
	; the output, and to select FTDI_D- and FTDI_D+ as the outputs of the USB switch
	ldi R16, 0x0A				; pins3 and 1 high
	sts PORTQ_DIRSET, R16		; Set pins 3 and 1 as output
	sts PORTQ_OUTCLR, R16				; pins 3 and 1 low
								; set PQ3 and PQ1 as low (enable true, sel off (FTDI))

	ret

OUT_CHAR:
	push R17

TX_POLL:
	lds R17, USARTE0_STATUS		;load status register
	sbrs R17, 5				;proceed to writing out the char if
								;  the DREIF flag is set (bit 5)
	rjmp TX_POLL				;else go back to polling
	sts USARTE0_DATA, R16		;send the character out over the USART
	pop R17

	ret

OUT_STRING:
	lpm R16, Z+ 		;read character pointed to by Z and inc Z
	cpi R16, 0x00		;check if char is null
	breq RETURN			;if char is null return from subroutine
	rcall OUT_CHAR		;char is not null, call OUT_CHAR
	rjmp OUT_STRING		;repeat for each non-null char

RETURN:
	ret 

IN_CHAR:

RX_POLL:
	lds R16, USARTE0_STATUS		;load the status register
	sbrs R16, 7				;proceed to reading in a char if the receive flag is set
							; if bit 7 is 1, there is unread data in the receive buffer
	rjmp RX_POLL				;else continue polling
	lds R16, USARTE0_DATA		;read the character into R16

	ret

;subroutine to delay by 500us * R20
DELAYx500us:
	;load 0xFF into R19. when R19 is decremented to 0, reset to 0xFF
	;and decremented again over and over (0x14 times) the delay is 10ms
	ldi R19, 0xFF
	SUBDELAY:
		dec R19
		cpi R19, 0x00
		brne SUBDELAY
	dec R20
	cpi R20, 0x00
	brne DELAYx500us

	ret