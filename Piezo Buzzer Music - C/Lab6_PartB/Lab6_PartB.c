/* 
 Lab6_PartB.c
 Lab 6 Part B
 Name: Mitchell Irvin
 Section: 1E83
 TA Name: Khaled Hassan 
 Description: this program will do the stuff
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "ebi_driver.h"

//////////////////////////////////INITIALIZATIONS////////////////////////////////////
#define F_CPU 2000000       // ATxmega runs at 2MHz on Reset.
#define CS0_Start 0x288000
#define CS0_End 0x289FFF
#define CS1_Start 0x394000 //runs to 397FFFF (16k)
#define LCD_Command 0x395000	//even to clear RS so it's read as a command
#define LCD_Data 0x395001		//odd to set RS so it's read as data

//note frequencies
#define A5 880
#define B5 987
#define C6 1047
#define C6_SHARP 1109
#define D6_FLAT 1109
#define D6 1175
#define D6_SHARP 1245
#define E6_FLAT 1245
#define E6 1319
#define F6 1397
#define F6_SHARP 1480
#define G6_FLAT 1480
#define G6 1568
#define G6_SHARP 1661
#define A6_FLAT 1661
#define A6 1760
#define A6_SHARP 1865
#define B6_FLAT 1865
#define B6 1976
#define C7 2093
#define C7_SHARP 2217
#define D7_FLAT 2217
#define D7 2349

//times
#define dur_567 17744
#define dur_374 11700
#define dur_1492 46630

//durations for songs
#define doub 35488
#define dotfull 25000
#define full 17744
#define dothalf 12500
#define half 8872
#define quarter 4436

//global int to keep track of what to do when an interrupt is triggered
int i;

//prototypes
void RoughDelay1sec(void);
void EBI_init();
void LCD_init(); 
void printNote(char *note, char *freq);
void TCE0_init(); 
void TCE0_quit(); 
void TCE1_init();
int keypad_scan();
void keypad_init();
void pollBusy();
void outChar(char c);
void outString(char *str);
void freqToNote(int freq);
void playNote(char *note, char *freq);
int playSoundsOfSilence(); 
int playDescScale();
int finishNote(int k); 
int doPause(int period);
int doTheThing(int period, int freq);
int doPause2(int period);
int doTheThing2(int period, int freq);

int main(void)
{
	//init EBI for 3 port SRAM mode w/ ALE1
	keypad_init();
	EBI_init();
	LCD_init(); 
	TCE1_init(); 
	TCE0_init();
	
	PMIC.CTRL = 0x01;	//enable low level interrupts
	sei();				//set global interrupt 
	int k = 0; 
	
    while (1) 
    {
		k = keypad_scan(); 
		while(k == 16) k = keypad_scan(); //poll keypad for input
		i = 0;	//set to 0 for ISR to read 
		
		if(k == 1) {
			char note[] = "C6";
			char freq[] = "1046.5 Hz";
			playNote(note, freq);
			freqToNote(C6);	//play C6
			while(k == 1) k = keypad_scan();	//wait for key to be released
		} else if(k == 2) {
			char note[] = "C6# / Db6";
			char freq[] = "1108.73 Hz";
			playNote(note, freq);	//function to handle LCD output and interrupt stuff
			freqToNote(C6_SHARP);	//play C6#
			while(k == 2) k = keypad_scan();	//wait for key to be released
		} else if(k == 3) {
			char note[] = "D6";
			char freq[] = "1174.66 Hz";
			playNote(note, freq);	//function to handle LCD output and interrupt stuff
			freqToNote(D6);	//play D6
			while(k == 3) k = keypad_scan();	//wait for key to be released
		} else if(k == 4) {
			char note[] = "D6# / Eb6";
			char freq[] = "1244.51 Hz";
			playNote(note, freq);	//function to handle LCD output and interrupt stuff
			freqToNote(D6_SHARP);	//play D6#
			while(k == 4) k = keypad_scan();	//wait for key to be released
		} else if(k == 5) {
			char note[] = "E6";
			char freq[] = "1318.51 Hz";
			playNote(note, freq);	//function to handle LCD output and interrupt stuff
			freqToNote(E6);	//play E6
			while(k == 5) k = keypad_scan();	//wait for key to be released
		} else if(k == 6) {
			char note[] = "F6";
			char freq[] = "1396.91 Hz";
			playNote(note, freq);	//function to handle LCD output and interrupt stuff
			freqToNote(F6);	//play F6
			while(k == 6) k = keypad_scan();	//wait for key to be released
		} else if(k == 7) {
			char note[] = "F6# / Gb6";
			char freq[] = "1479.98 Hz";
			playNote(note, freq);	//function to handle LCD output and interrupt stuff
			freqToNote(F6_SHARP);	//play F6#
			while(k == 7) k = keypad_scan();	//wait for key to be released
		} else if(k == 8) {
			char note[] = "G6";
			char freq[] = "1567.98 Hz";
			playNote(note, freq);	//function to handle LCD output and interrupt stuff
			freqToNote(G6);	//play G6
			while(k == 8) k = keypad_scan();	//wait for key to be released
		} else if(k == 9) {
			char note[] = "G6# / Ab6";
			char freq[] = "1661.22 Hz";
			playNote(note, freq);	//function to handle LCD output and interrupt stuff
			freqToNote(G6_SHARP);	//play G6#
			while(k == 9) k = keypad_scan();	//wait for key to be released
		} else if(k == 0) {
			char note[] = "A6";
			char freq[] = "1760.00 Hz";
			playNote(note, freq);	//function to handle LCD output and interrupt stuff
			freqToNote(A6);	//play A6
			while(k == 0) k = keypad_scan();	//wait for key to be released
		} else if(k == 10) {
			char note[] = "A6# / Bb6";
			char freq[] = "1864.66 Hz";
			playNote(note, freq);	//function to handle LCD output and interrupt stuff
			freqToNote(A6_SHARP);	//play A6#
			while(k == 10) k = keypad_scan();	//wait for key to be released
		} else if(k == 11) {
			char note[] = "B6";
			char freq[] = "1975.33 Hz";
			playNote(note, freq);	//function to handle LCD output and interrupt stuff
			freqToNote(B6);	//play B6
			while(k == 11) k = keypad_scan();	//wait for key to be released
		} else if(k == 12) {
			char note[] = "C7";
			char freq[] = "2093.00 Hz";
			playNote(note, freq);	//function to handle LCD output and interrupt stuff
			freqToNote(C7);	//play D6
			while(k == 12) k = keypad_scan();	//wait for key to be released
		} else if(k == 13) {
			char note[] = "C7# / Db7";
			char freq[] = "2217.46 Hz";
			playNote(note, freq);	//function to handle LCD output and interrupt stuff
			freqToNote(C7_SHARP);	//play D6
			while(k == 13) k = keypad_scan();	//wait for key to be released
		} else if(k == 14) {
			playSoundsOfSilence(); 
			while(k == 14) k = keypad_scan();	//wait for key to be released
		} else if(k == 15) {
			playDescScale();
			while(k == 15) k = keypad_scan();	//wait for key to be released
		} else {
			//switch not active, turn off counter 
			TCE0_quit();
		}
		
    }
}

void playNote(char *note, char *freq) {
	printNote(note, freq);
	TCE1.PER = dur_567;
	TCE1.CTRLFSET = 0x08;	//restart the TC 
	TCE0_init();
}

//function to take a frequency and output the correct note
void freqToNote(int freq) {
	double frac = (double)1000/freq;
	double period = (double)frac * 1000;
	TCE0.CCA = (int)period; 
}

//initialize all registers for TC here
//OC0B (pin 1 port E)
void TCE0_init() {
	//init PORTE pins
	PORTE.DIRCLR = 0x01;	//set pin 0 as input (switch to toggle buzzer)
	PORTE.DIRSET = 0x02;	//set pin 1 as output
	
	//init TCE0 regs
	TCE0.CTRLA = 0x01;		//prescaler: clk
	TCE0.CTRLB = 0x21;		//CCB enabled, WG mode = frequency
							//time period controlled by CCA reg
	TCE0.CTRLD = 0x00;		//not using event capture for this 
							//b/c using prescaling see 14.2 para. 3
	TCE0.CTRLE = 0x00;		//normal byte mode
	
	TCE0.CTRLFSET = 0x80; 
}

//function to set TC w/ interrupts to determine
//notes of length 1492ms, 374.4ms, or 567.8ms
void TCE1_init() {
	TCE1.CTRLA = 0x05;		//prescaler = 64
	TCE1.CTRLB = 0x00;		//normal mode timer 
	TCE1.INTCTRLA = 0x01;	//enable low level interrupt on overflow
	TCE1.PER = 7812;			//period for 567.8ms
}

void TCE0_quit() {
	//turn off counter
	TCE0.CTRLA = 0x00;		
	TCE0.CTRLB = 0x00;	
	TCE0.CCA = 0x00; 
}

void RoughDelay1sec(void)
{
	volatile uint32_t ticks;            //Volatile prevents compiler optimization
	for(ticks = 0; ticks <= F_CPU; ticks++);	//increment 2e6 times -> ~ 1 sec
}

void LCD_init() {
	//Normal Further LCD Initialization - LCD_Notes_8-bit.pdf
	pollBusy(); 
	__far_mem_write(LCD_Command, 0x38);	//two lines
	pollBusy();
	__far_mem_write(LCD_Command, 0x0F); //display on; cursor on; blink on
	pollBusy();
	__far_mem_write(LCD_Command, 0x01);	//clear screen; cursor home
	pollBusy();
}

void printNote(char *note, char *freq) {
	__far_mem_write(LCD_Command, 0x01); //clear LCD
	pollBusy(); 
	outString(note);
	pollBusy();
	__far_mem_write(LCD_Command, 0xC0); //next line
	pollBusy();
	outString(freq);
}

void EBI_init()
{
	PORTH.DIR = 0x37;       // Enable RE, WE, CS0, CS1, ALE1
	PORTH.OUT = 0x33;		// activation levels
	PORTK.DIR = 0xFF;       // Enable Address 7:0 (outputs)
	// Do not need to set PortJ to outputs
	
	EBI.CTRL = EBI_SRMODE_ALE1_gc | EBI_IFMODE_3PORT_gc;            // ALE1 multiplexing, 3 port configuration

	EBI.CS0.BASEADDRH = (uint8_t) (CS0_Start>>16) & 0xFF;
	EBI.CS0.BASEADDRL = (uint8_t) (CS0_Start>>8) & 0xFF;            // Set CS0 range to 0x288000 - 0x289FFF
	EBI.CS0.CTRLA = EBI_CS_MODE_SRAM_gc | EBI_CS_ASPACE_8KB_gc;	    // SRAM mode, 8k address space

	// BASEADDR is 16 bit (word) register. C interface allows you to set low and high parts with 1
	// instruction instead of the previous two
	EBI.CS1.BASEADDR = (uint16_t) (CS1_Start>>8) & 0xFFFF;          // Set CS1 range to 0x394000 - 0x397FFF
	EBI.CS1.CTRLA = EBI_CS_MODE_SRAM_gc | EBI_CS_ASPACE_16KB_gc;	//16k size SRAM mode
}

void keypad_init() {
	//first, set upper 4 bits of PORTE as output
	PORTF.DIR = 0x0F;
	//then set lower 4 bits to have internal pull-up resistors
	//we will write 0 to the column we are checking and will be
	//checking to see if a row is 0 to determine a key press
	PORTF.PIN4CTRL = 0x18;
	PORTF.PIN5CTRL = 0x18;
	PORTF.PIN6CTRL = 0x18;
	PORTF.PIN7CTRL = 0x18;
}

int keypad_scan() {
	//values to write to PORTF_OUT to check each column
	//in order from row1 to row4
	uint16_t rows[] = {0x0E, 0x0D, 0x0B, 0x07};
	//values to compare PORTE_IN to after writing to PORTF_OUT
	uint16_t col1[] = {0x7E, 0xBE, 0xDE, 0xEE};
	uint16_t col2[] = {0x7D, 0xBD, 0xDD, 0xED};
	uint16_t col3[] = {0x7B, 0xBB, 0xDB, 0xEB};
	uint16_t col4[] = {0x77, 0xB7, 0xD7, 0xE7};
	//variable to read from PORTF_IN
	uint16_t volatile input = 0x00;
	
	for(int i = 0; i < 4; i++) {
		//write to output the column that we're currently checking
		PORTF.OUT = rows[i];
		//two nops to give time to get proper input read
		asm volatile ("nop");
		asm volatile ("nop");
		//read from PORTE_IN
		input = PORTF.IN;
		//if on first column
		if(i == 0){
			//check each row
			for(int j = 0; j < 4; j++) {
				//compare input to our expected vals for col1
				if(input == col1[j]) {
					if(j == 3) {
						return 1; //if 'A'
						} else if(j == 2) {
						return 4;
						} else if(j == 1) {
						return 7;
						} else {
						return 14;
					}
				}
			}
		}
		//if on second column
		else if(i == 1){
			//check each row
			for(int j = 0; j < 4; j++) {
				//compare input to our expected vals for col2
				if(input == col2[j]) {
					if(j == 3) {
						return 2;
						} else if (j == 2) {
						return 5;
						} else if(j == 1) {
						return 8;
						} else {
						return 0;
					}
				}
			}
		}
		//if on third column
		else if(i == 2){
			//check each row
			for(int j = 0; j < 4; j++) {
				//compare input to our expected vals for col3
				if(input == col3[j]) {
					if(j == 3) {
						return 3;
						} else if(j == 2) {
						return 6;
						} else if(j == 1) {
						return 9;
						} else {
						return 15;
					}
				}
			}
		}
		//if on fourth column
		else {
			//check each row
			for(int j = 0; j < 4; j++) {
				//compare input to our expected vals for col4
				if(input == col4[j]) {
					if(j == 0) {
						return 13;
						} else if(j == 1) {
						return 12;
						} else if(j == 2) {
						return 11;
						} else {
						return 10;
					}
				}
			}
		}
	} //end for loop (writing to each column)
	//no key pressed, return default value of 16
	return 16;
} //end keypad_scan

void pollBusy()
{
	//create variable to hold data read from LCD
	volatile uint8_t readLCD_8;
	
	//allow two cycles for busy flag to be set
	//as said in part A section 4 of lab doc
	asm volatile ("nop");
	asm volatile ("nop");
	
	// read to determine if DB7 is still set
	readLCD_8 = __far_mem_read(LCD_Command);

	//while DB7 is set, read and check new value
	//this will spin until the busy flag is no longer 1
	while(readLCD_8 > 0x7F){
		//read from LCD_Command again
		readLCD_8 = __far_mem_read(LCD_Command);
	}
}

void outChar(char c) {
	pollBusy();	//wait til BF not set
	__far_mem_write(LCD_Data, c);	//write to data the character passed in
}

void outString(char *str) {
	//counter for index
	int index = 0;
	//until we've reached the end of our string
	while(str[index] != '\0') {
		//output currently indexed character
		outChar(str[index]);
		//wait til busy flag isn't set
		pollBusy();
		//increment index
		index++;
	}
}

void playANote(int freq){
	TCE0.CTRLA = 0x01;
	freqToNote(freq);
}

int playSoundsOfSilence() {
	char song1[] = "Sounds Of";
	char song2[] = "Silence";
	printNote(song1, song2);
	
	i = 1;	//condition for ISR
	
	//first line
	if(doTheThing(quarter, B6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;		//pause b/w notes

	if(doTheThing(quarter, B6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(quarter, D6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(quarter, D6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(quarter, F6_SHARP) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(quarter, F6_SHARP) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(dotfull, E6) == 0) return 0;	//pause b/w notes
	if(doPause(full) == 0) return 0;	//pause b/w notes
	
	//second line
	if(doTheThing(quarter, A6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;		//pause b/w notes

	if(doTheThing(quarter, A6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(quarter, C6_SHARP) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(quarter, C6_SHARP) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(quarter, E6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(quarter, E6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(dotfull, D6) == 0) return 0;	//pause b/w notes
	if(doPause(full) == 0) return 0;	//pause b/w notes
	
	//third line
	if(doTheThing(quarter, C6_SHARP) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;		//pause b/w notes

	if(doTheThing(quarter, C6_SHARP) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
		
	if(doTheThing(quarter, F6_SHARP) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
		
	if(doTheThing(quarter, F6_SHARP) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
		
	if(doTheThing(quarter, A6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
		
	if(doTheThing(quarter, A6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
		
	if(doTheThing(dothalf, B6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(dothalf, B6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(dotfull, A6) == 0) return 0;	//pause b/w notes
	if(doPause(full) == 0) return 0;	//pause b/w notes
	
	//fourth line
	if(doTheThing(quarter, D6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;		//pause b/w notes

	if(doTheThing(quarter, D6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
		
	if(doTheThing(quarter, F6_SHARP) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
		
	if(doTheThing(quarter, F6_SHARP) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
		
	if(doTheThing(quarter, A6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
		
	if(doTheThing(quarter, A6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
		
	if(doTheThing(dothalf, B6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(dothalf, B6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(dotfull, A6) == 0) return 0;	//pause b/w notes
	if(doPause(full) == 0) return 0;	//pause b/w notes
	if(doPause(full) == 0) return 0;	//pause b/w notes
	
	//fifth line
	if(doTheThing(quarter, D6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(quarter, D6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(half, B6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(full, B6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes

	if(doTheThing(quarter, B6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(quarter, C7_SHARP) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(half, D7) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(half, D7) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(half, C7_SHARP) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(half, B6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(dotfull, A6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	//next
	if(doTheThing(quarter, B6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(quarter, A6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(doub, F6_SHARP) == 0) return 0;	//pause b/w notes
	
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	if(doPause(full) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(quarter, D6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	if(doTheThing(quarter, D6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	if(doTheThing(quarter, D6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(dotfull, A6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(quarter, C6_SHARP) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(half, D6) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	if(doTheThing(full, B5) == 0) return 0;	//pause b/w notes
	if(doPause(quarter) == 0) return 0;	//pause b/w notes
	
	i = 0;
	return 1; 	
}

int playDescScale() {
	char song1[] = "Descending";
	char song2[] = "Scale";
	printNote(song1, song2);
		
	i = 1;	//condition for ISR
	
	//first line
	if(doTheThing2(half, C7) == 0) return 0;	//pause b/w notes
	if(doPause2(quarter) == 0) return 0;		//pause b/w notes
	
	if(doTheThing2(half, B6) == 0) return 0;	//pause b/w notes
	if(doPause2(quarter) == 0) return 0;		//pause b/w notes
	
	if(doTheThing2(half, A6) == 0) return 0;	//pause b/w notes
	if(doPause2(quarter) == 0) return 0;		//pause b/w notes
	
	if(doTheThing2(half, F6) == 0) return 0;	//pause b/w notes
	if(doPause2(quarter) == 0) return 0;		//pause b/w notes
	
	if(doTheThing2(half, E6) == 0) return 0;	//pause b/w notes
	if(doPause2(quarter) == 0) return 0;		//pause b/w notes
	
	if(doTheThing2(half, D6) == 0) return 0;	//pause b/w notes
	if(doPause2(quarter) == 0) return 0;		//pause b/w notes
	
	if(doTheThing2(half, C6) == 0) return 0;	//pause b/w notes
	if(doPause2(quarter) == 0) return 0;		//pause b/w notes
	
	if(doTheThing2(half, B5) == 0) return 0;	//pause b/w notes
	if(doPause2(quarter) == 0) return 0;		//pause b/w notes
	
	if(doTheThing2(half, A5) == 0) return 0;	//pause b/w notes
	if(doPause2(quarter) == 0) return 0;		//pause b/w notes
	
	i = 0; 
	return 0; 
}

int doTheThing(int period, int freq) {
	int f = 0; 
	TCE1.PER = period;
	TCE1.CTRLFSET = 0x08;	//restart the TC
	TCE0_init(); 
	playANote(freq);		//play the note
	while(i == 1) {
		f = keypad_scan();
		if(f != 14){
			//if keypress changes return from function
			TCE0_quit();
			return 0;
		}
	}
	i = 1; 
	return 1; 
}

int doTheThing2(int period, int freq) {
	int f = 0; 
	TCE1.PER = period;
	TCE1.CTRLFSET = 0x08;	//restart the TC
	TCE0_init(); 
	playANote(freq);		//play the note
	while(i == 1) {
		f = keypad_scan();
		if(f != 15){
			//if keypress changes return from function
			TCE0_quit();
			return 0;
		}
	}
	i = 1; 
	return 1; 
}

int doPause(int period) {
	int f = 0; 
	TCE1.PER = period;
	TCE1.CTRLFSET = 0x08;	//restart the TC
	TCE0_quit();		//break 
	while(i == 1) {
		f = keypad_scan();
		if(f != 14){
			//if keypress changes return from function
			return 0;
		}
	}
	i = 1; 
	return 1; 
}

int doPause2(int period) {
	int f = 0; 
	TCE1.PER = period;
	TCE1.CTRLFSET = 0x08;	//restart the TC
	TCE0_quit();		//break 
	while(i == 1) {
		f = keypad_scan();
		if(f != 15){
			//if keypress changes return from function
			return 0;
		}
	}
	i = 1; 
	return 1; 
}

ISR(TCE1_OVF_vect) {
	if(i == 0){
		TCE0_quit();
	} else if(i == 1) {
		i = 2;		//increment i so loop will break and song will continue
	}
	
}