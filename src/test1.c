#include "common.h"

#ifdef TEST
#ifdef TEST_1

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	unsigned long ts;
	
	sys_init();
	
	while(1) {
		debug_led_on();

		dimmer_drv_off();
		dimmer_pwm_off();
		vin_adc_off();
		sys_counter_off();
		sys_waken_by_ui_on();

		while(1) {
			if(!ui_get())
				break;
		}
	
		sys_waken_by_ui_off();
		sys_counter_on();
		vin_adc_on();
		dimmer_pwm_on();
		dimmer_drv_on();

		debug_led_off();
		
		ts=sys_ts_get();
		while(sys_ts_get()-ts<5000)
			_NOP();
	}
}

#endif
#endif