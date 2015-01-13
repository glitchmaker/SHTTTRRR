/*
 * Intshttr.c
 *
 * Created: 10/01/2015 19:01:21
 *  Author: glitchmaker
 */ 

#define F_CPU 1000000UL
#define BUTTON_PRESSED ((PINB & 0x10) == 0)
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>

unsigned int state=0;
volatile unsigned int seconds=0;
volatile unsigned int start_second = 0;
volatile unsigned int stop_second = 0;
volatile unsigned int duration = 0;
volatile unsigned int stored_duration=0;
volatile int i = 0;
volatile unsigned int timeout = 0;

int set_PORTB_bit(int position, int value);
int get_seconds();
int blink(int times);
void _delay_sec(int seconds);
void variable_blink(int time_up, int time_down, int times);

enum states{
	START_P,
	STOP_P,
	RUN,
	STOP,
	};
	
void set_watchdog()
{
	WDTCR |= (1<<WDP2)|(1<<WDP1);  // watchdog timer to 1 second
	WDTCR |= (1<<WDIE); // sets watchdog to interrupts	
}

void shoot()
{
	set_PORTB_bit(3,1);
	_delay_ms(550);
	set_PORTB_bit(3,0);
}

void set_interrupt()
{
	GIMSK |= (1<<PCIE);
	PCMSK |= (1<<PCINT4);
}

void unset_interrupt()
{
	GIMSK &= ~(1<<PCIE);
	PCMSK &= ~(1<<PCINT4);
}

ISR(WDT_vect)
{
	seconds++;
	if (state==RUN)
	{
		timeout--;
		if(timeout==0)
		{
			shoot();
			variable_blink(50,10,1);
			timeout=duration;
		}
	}
}

ISR(PCINT0_vect)
{
	unset_interrupt();// halts interrups to avoid interference
	_delay_ms(250); //helps to debouce
	switch(state)
	{
		case START_P:
		start_second = get_seconds(); // gets initial second
		blink(1);
		state=STOP_P;
		break;
		case STOP_P:
		stop_second = get_seconds();// gets final second
		duration = stop_second - start_second;// calculates duration in seconds
		stored_duration = duration; // stores the base duration
		timeout=duration;
		blink(1);
		state=RUN;
		break;
		default:
		break;		
	}	
	set_interrupt();
}
void variable_delay_ms(int duration)
{
	while (duration--)
	{
		_delay_ms(1);
	}
		
}


void variable_blink(int time_up, int time_down, int times)
{
	while(times--)
	{
		set_PORTB_bit(0,1);
		variable_delay_ms(time_up);
		set_PORTB_bit(0,0);
		variable_delay_ms(time_down);
	}
}

int get_seconds()
{
	int current_second;
	ATOMIC_BLOCK(ATOMIC_FORCEON){
		current_second = seconds;
	}
	return current_second;
}

void set_unsed_pins()
{
	PORTB |= _BV(PB1);
	PORTB |= _BV(PB2);
	PORTB |= _BV(PB5);
}

int blink(int times)
{
	int count;
	for (count=0;count<times;count++)
	{
		set_PORTB_bit(0,1);
		_delay_ms(300);
		set_PORTB_bit(0,0);
		_delay_ms(200);
		
	}
	return 1;
}

int main(void)
{
	DDRB = 0b00000001;
	set_unsed_pins();
	cli();
	set_watchdog();
	set_interrupt();
	sei();
	for( ;1==1; )
	{
		
	}
	
	return 1;
}


int set_PORTB_bit(int position, int value)
{
	if(value == 0)
	{
		PORTB &= ~(1 << position);
	}
	else
	{
		PORTB |= (1 << position);
	}
	return 1;
}