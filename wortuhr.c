#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <time.h>
#include <stdlib.h>

#include "wortuhr.h"

#define SYSTEM_LED GPIO13
#define SELECTOR_PIN GPIO7

int is_dst(struct tm time);

// check if given time should be dst (timezones didn't work so i have to use this ugly workaround...)
int is_dst(struct tm time) {
	if(time.tm_mon > 2 && time.tm_mon < 9) return 1; // april - september is dst
	if(time.tm_mon == 9 && time.tm_mday < 25) return 1; // october up to last week is dst

	if(time.tm_mon < 2 || time.tm_mon > 9) return 0; // november - february is not dst
	if(time.tm_mon == 2 && time.tm_mday < 25) return 0; // march up to last week is not dst

	// idea: calculate remaining days of week and remaining days of month
	// if remaining days of week > remaining days of month we are on or after the last sunday of the month
	// for sunday we also check if it is >=3 hours after midnight in utc
	int rem_days_of_month = 31 - time.tm_mday;
	int rem_days_of_week = 6 - time.tm_wday;
	if((time.tm_mon == 2 && (rem_days_of_week >= rem_days_of_month)) || (time.tm_mon == 10 && (rem_days_of_week <= rem_days_of_month))) {
		if(time.tm_wday == 0) {
			return (time.tm_hour >= 2);
		} else {
			return 1;
		}
	}

	return 0;
}

// called every second by the rtc
void rtc_isr(void)
{
	// clear interrupt flag and toggle the onboard led (backside of clock, not visible from front)
	rtc_clear_flag(RTC_SEC);
	gpio_toggle(GPIOC, GPIO13);

	// get unixtime from rtc and calculate date from it
	time_t epoch = rtc_get_counter_val();
	struct tm time = *gmtime(&epoch);
	int minute = time.tm_min;
	int hour = time.tm_hour+1; // +1 hour for time in germany (without dst)
	if(is_dst(time)) hour++; // check if dst and add one hour if necessary

	// clock logic
	CLEAR_SCREEN
	ES IST

	if(minute <= 2) { UHR }
	if(minute >=  3 && minute <=  7) { FUENF_1 NACH }
	if(minute >=  8 && minute <= 12) { ZEHN_1 NACH }
	if(minute >= 13 && minute <= 17) { VIER_1 TEL NACH }

	if(minute >= 18) hour++;

	if(minute >= 18 && minute <= 22) { ZEHN_1 VOR_1 HALB }
	if(minute >= 23 && minute <= 27) { FUENF_1 VOR_1 HALB }
	if(minute >= 28 && minute <= 32) { HALB }
	if(minute >= 33 && minute <= 37) { FUENF_1 NACH HALB }
	if(minute >= 38 && minute <= 42) { ZEHN_1 NACH HALB }
	if(minute >= 43 && minute <= 47) { VIER_1 TEL VOR_2 }
	if(minute >= 48 && minute <= 52) { ZEHN_1 VOR_1 }
	if(minute >= 53 && minute <= 57) { FUENF_1 VOR_1 }
	if(minute >= 58) { UHR }

	hour = hour % 12;

	if(hour == 0) { ZWOELF }
	if(hour == 1) { EIN if(minute >= 3 && minute <= 58) S } // "ES IST EIN UHR" vs "ES IST FUENF VOR EINS"
	if(hour == 2) { ZWEI }
	if(hour == 3) { DREI_2 }
	if(hour == 4) { VIER_2 }
	if(hour == 5) { FUENF_2 }
	if(hour == 6) { SECHS }
	if(hour == 7) { SIEBEN }
	if(hour == 8) { ACHT }
	if(hour == 9) { NEUN }
	if(hour == 10) { ZEHN_2 }
	if(hour == 11) { ELF }
}


int main(void) {
	/* clock setup */
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	/* gpio setup */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);
	rcc_periph_clock_enable(RCC_GPIOC);

	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO6 | SELECTOR_PIN);
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, SEGMENT_PINS);
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, SYSTEM_LED);

	gpio_set(GPIOA, GPIO6); // disable usb
	gpio_set(GPIOC, GPIO13); // disable led

	/* rtc setup */
	rtc_auto_awake(LSE, 0x7fff);
	rtc_interrupt_enable(RTC_SEC);

	/* nvic setup */
	nvic_enable_irq(NVIC_RTC_IRQ);
	nvic_set_priority(NVIC_RTC_IRQ, 1);

	int tmp=0;
	while(1){
		// set pins for top segment + slight delay
		gpio_set(GPIOA, SELECTOR_PIN);
		gpio_port_write(GPIOB, top_half);
		for(tmp = 0; tmp < 10000; tmp++) __asm__("nop");

		// set pins for bottom segment + slight delay
		gpio_clear(GPIOA, SELECTOR_PIN);
		gpio_port_write(GPIOB, bottom_half);
		for(tmp = 0; tmp < 10000; tmp++) __asm__("nop");
	}

	return 0;
}
