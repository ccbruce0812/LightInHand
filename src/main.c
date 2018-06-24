#include "common.h"

#define PWM_LEVEL(a, b) (((unsigned int)(a))*(b)/100)

unsigned char EEMEM CH0_MAX=DEFAULT_CH0_MAX;
unsigned char EEMEM CH1_MAX=DEFAULT_CH1_MAX;
unsigned char EEMEM LEVEL=DEFAULT_LEVEL;
char EEMEM MAX_DELTA=1;
char EEMEM LEVEL_DELTA=1;

BOOL is_release(unsigned char *pcount) {
	if(ui_sw_get()) {
		if(*pcount<RELAX_THRESHOLD)
			(*pcount)++;
		else	
			return TRUE;
	}
	
	return FALSE;
}

void transit(unsigned char *pcount, unsigned int *plast_ts, unsigned char *plast_state,
			  unsigned char count, unsigned int ts, unsigned char state) {
	*pcount=count;
	*plast_ts=ts;
	*plast_state=state;
}

void on_shutdown(void) {
	ui_led_off();
	dimmer_drv_off();
	dimmer_pwm_off();
//	vmon_adc_off();
	vmon_ac_off();
	sys_counter_off();
	sys_waken_by_sw_on();

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	cli();
	sleep_enable();
	sleep_bod_disable();
	sei();
	sleep_cpu();
	sleep_disable();

	sys_waken_by_sw_off();
	sys_counter_on();
}

void on_boot(void) {
//	vmon_adc_on();
	vmon_ac_on();
	dimmer_pwm_on();
	dimmer_drv_on();
	ui_led_on();
}

void on_light_start(unsigned char *pch0_max, unsigned char *pch1_max, unsigned char *plevel, char *pdelta) {
	*pch0_max=eeprom_read_byte(&CH0_MAX);
	*pch1_max=eeprom_read_byte(&CH1_MAX);
	*plevel=eeprom_read_byte(&LEVEL);
	*pdelta=eeprom_read_byte((unsigned char *)&LEVEL_DELTA);
	
	dimmer_pwm_set(CH0, PWM_LEVEL(*pch0_max, *plevel));
	dimmer_pwm_set(CH1, PWM_LEVEL(*pch1_max, *plevel));
}

void on_light_run(unsigned char ch0_max, unsigned char ch1_max, unsigned char *plevel, char *pdelta) {
	if(*plevel==100)
		*pdelta=-1;
	else if(!*plevel)
		*pdelta=1;

	*plevel+=*pdelta;

	dimmer_pwm_set(CH0, PWM_LEVEL(ch0_max, *plevel));
	dimmer_pwm_set(CH1, PWM_LEVEL(ch1_max, *plevel));
}

void on_light_stop(unsigned char *plevel, char *pdelta) {
	eeprom_write_byte(&LEVEL, *plevel);
	eeprom_write_byte((unsigned char *)&LEVEL_DELTA, *pdelta);
}

#define on_breath0_start on_light_start

#define on_breath0_run on_light_run

#define on_breath1_start on_breath0_start

void on_breath1_run(unsigned char ch0_max, unsigned char ch1_max, unsigned char *plevel, char *pdelta) {
	if(*plevel==100)
		*pdelta=-1;
	else if(!*plevel)
		*pdelta=1;

	*plevel+=*pdelta;

	dimmer_pwm_set(CH0, PWM_LEVEL(ch0_max, *plevel));
	dimmer_pwm_set(CH1, PWM_LEVEL(ch1_max, 100-*plevel));
}

void on_candle_start(unsigned char ch0_max, unsigned char ch1_max) {
	dimmer_pwm_set(CH0, PWM_LEVEL(ch0_max, 50));
	dimmer_pwm_set(CH1, PWM_LEVEL(ch1_max, 50));
	srand(sys_ts_get());
}

void on_candle_run(unsigned char ch0_max, unsigned char ch1_max) {
	unsigned char level=50+(rand()%30-15);
	
	dimmer_pwm_set(CH0, PWM_LEVEL(ch0_max, level));
	dimmer_pwm_set(CH1, PWM_LEVEL(ch1_max, level));
}

void on_kelvin_start(unsigned char *pch0_max, unsigned char *pch1_max, char *pdelta, unsigned int *pled_interval) {
	*pch0_max=eeprom_read_byte(&CH0_MAX);
	*pch1_max=eeprom_read_byte(&CH1_MAX);
	*pdelta=eeprom_read_byte((unsigned char *)&MAX_DELTA);

	dimmer_pwm_set(CH0, PWM_LEVEL(*pch0_max, 80));
	dimmer_pwm_set(CH1, PWM_LEVEL(*pch1_max, 80));
	
	*pled_interval=SETUP_INTERVAL;
}

void on_kelvin_run(unsigned char *pch0_max, unsigned char *pch1_max, char *pdelta) {
	if(*pch0_max==255)
		*pdelta=-1;
	else if(*pch0_max==0)
		*pdelta=1;

	*pch0_max+=*pdelta;
	*pch1_max=255-*pch0_max;

	dimmer_pwm_set(CH0, PWM_LEVEL(*pch0_max, 80));
	dimmer_pwm_set(CH1, PWM_LEVEL(*pch1_max, 80));
}

void on_kelvin_stop(unsigned char *pch0_max, unsigned char *pch1_max, char *pdelta, unsigned int *pled_interval) {
	*pled_interval=0;
	
	eeprom_write_byte(&CH0_MAX, *pch0_max);
	eeprom_write_byte(&CH1_MAX, *pch1_max);
	eeprom_write_byte((unsigned char *)&MAX_DELTA, *pdelta);
}

void on_vmon(unsigned int *pled_interval, unsigned int orig_led_interval) {
/*
	unsigned char val=vmon_adc_get();
	
	if(val>=210) {			//vin>=3.0V => on
		dimmer_drv_on();
		dimmer_pwm_on();
		*pled_interval=orig_led_interval;
	} else {				//3.0V>vin => off
		dimmer_drv_off();
		dimmer_pwm_off();
		*pled_interval=LBAT_INTERVAL;
	}
*/

	if(vmon_ac_get()) {
		dimmer_drv_on();
		dimmer_pwm_on();
		*pled_interval=orig_led_interval;
	} else {
		dimmer_drv_off();
		dimmer_pwm_off();
		*pled_interval=LBAT_INTERVAL;
	}
}

#ifndef TEST
int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;
	
	unsigned int last_ui_ts=0, last_dim_ts=0, last_blink_ts=0, last_vmon_ts=0, last_led_ts=0, led_interval=0;
	unsigned char last_state=STATE_UP, count=0, mode=MODE_SHUTDOWN, led_state=TRUE;

	unsigned char ch0_max=DEFAULT_CH0_MAX, ch1_max=DEFAULT_CH1_MAX, level=DEFAULT_LEVEL;
	char level_delta=DEFAULT_DELTA, max_delta=DEFAULT_DELTA;

	sys_init();

	while(1) {
		unsigned int ts=sys_ts_get();

		if(ts<last_ui_ts)
			last_ui_ts=ts;
		
		if(ts<last_dim_ts)
			last_dim_ts=ts;
		
		if(ts<last_blink_ts)
			last_blink_ts=ts;
		
		if(ts<last_vmon_ts)
			last_vmon_ts=ts;
		
		if(ts<last_led_ts)
			last_led_ts=ts;
		
		if(!led_interval) {
			last_led_ts=ts;
			led_state=TRUE;
			ui_led_on();
		} else {
			if(ts-last_led_ts>=led_interval) {
				if(led_state) {
					ui_led_on();
					led_state=FALSE;
				} else {
					ui_led_off();
					led_state=TRUE;
				}
				
				last_led_ts=ts;
			}
		}
		
		switch(last_state) {
			case STATE_UP:
				if(!ui_sw_get())
					transit(&count, &last_ui_ts, &last_state, 0, ts, STATE_DOWN);
				else {
					if(mode==MODE_SHUTDOWN)
						on_shutdown();
					else {
						if(ts-last_vmon_ts>=VMON_INTERVAL) {
							on_vmon(&led_interval, mode==MODE_KELVIN?SETUP_INTERVAL:0);
							last_vmon_ts=ts;
						}
						
						switch(mode) {
							case MODE_BREATH0:
								if(ts-last_dim_ts>=DIM_INTERVAL) {
									on_breath0_run(ch0_max, ch1_max, &level, &level_delta);
									last_dim_ts=ts;
								}
								break;
								
							case MODE_BREATH1:
								if(ts-last_dim_ts>=DIM_INTERVAL) {
									on_breath1_run(ch0_max, ch1_max, &level, &level_delta);
									last_dim_ts=ts;
								}
								break;
								
							case MODE_CANDLE:								
								if(ts-last_blink_ts>=BLINK_INTERVAL) {
									on_candle_run(ch0_max, ch1_max);
									last_blink_ts=ts;
								}
								break;
								
							default:
								;
						}
					}
				}
				break;

			case STATE_DOWN:
				if(is_release(&count)) {
					last_state=STATE_UP;
					break;
				}
			
				if(ts-last_ui_ts>=DOWN_THRESHOLD)
					transit(&count, &last_ui_ts, &last_state, 0, ts, STATE_PRESS);
				break;

			case STATE_PRESS:
				if(is_release(&count)) {
					if(ts-last_ui_ts<PRESS_THRESHOLD) {
						switch(mode) {
							case MODE_SHUTDOWN:
								on_boot();
								on_light_start(&ch0_max, &ch1_max, &level, &level_delta);
								break;
								
							case MODE_LIGHT:
								on_light_stop(&level, &level_delta);
								on_breath0_start(&ch0_max, &ch1_max, &level, &level_delta);
								break;
								
							case MODE_BREATH0:
								on_breath1_start(&ch0_max, &ch1_max, &level, &level_delta);
								break;
								
							case MODE_BREATH1:
								on_candle_start(ch0_max, ch1_max);
								break;
								
							case MODE_CANDLE:
								on_kelvin_start(&ch0_max, &ch1_max, &max_delta, &led_interval);
								break;
								
							case MODE_KELVIN:
								on_kelvin_stop(&ch0_max, &ch1_max, &max_delta, &led_interval);
								break;
								
							default:
								;
						}
					
						mode=(mode+1)%MODE_MAX;
					}

					last_state=STATE_UP;
					break;
				}
				
				if(ts-last_ui_ts>=PRESS_THRESHOLD) {
					switch(mode) {
						case MODE_LIGHT:
							if(ts-last_dim_ts>=DIM_INTERVAL) {
								on_vmon(&led_interval, 0);
								on_light_run(ch0_max, ch1_max, &level, &level_delta);
								last_dim_ts=ts;
							}
							break;
							
						case MODE_KELVIN:
							if(ts-last_dim_ts>=DIM_INTERVAL) {
								on_vmon(&led_interval, SETUP_INTERVAL);
								on_kelvin_run(&ch0_max, &ch1_max, &max_delta);
								last_dim_ts=ts;
							}
							break;
							
						default:
							;
					}
				}
				break;

			default:
				;
		}
	}

	return 0;
}
#endif
