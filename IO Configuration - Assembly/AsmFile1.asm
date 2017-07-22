/*	Lab3
	Ryan Henry
	7F32
	Austin
	This program is designed to set up an external I/O port.
	It then reads in values from the port and outputs them to an LED.
	It shifts the LEDs Right 8 times with a 2 second delay inbetween.
	Then it reads in from the input port again and repeats.
 */
.include "ATxmega128A1Udef.inc"
;******************************INITIALIZATIONS***************************************
.set IOPORT = 0x4000	; This could be a 22-bit address, e.g., 0x37 E000
.set IPORTEND = 0x4FFF

.org 0x0000					;Place code at address 0x0000
	rjmp MAIN				;Relative jump to start of program

.org 0x200
MAIN:
	ldi R16, 0x17		;Configure the PORTH bits 4,2,1 and 0 as outputs. 
	sts PORTH_DIRSET, R16 	;  These are the CS0(L), ALE1, RE(L), and WE(L) outputs. 
							
	ldi R16, 0x13			;For active low signals, we must set the 
	sts PORTH_OUTSET, R16	;default output to 1=H=false. See 8331, sec 27.9.
	
	ldi R16, 0xFF			;Set all PORTJ pins (D7-D0) to be outputs. 
	sts PORTJ_DIRSET, R16	

	ldi R16, 0xFF			;Set all PORTK pins (A15-A0) to be outputs. 	
	sts PORTK_DIRSET, R16	
	
	ldi R16, 0x01			;Store 0x01 in EBI_CTRL register to select 3 port EBI(H,J,K) 
	sts EBI_CTRL, R16		;mode and SRAM ALE1 mode.
		
;Reserve a chip-select zone for our input port. The base address register is made up of
;  12 bits for the address (A23:A12). The lower 12 bits of the address (A11-A0) are 
;  assumed to be zero. This limits our choice of the base addresses.
	ldi ZH, high(EBI_CS0_BASEADDR)
	ldi ZL, low(EBI_CS0_BASEADDR)

	ldi R16, low(IOPORT>>8)	;Store the LOW Byte of the Base Address, BASEADDRL. 
	st Z+, R16				;  Actually, this will store only bits A15:A12; 
							;  A11:A8 are ignored
; We increment the Z pointer so that we can load the upper byte of the 
;   base address register next.
; We only choose the upper 12 bits (of the 24-bits), A23:A12
;   of the address. When we shift 16 bits right (below), this will leave A23:A16 to
;   be copied into the high part of the Base Address, BASEADDRH.
		
	ldi R16, (IOPORT>>16)	;Store the UPPER byte (A23:16) into the upper byte
	st Z, R16				;  of the Base Address register, BASEADDRH.		

	ldi R16, 0b0010001			;Set to 4K chip select space and turn on SRAM mode
	sts EBI_CS0_CTRLA, R16		;address space of the input port will be
								;0x4000 to 0x4FFF


	ldi XH, high(IOPORT)		;Set X register to address $4000 se we can use it with EBI
	ldi XL, low(IOPORT)			;to input and output

	Test:

	ld R20, X					;Read inputs from Data7:0
	ldi R21, 8					;Initialize counter to keep track of shifts
	
	Shift8:
	
	st X, R20					;Output data to external port
	ldi R16, 200				;Ld R16 with 200 so that delays will be 2s
	
	rcall delayx10ms			;call subroutine
	
	LSR R20						;Shift register contents right 
	st X, R20					;output register
	
	dec R21						;count down the number of times you ned to shift
	cpi R21, 1					;if you've shifted less than 8 times, jmp back
	brsh Shift8

	rjmp test					;jmp back to read input again



	delayx10ms:			;r16 is passed into this subroutine
						;this subroutine delays the program by r16 x 10ms
						;r16,r17,r18 are altered
	i:
	ldi r18, 49			;outside loop, runs the 10ms delay r16 times
	dec r16
	cpi r16, 1
	brsh j
	ret

	j:
	rjmp delay_200us

	k:
	dec r18				;middle loop, runs the 200us delay 50 times
	cpi r18, 1
	brsh j
	rjmp i

	delay_200us:

	ldi r17, 99			;inner loop, delays the processor 200us
	here:
	dec r17
	cpi r17, 1
	brsh here
	rjmp k