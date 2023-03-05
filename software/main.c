/*
 * main.c
 *
 * Created: 2/7/2023 6:40:06 PM
 *  Author: mikek
 */ 

#define F_CPU 8000000UL

#include <xc.h>
#include <util/delay.h>
#include <avr/interrupt.h>

//function prototypes
void init_display(void);
void init_adc(void);
uint16_t read_adc(uint8_t pin);
void update_display(uint16_t value);

//map for segment pins -> digit values
const uint8_t character_map[16] =
{
	0b00111111,  //0
	0b00000110,  //1
	0b01011011,  //2
	0b01001111,  //3
	0b01100110,  //4
	0b01101101,  //5
	0b01111101,  //6
	0b00000111,  //7
	0b01111111,  //8
	0b01100111,  //9
	0b01110111,  //A
	0b01111100,  //b
	0b00111001,  //C
	0b01011110,  //d
	0b01111001,  //E
	0b01110001   //F
};

uint8_t adc_flag = 0;
uint16_t adc_value = 0;
uint16_t speed_value = 0;

ISR(ADC_vect)
{
	adc_flag = 1;
}

int main(void)
{
	init_display();
	init_adc();
	//enable global interrupts
	sei();
	
    while(1)
    {	
		speed_value = read_adc(0);
		adc_value = 1023 - read_adc(1);
		update_display(adc_value);
	}
}

//function to init 7-segment display pins
void init_display(void)
{
	//set all digit pins as outputs
	DDRB = 0b11111111;
	//set all segment pins as outputs
	DDRD = 0b11111111;
}

//function to init adc pins
void init_adc(void)
{
	//AVcc with external cap at AREF pin
	ADMUX |= 0b01 << REFS0;
	//enable ADC, enable ADC interrupt, set clock prescaler to 64
	ADCSRA |= (1 << ADEN) | (1 << ADIE) | (0b110 << ADPS0);
	
	//enable ADC0, ADC1 pins
	DIDR0 |= 0b11;
}

uint16_t read_adc(uint8_t pin)
{
	uint8_t adcl = 0;
	uint8_t adch = 0;
	
	adc_flag = 0;
	
	ADMUX &= 0xF0;
	ADMUX |= pin;
	
	//start adc conversion
	ADCSRA |= 1 << ADSC;
	
	//wait for conversion to complete
	while(!adc_flag);
	
	adcl = ADCL;
	adch = ADCH;
	
	return (adch << 8) | adcl;
}

//function to update the value of the 7 segment display
void update_display(uint16_t value)
{
	//turn all digits off
	PORTB = 0b11111111;
	//turn all segments off
	PORTD = 0;
	
	//loop to sequentially drive each digit
	int i = 0;
	do
	{
		//turn all digits off
		PORTB = 0b11111111;
		//select digit
		PORTB &= ~(1 << (3-i));
		
		//write digit value to segment pins
		PORTD = character_map[value % 10];
		
		for(int j = 0; j < (speed_value + 1); j++)
		{
			_delay_ms(1);
		}
		
		i++;
	} while (value /= 10);
}