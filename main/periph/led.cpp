#include "led.h"
#include "pwm.h"
#include "timer.h"
#include "nrf_log.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "stdint.h"

namespace RuningStatus {

/* LED status pin */
constexpr static const uint8_t LED_STATUS_PIN 			= NRF_GPIO_PIN_MAP(0,6);
constexpr static const uint32_t LED_INTERVAL_TIME_MS 	= 10;
constexpr static const float LED_STEP_RANGE 			= 0.01f;
static Wrapper::PWM *_led = nullptr;
static float _led_duty = 1.0f;
static bool _add = true;


/* system running status display */
void sys_state_task(uint32_t param) {
	if (_led_duty >= 1.0f ) {
		_add = false;
	}
	if (_led_duty <= 0.0f) {
		_add = true;
	}
	if (_add) {
		_led_duty += LED_STEP_RANGE;
	} else {
		_led_duty -= LED_STEP_RANGE;
	}

	_led->update(_led_duty);
}


// indicator light init
void init(void) {
	// LED configure
	_led = new Wrapper::PWM(Wrapper::PWM::UNIT0, 0, LED_STATUS_PIN);
	_led->start(_led_duty);
	
	// set timer callback task
	Wrapper::AppTimer::Task(sys_state_task, LED_INTERVAL_TIME_MS);
}

}
