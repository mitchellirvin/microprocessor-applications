; Lab4_serial_menu.asm
; Lab 4 Part E
; Name: Mitchell Irvin
; Section: 1E83
; TA Name: Khaled Hassan 
; Description: this program will display a list of categories to the user,
; with each category numbered. When the user sends one of the corresponding
; numbers, the program will respond with "Mitch's favorite ____ is _____". 

;Definitions for all the registers in the processor. ALWAYS REQUIRED.
.include "ATxmega128A1Udef.inc"

.equ BSel = 9			; these values for 38400Hz yield error = 0.16
.equ BScale = -2		; 38400 Hz
.equ CR = 0x0D 
.equ LF = 0x0A 
.equ ESC = 0x1B 

.org 0x0000	
	jmp MAIN

; need to use EEPROM memory (1000-17FF) for storing our strings in data memory
; store all the strings we need for our "Favorites" program to work
.org 0x1000
MENU: .db "Mitch's favorite:", CR, LF, \
"1. OS/Computer", CR, LF, \
"2. EE/CE Course", CR, LF, \
"3. Hobby", CR, LF, \
"4. Quote", CR, LF, \
"5. Movie", CR, LF, \
"6. Re-display Menu", CR, LF, \
"ESC: exit", CR, LF, CR, LF, '\0'

.org 0x1070
COMP: .db "Mitch's favorite OS/Computer is Windows/PC", CR, LF, CR, LF, '\0'

.org 0x1100
COURSE: .db "Mitch's favorite EE/CE Course is Digital Logic", CR, LF, CR, LF, '\0'

.org 0x1150
HOBBY: .db "Mitch's favorite Hobby is weightlifting", CR, LF, CR, LF, '\0'

.org 0x1200
QUOTE: .db "Mitch's favorite Quote is: We made Iran great again. -Trump", CR, LF, CR, LF, '\0'

.org 0x1250
MOVIE: .db "Mitch's favorite Movie is Edge of Tomorrow", CR, LF, CR, LF, '\0'

.org 0x1300
TERMINATE: .db "Done!", '\0'

.org 0x0200
MAIN:
	ldi YL, 0xFF	;initialize low byte of stack pointer
	out CPU_SPL, YL
	ldi YL, 0x3F
	out CPU_SPH, YL	

	call INIT_GPIO		;init GPIO ports and USART
	call USART_INIT

LOOP:
	ldi ZL, low(MENU << 1)		;point Z at MENU label in memory
	ldi ZH, high(MENU << 1)

	call OUT_STRING			;output the string where Z is pointing

INVALIDCHAR:
	;wait for character to be input
	call IN_CHAR

	cpi R16, '1'	;if user input 1, branch to ONE
	breq ONE

	cpi R16, '2'	;if user input 2, branch to TWO
	breq TWO

	cpi R16, '3'	;if user input 2, branch to THREE
	breq THREE

	cpi R16, '4'	;if user input 2, branch to FOUR
	breq FOUR

	cpi R16, '5'	;if user input 2, branch to FIVE
	breq FIVE

	cpi R16, '6'	;if user input 6, LOOP
	breq LOOP

	cpi R16, ESC	;if user input ESC, quit
	breq DONE

	rjmp INVALIDCHAR		;user didn't input a valid character, jump to 
							;INVALID CHAR to receive input again

ONE:
	ldi ZL, low(COMP << 1)		;point Z to response string 1
	ldi ZH, high(COMP << 1)

	call OUT_STRING		;print string at location Z is pointing to
	jmp LOOP			;display menu again and wait for input

TWO:
	ldi ZL, low(COURSE << 1)		;point Z to response string 2
	ldi ZH, high(COURSE << 1)

	call OUT_STRING		;print string at location Z is pointing to
	jmp LOOP			;display menu again and wait for input

THREE:
	ldi ZL, low(HOBBY << 1)		;point Z to response string 3
	ldi ZH, high(HOBBY << 1)

	call OUT_STRING		;print string at location Z is pointing to
	jmp LOOP			;display menu again and wait for input

FOUR:
	ldi ZL, low(QUOTE << 1)		;point Z to response string 4
	ldi ZH, high(QUOTE << 1)

	call OUT_STRING		;print string at location Z is pointing to
	jmp LOOP			;display menu again and wait for input

FIVE:
	ldi ZL, low(MOVIE << 1)		;point Z to response string 5
	ldi ZH, high(MOVIE << 1)

	call OUT_STRING		;print string at location Z is pointing to
	jmp LOOP			;display menu again and wait for input

;output "Done!" and terminate execution
DONE:
	ldi ZL, low(TERMINATE << 1)		;point Z to response string 5
	ldi ZH, high(TERMINATE << 1)

	call OUT_STRING		;print string at location Z is pointing to
	rjmp END

END:			;terminate program w/ infinite loop
	rjmp END

USART_INIT:
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
	sts PORTD_DIRCLR, R16		; set pin 2 as input

	ldi R16, 0x08				;pin 3 is transmit = output
	sts PORTD_DIRSET, R16		; set pin 3 as output
	sts PoRTD_OUTSET, R16		; set activation level as high (pin3)
	
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