; lab3.asm
; Lab 3
; Name: Mitchell Irvin
; Section: 1E83
; TA Name: Khaled Hassan 
; Description: this code creates an external I/O port and uses it
; to read a value, shift it left or right (if it's even or odd, 
; respectively) 8 times, with a 0.5s delay between each shift
; then read input again and repeat infinitely 

;Definitions for all the registers in the processor. ALWAYS REQUIRED.
.include "ATxmega128A1Udef.inc"

.set IO_PORT = 0x288000		; Set address start
.set IO_PORT_END = 0x289FFF	; set address end for documentation purposes 

.org 0x0000	
	rjmp MAIN

.org 0x200
MAIN:
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

	ldi R16, 0x15		; Set to 8K chip select space and turn on SRAM mode, 0x28 8000 - 0x28 9FFF
							; in manual under EBI CS ctrl register A: bit 7 = 0, bits 6:2 are ASIZE, 8k = 00101
							; bits 1:0 are mode, SRAM = 01 therefore, load R16 with 00010101
	sts EBI_CS0_CTRLA, R16					

	ldi R16, 0x28			; initalize a pointer to point to the base address of the IN_PORT
	sts CPU_RAMPX, r16					; use the CPU_RAMPX register to set the third byte of the pointer

	ldi XH, high(IO_PORT)				; set the middle (XH) and low (XL) bytes of the pointer as usual
	ldi XL, low(IO_PORT)

START:
	ld R16, X							; read the input port into R16
	
	ldi R22, 0x08		;load shift counter into R22
	ldi R23, 0x01		;load even checker into R23
	;if even, shift left 8 times every 0.5s then read from input again
	;if odd, shift right 8 times every ~0.5s then read from input again
	mov R24, R16	;copy R16 into R24
	and R24, R23	;AND the input value with 0x01
	cp R24, R23		;if input value is now equal to 0x01 then the number was odd
	breq SHIFTRIGHT		;the value was odd so branch to shiftright
	rjmp SHIFTLEFT		;the value was even so branch to shiftleft

SHIFTLEFT:
	st X, R16				; write to external output
	ldi R21, 0x32			;set value such that delay will be 50 * 10ms = .5s
	rcall DELAYx10ms		;delay
	rol R16					;rotate left
	brcs CARRYLEFT
	rjmp FINISHLEFT

CARRYLEFT:
	ldi R21, 0x01
	add R16, R21 
	rjmp FINISHLEFT

FINISHLEFT:
	cpi R22, 0x00			;if we've shifted 8 times
	breq START				;branch back to start for another input value
	dec R22					;decrement shift counter
	rjmp SHIFTLEFT			;otherwise shift left again

SHIFTRIGHT:
	st X, R16				; write to external output
	ldi R21, 0x32			;set value such that delay will be 50 * 10ms = .5s
	rcall DELAYx10ms		;delay
	ror R16					;rotate right
	brcs CARRYRIGHT
	rjmp FINISHRIGHT

CARRYRIGHT:
	ldi R21, 0x80
	add R16, R21 
	rjmp FINISHRIGHT

FINISHRIGHT:
	cpi R22, 0x00			; if we've shifted 8 times
	breq START				;then branch back to start for another input value
	dec R22					;decrement shift counter
	rjmp SHIFTRIGHT			;otherwise shift right again


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