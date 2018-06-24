#include "common.h"

#ifdef __AVR_ATmega328P__

/*
 * ticks_per_msec=1/((prescalar_of_counter*steps)/(F_CPU))/1000
 * 1/((8*256)/(8*10^6))/1000=3.9063
 */
#define TICKS_PER_MSEC	(4)

unsigned int g_ticks_elapsed=0;
unsigned int g_msec_elapsed=0;

uint16_t g_lfsr=0x1234;

ISR(TIMER0_OVF_vect) {
	g_ticks_elapsed++;

	if(g_ticks_elapsed>=TICKS_PER_MSEC) {
		g_msec_elapsed+=g_ticks_elapsed/TICKS_PER_MSEC;
		g_ticks_elapsed%=TICKS_PER_MSEC;
	}
}

ISR(PCINT0_vect) {
	;
}

void srand(uint16_t seed) {
	g_lfsr=seed;
	if(!g_lfsr)
		g_lfsr=0x1234;
}

uint16_t rand(void) {
    uint16_t lsb=g_lfsr&1;
		
	g_lfsr>>=1;
	if(lsb) 
		g_lfsr^=0xb400;

    return g_lfsr;
}

void sys_init(void) {
	cli();

	//PWM
	DDRD|=_BV(DDD6);				//PD6/OC0A: output, default=low
	DDRD|=_BV(DDD5);				//PD5/OC0B: output, default=low
	
	//UI
	PORTB|=_BV(PORTB1);				//PB1/PCINT1: input, pulled-up
	
	//VMON
	;								//PC1/ADC1: input
	
	//EN
	DDRB|=_BV(DDB2);				//PB2: output, default=low

	//STATUS
	DDRD|=_BV(DDD2);				//PD2: output, default=low

	TCCR0A|=(_BV(WGM01)|_BV(WGM00));				//Counter0, fast mode, capture overflow event
	TIMSK0|=_BV(TOIE0);

	ADMUX|=_BV(REFS0)|_BV(REFS1);					//Internal bandgap as Vref, divider=64 (8MHz/64=125KHz), left adjust
	ADMUX|=_BV(ADLAR);
	ADCSRA|=(_BV(ADPS2)|_BV(ADPS1));

	ADMUX|=_BV(MUX0);								//ADC1 as input, no digital function
	DIDR0|=_BV(ADC1D);
	
	PCIFR|=_BV(PCIF2)|_BV(PCIF1)|_BV(PCIF0);		//Clear all pin change interrupt flags, enable PCI0
	PCICR|=_BV(PCIE0);

	sei();
}

unsigned int sys_ts_get(void) {
	unsigned int ret=0;
	
	cli();
	ret=g_msec_elapsed;
	sei();
	
	return ret;
}

void sys_counter_on(void) {
	TCCR0B|=_BV(CS01);				//Counter0 on, divider=8 (8MHz/8/256=3.90625KHz)
}

void sys_counter_off(void) {
	TCCR0B&=~_BV(CS01);
}

void sys_waken_by_sw_on(void) {
	cli();
	PCMSK0|=_BV(PCINT1);			//Capture PCI0 event when PC occurs on PB1
	PORTB|=_BV(PORTB1);
	sei();
}

void sys_waken_by_sw_off(void) {
	cli();
	PCIFR|=_BV(PCIF2)|_BV(PCIF1)|_BV(PCIF0);
	PCMSK0&=~_BV(PCINT1);
	sei();
}

void dimmer_pwm_on(void) {
	cli();
	TCCR0A|=_BV(COM0A1)|_BV(COM0B1);		//PWM on, non-inverting mode
	sei();
}

void dimmer_pwm_off(void) {
	cli();
	TCCR0A&=~(_BV(COM0A1)|_BV(COM0B1));
	sei();
}

unsigned char dimmer_pwm_get(int ch) {
	if(ch==CH0)
		return OCR0A;
	
	return OCR0B;
}

void dimmer_pwm_set(int ch, unsigned char val) {
	if(ch==CH0)
		OCR0A=val;
	else
		OCR0B=val;
}

void dimmer_drv_on(void) {
	PORTB|=_BV(PORTB2);
}

void dimmer_drv_off(void) {
	PORTB&=~_BV(PORTB2);
}

unsigned char ui_sw_get(void) {
	return PINB&_BV(PINB1);
}

void ui_led_on(void) {
	PORTD|=_BV(PORTD2);
}

void ui_led_off(void) {
	PORTD&=~_BV(PORTD2);
}

void vmon_adc_on(void) {
	ADCSRA|=_BV(ADEN);
}

void vmon_adc_off(void) {
	ADCSRA&=~_BV(ADEN);
}

unsigned char vmon_adc_get(void) {
	while(ADCSRA&_BV(ADSC));
	ADCSRA|=_BV(ADSC);
	while(ADCSRA&_BV(ADSC));

	return ADCH;
}

#endif