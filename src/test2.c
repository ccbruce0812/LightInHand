#include "common.h"

#ifdef TEST
#ifdef TEST_2

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	unsigned long last_ui_ts, last_blink_ts, last_vmon_ts;
	int last_ui_state, last_led_state, last_blink_state;
	unsigned int interval=0;
	
	sys_init();
	sys_counter_on();
	vmon_adc_on();

	last_vmon_ts=0;
	last_blink_ts=0;
	last_blink_state=0;
	debug_led_off();
	dimmer_drv_off();
	
	while(1) {
		unsigned long ts=sys_ts_get();
		
		if(ts-last_vmon_ts>=1000) {
			interval=vmon_adc_get();
			last_vmon_ts=ts;
			
			interval=interval*100/256;
		}
		
		if(ts-last_blink_ts>=interval) {
			if(!last_blink_state) {
				last_blink_state=-1;
				debug_led_on();
			} else {
				last_blink_state=0;
				debug_led_off();
			}
			last_blink_ts=ts;
		}
		
		_delay_ms(10);
	}
}

#endif
#endif