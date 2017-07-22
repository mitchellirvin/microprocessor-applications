/* 
 Lab5_lcd_keypad.c
 Lab 5 Part E
 Name: Mitchell Irvin
 Section: 1E83
 TA Name: Khaled Hassan 
 Description: this program will do things based on key pressed:
 0,1 = toggle display on or off
 2,3 = continuously display pot tap voltage
 4,5 = display "Mitch Irvin" on LCD
 6,7 = display "May the Schwartz
				be with you!"
 *,# = clear LCD and blink cursor at home, control LED w/ CdS circuit
 others = nothing
*/

//////////////////////////////////////INCLUDES///////////////////////////////////////
#include <avr/io.h>
#include "ebi_driver.h"

//////////////////////////////////INITIALIZATIONS////////////////////////////////////
#define F_CPU 20000       // ATxmega runs at 2MHz on Reset.
// take two 0s off for faster voltage testing response time
#define CS0_Start 0x288000
#define CS0_End 0x289FFF
#define CS1_Start 0x394000 //runs to 397FFFF (16k)
#define LCD_Command 0x395000	//even to clear RS so it's read as a command
#define LCD_Data 0x395001		//odd to set RS so it's read as data

void LCD_VM(int key); 
void LCD_CDS(int key); 
void toggle_LCD(int flip);
void RoughDelay1sec(void);
void EBI_init();
void ADC_init();
void ADC_init_CH1(); 
int keypad_scan();
void keypad_init();
void pollBusy();
void outChar(char c);
void outString(char *str);
int toAscii(int n);
float toDec(float f);
float diffTen(float volt, int sub);

int main(void)
{
	//perform function based on key pressed
	int keypressed = 0; 
	int flip = 0; 
	char name[] = "Mitch Irvin\0";
	char space[] = "May the Schwartz\0";
	char balls[] = "be with you!\0";
	char others1[] = "Grassssss... \0";
	char others2[] = "tastes bad!\0";
	
	keypad_init();
	EBI_init();		//initialize EBI
	//ADC_init_CH1();	//initialize CH1 for CdS cell
	
	//Normal Further LCD Initialization - LCD_Notes_8-bit.pdf
	__far_mem_write(LCD_Command, 0x38);	//two lines
	pollBusy();
	__far_mem_write(LCD_Command, 0x0F); //display on; cursor on; blink on
	pollBusy();
	__far_mem_write(LCD_Command, 0x01);	//clear screen; cursor home
	pollBusy();
	
	while(1) {
		keypressed = keypad_scan();
		if(keypressed == 0 || keypressed == 1) {
			ADC_init();		//initialize ADCB CH0
			toggle_LCD(flip); 
			flip = ~flip; 
		} else if(keypressed == 2 || keypressed == 3) {
			ADC_init();		//initialize ADCB CH0
			LCD_VM(keypressed);
		} else if(keypressed == 4 || keypressed == 5) {
			ADC_init();
			pollBusy();
			//clear LCD, cursor home, and wait for busy flag to clear
			__far_mem_write(LCD_Command, 0x01);
			pollBusy();
			pollBusy();
			outString(name);
			pollBusy();
			RoughDelay1sec();
			RoughDelay1sec();
		} else if(keypressed == 6 || keypressed == 7) {
			ADC_init();
			pollBusy();
			//clear LCD, cursor home, and wait for busy flag to clear
			__far_mem_write(LCD_Command, 0x01);
			pollBusy();
			outString(space);
			pollBusy();
			__far_mem_write(LCD_Command, 0xC0); //next line
			pollBusy();
			outString(balls);
			pollBusy();
		} else if(keypressed == 14 || keypressed == 15) {
			ADC_init_CH1();
			pollBusy();
			LCD_CDS(keypressed);
			pollBusy();
		} else if(keypressed == 16) {
			continue;
		} else {
			ADC_init();
			pollBusy();
			//clear LCD, cursor home, and wait for busy flag to clear
			__far_mem_write(LCD_Command, 0x01);
			pollBusy();
			outString(others1);
			pollBusy();
			__far_mem_write(LCD_Command, 0xC0); //next line
			pollBusy();
			outString(others2);
			pollBusy();
		}
		continue; 
	}
	
	return 0;
}

void LCD_CDS(int key) {
	//clear CH1? wait for bit?
	
	
	int keystart = key; 
	float voltCDS = 0; 
		
	//Normal Further LCD Initialization - LCD_Notes_8-bit.pdf
	__far_mem_write(LCD_Command, 0x38);	//two lines
	pollBusy();
	__far_mem_write(LCD_Command, 0x0F); //display on; cursor on; blink on
	pollBusy();
	__far_mem_write(LCD_Command, 0x01);	//clear screen; cursor home
	pollBusy();
		
	//continuously read the voltage
	while(key == keystart || key == 16) {
		pollBusy();
		//clear LCD and wait for busy flag to clear
		__far_mem_write(LCD_Command, 0x01);
		pollBusy();
		
		voltCDS = toDec((float)ADCB.CH1.RES);
		
		if(voltCDS <= 1) {
			__far_mem_write(CS0_Start, 0x01);
		} else {
			__far_mem_write(CS0_Start, 0x00);
		}
			
		pollBusy();
		RoughDelay1sec();	//give LED some time to chill
		key = keypad_scan(); 
	}
}

void toggle_LCD(int flip) {
	if(flip == 0) {
		pollBusy();
		__far_mem_write(LCD_Command, 0x0F);	//LCD ON
		pollBusy();
	} else {
		pollBusy();
		__far_mem_write(LCD_Command, 0x08); //LCD OFF
		pollBusy();
	}
	RoughDelay1sec(); 
}

void LCD_VM(int key) {
	int keystart = key;
	
	//variables for reading voltage and converting to Hex
	int nibble1 = 0;
	int nibble2 = 0;
	char label[] = "V (0x\0";
		
	//three columns of decimal value
	float volt = 0;
	int ones = 0;
	float tensF = 0;
	int tens = 0;
	float hundredsF = 0;
	int hundreds = 0;
	//continuously read the voltage and display to the LED
	while(key == keystart || key == 16) {
		pollBusy();
		//clear LCD, cursor home, and wait for busy flag to clear
		__far_mem_write(LCD_Command, 0x01);
		pollBusy();
			
		//convert digital to hex to ascii values for each nibble
		nibble1 = toAscii(ADCB.CH0.RES / 16);	//lower nibble
		nibble2 = toAscii(ADCB.CH0.RES % 16);	//upper nibble
		//for 5V at pot this should produce 0x7F
		//for 2.5V at pot this should produce 0x40
			
		//compute decimal values using formula given in Part D section 5
		volt = toDec((float)ADCB.CH0.RES);
		ones = (int)volt;
		tensF = diffTen(volt, ones);
		tens = (int)tensF;
		hundredsF = diffTen(tensF, tens);
		hundreds = (int)hundredsF;
			
		//generate correct ascii from this
		ones = toAscii(ones);
		tens = toAscii(tens);
		hundreds = toAscii(hundreds);
			
		pollBusy();
		outChar(ones);
			
		pollBusy();
		outChar('.');
			
		pollBusy();
		outChar(tens);
			
		pollBusy();
		outChar(hundreds);
			
		pollBusy();
		outString(label);	//print label before you print the voltage
			
		pollBusy();
		outChar(nibble1);
			
		pollBusy();
		outChar(nibble2);
			
		pollBusy();
		outChar(')');
			
		RoughDelay1sec();	//give LED some time to chill
		pollBusy(); //wait for busy flag to clear
		
		key = keypad_scan(); 
	}
}

void RoughDelay1sec(void)
{
	volatile uint32_t ticks;            //Volatile prevents compiler optimization
	for(ticks = 0; ticks <= F_CPU; ticks++);	//increment 2e6 times -> ~ 1 sec
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

//method to initialize analog to digital converter CH0
void ADC_init() {
	//initialize pin4 on portB as input
	PORTB.DIRCLR = 0x08; 
	//set ADC registers
	ADCB.REFCTRL = 0x30;		//set external reference to AREFB w/ bandgap and tempref disabled
	ADCB.CTRLA = 0x05; 			//set ch0, don't flush, set enable
	ADCB.CTRLB = 0x1C;			//signed, 8 bit, right aligned
	
	//set CH0 registers
	ADCB.CH0.CTRL = 0x81;		//set to continuous conversion on CH0, no gain, single-ended
	ADCB.CH0.INTCTRL = 0x01;	//sets conversion complete level1 interrupts
	ADCB.CH0.MUXCTRL = 0x20;	//sets pin4 as group configuration
}

//method to initialize analog to digital converter
void ADC_init_CH1() {
	//same as part B but use pin5 on port B and ch1 instead of 0
	ADCB.REFCTRL = 0x30;		//set external reference to AREFB w/ bandgap and tempref disabled
	//set ADC registers
	ADCB.CTRLA = 0x09; 			//set ch1, don't flush, set enable
	ADCB.CTRLB = 0x1C;			//signed, 8 bit, right aligned
	ADCB.EVCTRL = 0x42;	//select channels 1 and 0 to be freerunning
	
	
	//set CH1 registers
	ADCB.CH1.CTRL = 0x81;		//set to continuous conversion on CH1, no gain, single-ended
	ADCB.CH1.MUXCTRL = 0x28;	//sets pin5 as group configuration
	//ADCB.CH1.INTCTRL = 0x01;	//sets conversion complete level1 interrupts
}

int toAscii(int n) {
	//if number is <= 10
	if(n < 10) {
		n += 0x30;	//add 48
		} else {
		n += 0x37; //add 55
	}
	return n;
}

float toDec(float f) {
	return ((f / 128.0) * 5);
}

float diffTen(float volt, int sub) {
	return ((volt - sub) * 10);
}

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

