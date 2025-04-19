#include "pwm.h"
#include "nrf_gpio.h"
#include "nrfx_pwm.h"
#include <cstring>

namespace Wrapper {

constexpr static const uint16_t PWM_TOP_VALUE = 10000;
static nrfx_pwm_t _pwm_unit[3] = {};
static nrfx_pwm_config_t _pwm_cfg[3] = {};
static nrf_pwm_values_individual_t _pwm_sequence[3] = {};



// PWM event handler
static void pwm_evt_handler(nrfx_pwm_evt_type_t event_type) {
	
}

void set_pwm_unit_instance(PWM::Unit unit) {
	size_t index = (size_t )unit;
	switch (unit) {
#if NRFX_CHECK(NRFX_PWM0_ENABLED)
	case PWM::Unit::UNIT0:  _pwm_unit[index] = NRFX_PWM_INSTANCE(0); break;
#endif
#if NRFX_CHECK(NRFX_PWM0_ENABLED)
	case PWM::Unit::UNIT1:  _pwm_unit[index] = NRFX_PWM_INSTANCE(1); break;
#endif
#if NRFX_CHECK(NRFX_PWM0_ENABLED)
	case PWM::Unit::UNIT2:  _pwm_unit[index] = NRFX_PWM_INSTANCE(2); break;
#endif
	default:
		return;
	}
}

void set_pwm_sequence_val(PWM::Unit unit, uint8_t channel, float duty) {
	size_t index = (size_t )unit;
	uint16_t val = PWM_TOP_VALUE * duty;
	switch (channel) {
	case 0:  _pwm_sequence[index].channel_0 = val; break;
	case 1:  _pwm_sequence[index].channel_1 = val; break;
	case 2:  _pwm_sequence[index].channel_2 = val; break;
	case 3:  _pwm_sequence[index].channel_3 = val; break;
	default:
		return;
	}
}

PWM::PWM(Unit unit, uint8_t channel, uint8_t pin) : _unit(unit), _channel(channel) {
	size_t index = (size_t )unit;
	if (_pwm_unit[index].p_registers == nullptr ) {
		set_pwm_unit_instance(unit);
		memset(_pwm_cfg[index].output_pins, NRFX_PWM_PIN_NOT_USED, 4);
		_pwm_cfg[index].irq_priority = APP_IRQ_PRIORITY_LOWEST;		// 中断优先级
		_pwm_cfg[index].base_clock   = NRF_PWM_CLK_1MHz;       		// PWM时钟频率设置为1MHz  
		_pwm_cfg[index].count_mode   = NRF_PWM_MODE_UP;        		// 向上计数模式
		_pwm_cfg[index].top_value    = PWM_TOP_VALUE;              	// 最大计数值10000 (10ms)
		_pwm_cfg[index].load_mode    = NRF_PWM_LOAD_INDIVIDUAL; 	// 独立装载模式
		_pwm_cfg[index].step_mode    = NRF_PWM_STEP_AUTO;      		// 序列中的周期自动推进
	}
	nrf_gpio_cfg_output(pin);
	_pwm_cfg[index].output_pins[channel] = pin;
	nrfx_pwm_init(&_pwm_unit[index], &_pwm_cfg[index], pwm_evt_handler);
}

int PWM::start(float duty) {
	size_t index = (size_t )_unit;
	set_pwm_sequence_val(_unit, _channel, duty);
	// 定义PWM播放序列，播放序列包含了PWM序列的起始地址、大小和序列播放控制描述
	nrf_pwm_sequence_t sequence = {
		.values = {.p_individual = &_pwm_sequence[index]},
		.length           = 4, // PWM序列中包含的周期个数
		.repeats          = 0, // 序列中周期重复次数为0
		.end_delay        = 0  // 序列后不插入延时
	};
    // 启动PWM播放
    nrfx_pwm_simple_playback(&_pwm_unit[index], &sequence, 1, NRFX_PWM_FLAG_LOOP);
	
	return 0;
}

void PWM::update(float duty) {
	size_t index = (size_t )_unit;
	set_pwm_sequence_val(_unit, _channel, duty);
	nrf_pwm_values_t seq_value;
	seq_value.p_individual = &_pwm_sequence[index];
	nrfx_pwm_sequence_values_update(&_pwm_unit[index], 0, seq_value);
}


}
