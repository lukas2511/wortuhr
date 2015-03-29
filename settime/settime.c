#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <time.h>

#define SYSTEM_LED GPIO13
#define SELECTOR_PIN GPIO7

int main(void) {
	/* clock setup */
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	/* rtc setup */
	rtc_auto_awake(LSE, 0x7fff);
	rtc_interrupt_enable(RTC_SEC);

	/* nvic setup */
	nvic_enable_irq(NVIC_RTC_IRQ);
	nvic_set_priority(NVIC_RTC_IRQ, 1);
	rtc_enter_config_mode();
	rtc_set_counter_val(__COUNTERVAL__);

	while(1);
	return 0;
}
