#include "common.h"

#ifdef TEST
#ifdef TEST_0

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	unsigned int last_ts=0, last_drv_ts=0;
	unsigned char last_level=0;
	BOOL last_drv_state=FALSE;

	sys_init();
	sys_counter_on();
	vin_adc_on();
	dimmer_pwm_on();
	dimmer_drv_on();

	while(1) {
		unsigned int ts=sys_ts_get();
		
		/*
		if(ts-last_ts>=VIN_TESTING_INTERVAL) {
			dimmer_pwm_set(TRUE, vin_adc_get()/2);
			dimmer_pwm_set(FALSE, vin_adc_get());
			
			last_ts=ts;
		}
		*/
		
		if(ts-last_ts>=DIMMING_INTERVAL) {
			dimmer_pwm_set(TRUE, last_level);
			dimmer_pwm_set(FALSE, 255-last_level);
			last_level++;
			
			last_ts=ts;
		}
		
		if(ts-last_drv_ts>=1000) {
			if(last_drv_state) {
				dimmer_drv_off();
				last_drv_state=FALSE;
			} else {
				dimmer_drv_on();
				last_drv_state=TRUE;
			}
			
			last_drv_ts=ts;
		}
		
		if(!ui_get())
			dimmer_drv_on();
	}

	return 0;
}

#endif
#endif