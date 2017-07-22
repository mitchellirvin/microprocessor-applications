; Lab4_ext_intr.asm
; Lab 4 Part A
; Name: Mitchell Irvin
; Section: 1E83
; TA Name: Khaled Hassan 
; Description: this program will use an external interrupt and create
; an ISR to display a count of how many times the interrupt has executed
; and output that to the LED array

;Definitions for all the registers in the processor. ALWAYS REQUIRED.
.include "ATxmega128A1Udef.inc"

.set IO_PORT = 0x4000			; Set address start to beginning of external memory
.set IO_PORT_END = 0x4FFF		; might as well be a comment, going to use 4k CS memory

.org 0x0000	
	rjmp MAIN

.org PORTE_INT0_vect		;using reg def from included file, this is the address of
							;our interrupt vector (interrupt0 for PORTE)
	jmp EXT_INT_ISR		;when our interrupt0 occurs, rjmp to our ISR

.org 0x200
MAIN:
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

	rcall INIT_INTERRUPT

	ldi XH, high(IO_PORT)				; set the middle (XH) and low (XL) bytes of the pointer as usual
	ldi XL, low(IO_PORT)
	
	;init the stack pointer to 0x3FFF (top of internal SRAM)
	ldi YL, 0xFF	
	out CPU_SPL, YL
	ldi YL, 0x3F
	out CPU_SPH, YL	

	ldi R25, 0x00	;interrupt counter, starts at 0

LOOP:			;loop forever while the interrupt fires
	st X, R25
	rjmp LOOP

INIT_INTERRUPT:
; now begin the interrupt initializations
	ldi R16, 0x01		; load R16 with 0b0000 0001 b/c we're enabling interrupt 0 (bits 0 and 1) with a low level priority (01)
						; via the PORTE_INTCTRL control register, see doc8331 13.13.10
	
	sts PORTE_OUT, R16		;set output to default to 1
	sts PORTE_DIRCLR, R16	;set pin0 as input

	sts PORTE_INTCTRL, R16		; enable PORTE interrupt 0
	sts PORTE_INT0MASK, R16		;store 0x01 to interrupt 0 mask register, marking pin0 as interrupt source

	ldi R16, 0x02			;load R16 with 0x02 because pins2:0 are input sense config (010 = falling edge)
								;pins5:3 000 for totem pole, pins 7 and 6 0 because INVEN and RREN are set to 0
	sts PORTE_PIN0CTRL, R16		;set pin0 of portE to sense on falling edge

	ldi R16, 0x01			;load R16 with 0b 00 (RREN and INVEN disabled) 00 0 (reserved) 
							; 0 (HILVLEN disabled) 0 (MDLVLEN disabled) 1 (LOLVLEN enabled)
	sts PMIC_CTRL, R16		; set programmable multilevel interrupt controller to enable low level interrupts

	sei			;enable global interrupt
	ret

; interrupt service routine for external interrupt
EXT_INT_ISR:
	lds R16, CPU_SREG		;need to push because it may change below
	push R16

	;shortest delay possible inside the ISR
	ldi R21, 0x0A		;set x to be 10, delay 10 * 10ms
	rcall DELAYx10ms	;delay by .1s

	;make sure we're on a falling edge 
	lds R16, PortE_IN	;read PORTE
	andi R16, 0x01		;AND with 0000 0001 to check the 0th bit
	sbrc R16, 0		;if the 0th bit was high, then we're on a rising edge, don't increment
	rjmp NOINC			
	inc R25		;otherwise it's a falling edge, increment R25

NOINC:
	ldi	 R17, 0x01
	sts  PORTE_INTFLAGS, R17	; Clear the PORTE_INTFLAG, not necessary but "can't hurt"

	pop R16					;pop and restore CPU_SREG to prev value
	sts CPU_SREG, R16
	reti		;special return from interrupt directive

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