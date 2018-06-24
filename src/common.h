#ifndef __COMMON_H__
#define __COMMON_H__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <avr/cpufunc.h>
#include <stdint.h>
#include <util/delay.h>

typedef unsigned char BOOL;

#define TRUE					(-1)
#define FALSE					(0)

#define DOWN_THRESHOLD			(30)
#define PRESS_THRESHOLD			(500)
#define RELAX_THRESHOLD	(5)

#define DEFAULT_CH0_MAX			(127)
#define DEFAULT_CH1_MAX			(128)
#define DEFAULT_LEVEL			(50)
#define DEFAULT_DELTA			(1)

#define DIM_INTERVAL			(50)
#define BLINK_INTERVAL			(100)
#define VMON_INTERVAL			(5000)
#define LBAT_INTERVAL			(250)
#define SETUP_INTERVAL			(500)

enum {
	STATE_UP=0,
	STATE_DOWN,
	STATE_PRESS
};

enum {
	MODE_LIGHT=0,
	MODE_BREATH0,
	MODE_BREATH1,
	MODE_CANDLE,
	MODE_KELVIN,
	MODE_SHUTDOWN,
	MODE_MAX
};

enum {
	CH0=0,
	CH1
};

void srand(uint16_t seed);
uint16_t rand(void);

void sys_init(void);
unsigned int sys_ts_get(void);
void sys_counter_on(void);
void sys_counter_off(void);
void sys_waken_by_sw_on(void);
void sys_waken_by_sw_off(void);

void dimmer_pwm_on(void);
void dimmer_pwm_off(void);
unsigned char dimmer_pwm_get(int ch);
void dimmer_pwm_set(int ch, unsigned char val);
void dimmer_drv_on(void);
void dimmer_drv_off(void);

unsigned char ui_sw_get(void);
void ui_led_on(void);
void ui_led_off(void);

/*
void vmon_adc_on(void);
void vmon_adc_off(void);
unsigned char vmon_adc_get(void);
*/

void vmon_ac_on(void);
void vmon_ac_off(void);
BOOL vmon_ac_get(void);

#endif
