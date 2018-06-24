#include "common.h"

#ifdef __AVR_ATtiny24A__

/*
 * ticks_per_msec=1/((prescalar_of_counter*steps)/(F_CPU))/1000
 * 1/((8*256)/(8*10^6))/1000=3.9063
 */
#define TICKS_PER_MSEC	(4)

unsigned int g_ticks_elapsed=0;
unsigned int g_msec_elapsed=0;

uint16_t g_lfsr=0x1234;

ISR(TIM0_OVF_vect) {
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
	DDRB|=_BV(DDB2);				//PB2/OC0A: output, default=low
	DDRA|=_BV(DDA7);				//PA7/OC0B: output, default=low
	
	//UI
	PORTA|=_BV(PORTA3);				//PA3/PCINT3: input, pulled-up
	
	//VMON
	;								//PA1/ADC1: input
	
	//EN
	DDRA|=_BV(DDA2);				//PA2: output, default=low

	//STATUS
	DDRB|=_BV(DDB0);				//PB0: output, default=low

	TCCR0A|=(_BV(WGM01)|_BV(WGM00));				//Counter0, fast mode, capture overflow event
	TIMSK0|=_BV(TOIE0);

/*
	ADMUX|=_BV(REFS1);								//Internal bandgap as Vref, divider=64 (8MHz/64=125KHz), left adjust
	ADCSRB|=_BV(ADLAR);
	ADCSRA|=(_BV(ADPS2)|_BV(ADPS1));

	ADMUX|=_BV(MUX0);								//ADC1 as input, no digital function
	DIDR0|=_BV(ADC1D);
*/
	ACSR|=(_BV(ACD)|_BV(ACBG));						//Disable comparator, Internal bandgap as V+

	ADCSRB|=_BV(ACME);								//Multiplexer is enabled for comparator

	ADMUX|=_BV(MUX0);								//ADC1 as V-, no digital function
	DIDR0|=_BV(ADC1D);

	GIFR|=_BV(INTF0)|_BV(PCIF1)|_BV(PCIF0);			//Clear all interrupt flags, enable PCI0
	GIMSK|=_BV(PCIE0);

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
	PCMSK0|=_BV(PCINT3);			//Capture PCI0 event when PC occurs on PA3
	sei();
}

void sys_waken_by_sw_off(void) {
	cli();
	GIFR|=_BV(INTF0)|_BV(PCIF1)|_BV(PCIF0);
	PCMSK0&=~_BV(PCINT3);
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
	PORTA|=_BV(PORTA2);
}

void dimmer_drv_off(void) {
	PORTA&=~_BV(PORTA2);
}

unsigned char ui_sw_get(void) {
	return PINA&_BV(PINA3);
}

void ui_led_on(void) {
	PORTB|=_BV(PORTB0);
}

void ui_led_off(void) {
	PORTB&=~_BV(PORTB0);
}

/*
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
*/

void vmon_ac_on(void) {
	ACSR&=~_BV(ACD);
}

void vmon_ac_off(void) {
	ACSR|=_BV(ACD);
}

BOOL vmon_ac_get(void) {
	return ACSR&_BV(ACO)?FALSE:TRUE;
}

#endif