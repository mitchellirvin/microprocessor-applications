; Lab4_serial_int.asm
; Lab 4 Part F
; Name: Mitchell Irvin
; Section: 1E83
; TA Name: Khaled Hassan 
; Description: this program is an interrupt driven echo system

;Definitions for all the registers in the processor. ALWAYS REQUIRED.
.include "ATxmega128A1Udef.inc"

.equ BSel = 9			; these values for 38400Hz yield error = 0.16
.equ BScale = -2		; 38400 Hz
.equ Prompt = '?'
.equ StringLength = 10

.set IO_PORT = 0x4000			; Set address start to beginning of external memory
.set IO_PORT_END = 0x4FFF		; might as well be a comment, going to use 4k CS memory

.org USARTD0_RXC_vect		;using reg def from included file, this is the address of
							;our interrupt vector (receive confirmed vector 
							;from portD's USART functionality)
	jmp EXT_INT_ISR		;when our interrupt0 occurs, rjmp to our ISR

.org 0x0000	
	rjmp MAIN

.cseg
.org 0x0200
MAIN:
	ldi YL, 0xFF	;initialize low byte of stack pointer
	out CPU_SPL, YL
	ldi YL, 0x3F
	out CPU_SPH, YL	

	ldi R16, 0x01			;load R16 with 0b 00 (RREN and INVEN disabled) 00 0 (reserved) 
							; 0 (HILVLEN disabled) 0 (MDLVLEN disabled) 1 (LOLVLEN enabled)
	sts PMIC_CTRL, R16		; set programmable multilevel interrupt controller to enable low level interrupts

	sei			;enable global interrupt

REPEAT:
;	rcall DELAY_500ms	; This can help with extra characters being displayed
	call INIT_GPIO
	call INIT_EBI
	call USART_INIT
	ldi R16, 0x00

TOGGLELED:
	st X, R16		;light up LED
	com R16		;toggle value
	ldi R21, 0x32
	call DELAYx10ms		;delay for .5s
	rjmp TOGGLELED			;repeat

USART_INIT:
	ldi R16, 0x10				
	sts USARTD0_CTRLA, R16		;enable low level receiver interrupt w/ CTRLA

	ldi R16, 0x18
	sts USARTD0_CTRLB, R16		;turn on TXEN, RXEN lines, bits 4 and 3

	ldi R16, 0x03				
	sts USARTD0_CTRLC, R16		;Set Parity to none, 8 bit frame, 1 stop bit

	ldi R16, (BSel & 0xFF)		;select only the lower 8 bits of BSel
	sts USARTD0_BAUDCTRLA, R16	;set baudctrla to lower 8 bites of BSel 

	ldi R16, ((BScale << 4) & 0xF0) | ((BSel >> 8) & 0x0F)							
	sts USARTD0_BAUDCTRLB, R16	;set baudctrlb to BScale | BSel. Lower 
								;  4 bits are upper 4 bits of BSel 
								;  and upper 4 bits are the BScale. 
	ret

INIT_GPIO:
	; portD bits 3:2 are TXD, RXD respectively
	; TXD is transmit data buffer, RXD is receive data buffer
	ldi R16, 0x04				; pin 2 is receive = input
	sts PortD_DIRCLR, R16		; set pin 2 as input

	ldi R16, 0x08				;pin 3 is transmit = output
	sts PortD_DIRSET, R16		; set pin 3 as output
	sts PortD_OUTSET, R16		; set activation level as high (pin3)
	
	; PORTQ pins 3 and 1 are USB switch SEL and USB switch ENable respectively
	; enable is low true, SEL is high true. we want to set both to 0, to enable
	; the output, and to select FTDI_D- and FTDI_D+ as the outputs of the USB switch
	ldi R16, 0x0A				; pins3 and 1 high
	sts PORTQ_DIRSET, R16		; Set pins 3 and 1 as output
	sts PORTQ_OUTCLR, R16				; pins 3 and 1 low
								; set PQ3 and PQ1 as low (enable true, sel off (FTDI))

	ret

INIT_EBI:
;begin standard 3port/ALE1/SRAM initializations
	ldi R16, 0x17		;Configure the PORTH bits 0,1,2,4 as outputs. 
	sts PORTH_DIRSET, R16 	;  These are the CS0(L), ALE1(H), WE(L), and RE(L) outputs. 
							;  (CS0 is bit 4; ALE1 is bit 2; RE is bit 1; WE is bit 0)

	ldi R16, 0x13		; Since RE(L), WE(L) and CS0(L) are active low, we must set  
	sts PORTH_OUTSET, R16	; the default output to 1 = H = false. 
							; -> bits 4,1,0 high, bit 2 low, all others low = 0001 0011
	
	ldi R16, 0xFF			; Set all PORTK pins (A15-A0) to be outputs. As requried	
	sts PORTK_DIRSET, R16	; in the data sheet. See 8331, sec 27.9. 
							; only 8 bits b/c A7:0 mux'd with A15:8

	ldi R16, 0xFF			;Set all PORTJ pins (D7-D0) to be outputs. As requried 
	sts PORTJ_DIRSET, R16	;  in the data sheet. See 8331, sec 27.9.

	ldi R16, 0x01			;Store  in EBI_CTRL register to select 3 port EBI(H,J,K) 
	sts EBI_CTRL, R16		;  mode and SRAM ALE1 mode. SRAM ALE1 mode is bits 3 and 2 = 00
							; 3PORT EBI mode is bits 1 and 0 = 01 therefore lower 4 bits = 0x1

;Reserve a chip-select zone for our input port. The base address register is made up of
;  12 bits for the address (A23:A12). The lower 12 bits of the address (A11-A0) are 
;  assumed to be zero. This limits our choice of the base addresses.

;Initialize the Z pointer to point to the base address for chip select 0 (CS0) in memory
	ldi ZH, high(EBI_CS0_BASEADDR)
	ldi ZL, low(EBI_CS0_BASEADDR)
	
;Load the middle byte (A15:8) of the three byte address into a register and store it as the 
;  LOW Byte of the Base Address, BASEADDRL.  This will store only bits A15:A12 and ignore 
;  anything in A11:8 as again, they are assumed to be zero. We increment the Z pointer 
;  so that we can load the upper byte of the base address register.
	ldi R16, byte2(IO_PORT)				
	st Z+, R16							; 

;Load the highest byte (A23:16) of the three byte address into a register and store it as the 
;  HIGH byte of the Base Address, BASEADDRH.
	ldi R16, byte3(IO_PORT)
	st Z, R16

	ldi R16, 0x11		; Set to 4K chip select space and turn on SRAM mode, 0x28 8000 - 0x28 9FFF
							; in manual under EBI CS ctrl register A: bit 7 = 0, bits 6:2 are ASIZE, 4k = 00100
							; bits 1:0 are mode, SRAM = 01 therefore, load R16 with 00010001
	sts EBI_CS0_CTRLA, R16	
	
	ldi XH, high(IO_PORT)				; set the middle (XH) and low (XL) bytes of the pointer as usual
	ldi XL, low(IO_PORT)

	ret	

OUT_CHAR:
	push R17

TX_POLL:
	lds R17, USARTD0_STATUS		;load status register
	sbrs R17, 5				;proceed to writing out the char if
								;  the DREIF flag is set (bit 5)
	rjmp TX_POLL				;else go back to polling
	sts USARTD0_DATA, R16		;send the character out over the USART
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
	lds R16, USARTD0_STATUS		;load the status register
	sbrs R16, 7				;proceed to reading in a char if the receive flag is set
							; if bit 7 is 1, there is unread data in the receive buffer
	rjmp RX_POLL				;else continue polling
	lds R16, USARTD0_DATA		;read the character into R16

	ret

EXT_INT_ISR:
	;push any registers that might be holding sensitive values to the stack
	;so we don't accidentally change them
	push R16
	push R17
	push R18
	push R19
	push R20
	push R21

	lds R16, CPU_SREG		;need to push because it may change below
	push R16

	call IN_CHAR		;read input character
	call OUT_CHAR		;echo character

	ldi	 R17, 0x80		;pin7 is the NMI flag, must clear before returning
	sts  USARTD0_STATUS, R17	; Clear the USARTD0's non-maskable interrupt flag, 
								; not necessary but "can't hurt"
	pop R16
	sts CPU_SREG, R16	;restore CPU_SREG value

	;pop reg values previously pushed back into place
	pop R21
	pop R20
	pop R19
	pop R18
	pop R17
	pop R16

	reti

;program to delay by x times 10ms, where x is the val in R21
DELAYx10ms:
	;load 0x14 into R20, this value will allow for the standard 10ms delay
	ldi R20, 0x14
	DELAY10ms:
		;load 0xFF into R19. when R19 is decremented to 0, reset to 0xFF
		;and decremented again over and over (0x14 times) the delay is 10ms
		ldi R19, 0xFF
		SUBDELAY:
			dec R19
			cpi R19, 0x00
			brne SUBDELAY
		dec R20
		cpi R20, 0x00
		brne DELAY10ms
	;do the 10ms delay R21 number of times
	dec R21
	cpi R21, 0x00
	brne DELAYx10ms
	ret