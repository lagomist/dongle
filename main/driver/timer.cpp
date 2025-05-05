#include "timer.h"

#include "nrf_sdh.h"
#include "nrf_drv_clock.h"
#include "app_error.h"
#include "sdk_errors.h"
#include "nrfx_rtc.h"
#define NRF_LOG_MODULE_NAME AppTimer
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();
#include <cstdlib>

namespace Wrapper {

namespace AppTimer {

struct SliceInstance {
    Action action;
    void *param;
    int count;
    uint32_t period;
    volatile uint32_t tick;
    uint8_t active;
};

struct InstanceList {
    SliceInstance content;
    struct InstanceList *next;
};

static const nrfx_rtc_t _rtc = NRFX_RTC_INSTANCE(2);
static InstanceList * _inst_head = nullptr;
static volatile uint32_t _timer_tick = 0;

static InstanceList* add_slice_instance(SliceInstance &inst) {  
    InstanceList *new_inst = (InstanceList* )malloc(sizeof(InstanceList));  
    if (new_inst == nullptr) return nullptr;
    if (_inst_head == nullptr) {
        _inst_head = new_inst;
    } else {
        InstanceList *current = _inst_head;
        while (current->next != nullptr) {
            current = current->next;
        }
        current->next = new_inst;
    }

    // set new node info
    new_inst->content = inst;
    new_inst->next = nullptr;
    return new_inst;
}


static void delete_slice_instance(InstanceList *inst) {
    if (inst == nullptr) return;
    InstanceList *current = _inst_head;
    InstanceList *prev = nullptr;

    while (current != nullptr) {
        if (current == inst) {
            if (prev == nullptr) {
                _inst_head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }
}


/* hardware timer callback */
static void timer_callback_handler(nrfx_rtc_int_type_t event_type) {
    for (InstanceList *inst = _inst_head; inst; inst = inst->next) {
        if (inst->content.active) {
            inst->content.tick += TICK_PERIOD_MS;
        }
    }
    _timer_tick++;
}

// timer task process call from main
void sliceProcess(void) {
    for (InstanceList *inst = _inst_head; inst; inst = inst->next) {
        if (inst->content.active && inst->content.action && inst->content.tick >= inst->content.period) {
            inst->content.action(inst->content.param);
            inst->content.tick = 0;

            // loop count
            if (inst->content.count > 0) {
                inst->content.count--;
                if (inst->content.count == 0) {
                    // halt action
                    inst->content.active = 0;
                }
            }
        }
    }
}


Task::Task(Action task, uint32_t period, void *param, int count) {
	create(task, period, param, count);
}

int Task::create(Action task, uint32_t period, void *param, int count) {
    SliceInstance inst = {
        .action = task,
        .param  = param,
        .count  = count,
        .period = period,
        .tick   = period,
        .active = 1,
    };

    InstanceList *handle = add_slice_instance(inst);
    if (handle == nullptr) {
        return -1;
    }
    _handle = handle;
    // activate
    handle->content.active = 1;

	return 0;
}

void Task::activate() {
    if (_handle == nullptr) return;
    InstanceList *handle = (InstanceList *)_handle;
    handle->content.active = 1;
}

void Task::suspend() {
    if (_handle == nullptr) return;
    InstanceList *handle = (InstanceList *)_handle;
    handle->content.active = 0;
}

void Task::restart() {
    if (_handle == nullptr) return;
    InstanceList *handle = (InstanceList *)_handle;
    handle->content.tick = handle->content.period;
    handle->content.active = 1;
}

void Task::kill() {
    if (_handle == nullptr) return;
    InstanceList *handle = (InstanceList *)_handle;
    handle->content.active = 0;
    delete_slice_instance(handle);
    _handle = nullptr;
}

void Task::setPeriod(uint32_t period) {
    if (_handle == nullptr) return;
    InstanceList *handle = (InstanceList *)_handle;
    handle->content.period = period;
    handle->content.tick = period;
}

void Task::setParam(void *param) {
    if (_handle == nullptr) return;
    InstanceList *handle = (InstanceList *)_handle;
    handle->content.param = param;
}

void Task::setCycleCount(uint32_t count) {
    if (_handle == nullptr) return;
    InstanceList *handle = (InstanceList *)_handle;
    handle->content.count = count;
    handle->content.active = 1;
}
bool Task::isActivated() {
    if (_handle == nullptr) return false;
    InstanceList *handle = (InstanceList *)_handle;
    return handle->content.active;
}

uint32_t Task::getRemainingTime() {
    if (_handle == nullptr) return -1;
    InstanceList *handle = (InstanceList *)_handle;
    return handle->content.tick * TICK_PERIOD_MS;
}



Timer::Timer(Action cb, uint32_t period, void *param, int count) {
    create(cb, period, param, count);
}

int Timer::create(Action cb, uint32_t period, void *param, int count) {
    SliceInstance inst = {
        .action = cb,
        .param  = param,
        .count  = count,
        .period = period,
        .tick   = 0,
        .active = false,
    };

    InstanceList *handle = add_slice_instance(inst);
    if (handle == nullptr) {
        return -1;
    }
    _cycle_count = count;
    _handle = handle;
    return 0;
}

void Timer::start() {
    if (_handle == nullptr) return;
    InstanceList *handle = (InstanceList *)_handle;
    handle->content.active = 1;
}

void Timer::restart() {
    if (_handle == nullptr) return;
    InstanceList *handle = (InstanceList *)_handle;
    handle->content.count = _cycle_count;
    handle->content.tick = 0;
    handle->content.active = 1;
}

void Timer::stop() {
    if (_handle == nullptr) return;
    InstanceList *handle = (InstanceList *)_handle;
    handle->content.active = 0;
}

void Timer::kill() {
    if (_handle == nullptr) return;
    InstanceList *handle = (InstanceList *)_handle;
    handle->content.active = 0;
    delete_slice_instance(handle);
    _handle = nullptr;
}

void Timer::setPeriod(uint32_t period) {
    if (_handle == nullptr) return;
    InstanceList *handle = (InstanceList *)_handle;
    handle->content.period = period;
    handle->content.tick = 0;
}

void Timer::setCycleCount(uint32_t count) {
    if (_handle == nullptr) return;
    InstanceList *handle = (InstanceList *)_handle;
    _cycle_count = count;
    handle->content.count = count;
    handle->content.active = 1;
    handle->content.tick = 0;
}

bool Timer::isRuning() {
    if (_handle == nullptr) return false;
    InstanceList *handle = (InstanceList *)_handle;
    return handle->content.active;
}

uint32_t Timer::getRemainingTime() {
    if (_handle == nullptr) return -1;
    InstanceList *handle = (InstanceList *)_handle;
    return handle->content.tick * TICK_PERIOD_MS;
}

uint32_t getTick() {
    return _timer_tick;
}


void init(void) {
	/* use the RTC timer */
    ret_code_t err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);
    nrf_drv_clock_lfclk_request(NULL);

	nrfx_rtc_config_t config = {
		/* 1 / (32768/33) = 1ms */
		.prescaler          = 33,
		.interrupt_priority = 3,
		.tick_latency       = NRFX_RTC_US_TO_TICKS(2000, 32768),
		.reliable           = false,
	};
	
	// use RTC2 write in struct and interrupt function
	err_code = nrfx_rtc_init(&_rtc, &config, timer_callback_handler);
	APP_ERROR_CHECK(err_code);
	// enable RTC interrupt handle
	nrfx_rtc_tick_enable(&_rtc, true);
	nrfx_rtc_enable(&_rtc);
    NRF_LOG_INFO("init success.");
}

}
}