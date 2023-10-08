#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>

/*---------------------(Global Variables)-------------------------*/

unsigned char tick_seconds_1=0;/*tenth place in seconds*/
unsigned char tick_seconds_2=0;/*ones place in seconds*/
unsigned char tick_minutes_1=0;/*tenth place in minutes*/
unsigned char tick_minutes_2=0;/*ones place in minutes*/
unsigned char tick_hours_1=0;/*tenth place in hours*/
unsigned char tick_hours_2=0;/*ones place in hours*/

/*----------------(Timer and Interrupt Modules)------------------*/

void TIMER1(void){/*generate interrupt every 1 second*/
	TCCR1A = (1<<FOC1A)|(1<<FOC1B);/*specify non PWM mode*/
	TCCR1B = (1<<WGM12)|(1<<CS10)|(1<<CS12);/*CTC Mode and 1024 pre-scaler*/
	TIMSK |= (1<<OCIE1A);/*enable interrupts for OCIE1A*/
	TCNT1 = 0;/*initial counter value=0*/
	OCR1A = 1000;/*max counter count is 1000*/
}
void INTERRUPTS(void){
	MCUCR = (1<<ISC01)|(1<<ISC11)|(1<<ISC10);/*rising or falling edge detection for INT0 and INT1*/
	GICR = (1<<INT0)|(1<<INT1)|(1<<INT2);/*module interrupt enable for external interrupts*/
	DDRD &= ~(1<<2);/*portd pin 2 input pin*/
	DDRD &= ~(1<<3);/*portd pin 3 input pin*/
	PORTD |= (1<<2);/*enable internal pull up resistor*/
	DDRB &= ~(1<<2);/*portb pin 2 input pin*/
	PORTB |= (1<<2);/*enable internal pull up resistor*/
}

/*---------------------(Main Function)-------------------------*/

int main(){

	TIMER1();/*timer sends interrupt every 1 second*/
	INTERRUPTS();/*enable the external interrupts*/
	DDRC |= 0x0F;/*First 4 pins in portc set as output for decoder*/
	PORTC &= 0xF0;/*initial value is set as 0*/
	DDRA |= 0x3F;/*first 6 pins set as output for transistors*/
	PORTA &= 0xC0;/*all 6 transistors are turned off initially*/
	SREG |= (1<<7);/*enable i-bit*/

	while(1){ /*7 segment is continuously being updated*/

		PORTC = (PORTC & 0xF0)|(tick_seconds_1 & 0x0F);/*insert seconds value in decoder*/
		PORTA |= (1<<0);/*first 7 segment is turned on*/
		_delay_ms(2);
		PORTA &= ~(1<<0);/*first 7 segment is turned off*/

		PORTC = (PORTC & 0xF0)|(tick_seconds_2 & 0x0F);
		PORTA |= (1<<1);
		_delay_ms(2);
		PORTA &= ~(1<<1);

		PORTC = (PORTC & 0xF0)|(tick_minutes_1 & 0x0F);
		PORTA |= (1<<2);
		_delay_ms(2);
		PORTA &= ~(1<<2);

		PORTC = (PORTC & 0xF0)|(tick_minutes_2 & 0x0F);
		PORTA |= (1<<3);
		_delay_ms(2);
		PORTA &= ~(1<<3);

		PORTC = (PORTC & 0xF0)|(tick_hours_1 & 0x0F);
		PORTA |= (1<<4);
		_delay_ms(2);
		PORTA &= ~(1<<4);

		PORTC = (PORTC & 0xF0)|(tick_hours_2 & 0x0F);
		PORTA |= (1<<5);
		_delay_ms(2);
		PORTA &= ~(1<<5);

	}
}

/*-------------------------(ISR Code)---------------------------*/

ISR(TIMER1_COMPA_vect){
	SREG |= (1<<7);/*enable i-bit because it is turned off automatically once in ISR(nesting)*/
	tick_seconds_1++;/*increment seconds value*/
	if(tick_seconds_1==10){/*reset first 7 segment to 0 once it reaches 9*/
		tick_seconds_2++;
		tick_seconds_1=0;
	}
	if(tick_seconds_2==6){/*reset second 7 segment once it reaches 6*/
		tick_minutes_1++;
		tick_seconds_2=0;
	}
	if(tick_minutes_1==10){
		tick_minutes_2++;
		tick_minutes_1=0;
	}
	if(tick_minutes_2==6){
		tick_hours_1++;
		tick_minutes_2=0;
	}
	if(tick_hours_1==10){
		tick_hours_2++;
		tick_hours_1=0;
	}
}

ISR(INT0_vect){/*reset stop-watch to zero*/
	tick_seconds_1=0;
	tick_seconds_2=0;
	tick_minutes_1=0;
	tick_minutes_2=0;
	tick_hours_1=0;
	tick_hours_2=0;
}

ISR(INT1_vect){/*pause stop-watch*/
	TCCR1B &= ~(1<<CS10) & ~(1<<CS12);/*remove clock from timer*/
}

ISR(INT2_vect){/*resume stop-watch*/
	TCCR1B |= (1<<CS12)|(1<<CS10);/*insert clock into timer*/
}
