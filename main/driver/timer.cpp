#include "timer.h"

#include "nrf_sdh.h"
#include "nrfx_clock.h"
#include "app_error.h"
#include "sdk_errors.h"
#include "nrfx_rtc.h"
#include <cstdlib>

namespace Wrapper {

namespace AppTimer {

struct SliceInstance {
    Action action;
    uint32_t param;
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
            delete (current);
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


Task::Task(Action task, uint32_t period, uint32_t param, int count) {
	create(task, period, param, count);
}

int Task::create(Action task, uint32_t period, uint32_t param, int count) {
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

uint32_t Task::control(Cmd cmd, uint32_t param) {
    if (_handle == nullptr) {
        return -1;
    }
    InstanceList *handle = (InstanceList *)_handle;
    
    switch(cmd) {
        case Cmd::PAUSE: {
            handle->content.active = 0;
            break;
        }
        case Cmd::RESUME: {
            handle->content.active = 1;
            break;
        }
        case Cmd::KILL: {
            handle->content.active = 0;
            delete_slice_instance(handle);
            _handle = nullptr;
            break;
        }
        case Cmd::PERIOD: {
            handle->content.period = param;
            handle->content.tick = param;
            break;
        }
        case Cmd::PARAM: {
            handle->content.param = param;
            break;
        }
        case Cmd::COUNT: {
            handle->content.count = param;
            handle->content.active = 1;
            break;
        }
        case Cmd::RESTART: {
            handle->content.tick = handle->content.period;
            handle->content.active = 1;
            break;
        }
        case Cmd::ACTIVATED:  
            return handle->content.active;
        case Cmd::REMAINING:
            return handle->content.tick * TICK_PERIOD_MS;
        default: break;
    }
    return -1;
}

static void clock_irq_handler(nrfx_clock_evt_type_t evt) {
    if (evt == NRFX_CLOCK_EVT_HFCLK_STARTED) {
    }
    if (evt == NRFX_CLOCK_EVT_LFCLK_STARTED) {
    }
}

void init(void) {
	/* use the RTC timer */
    if (nrfx_clock_init(clock_irq_handler) == NRFX_SUCCESS) {
        if (!nrf_sdh_is_enabled()) {
            nrfx_clock_enable();
            nrfx_clock_lfclk_start();
        }
    }

	nrfx_rtc_config_t config = {
		/* 1 / (32768/33) = 1ms */
		.prescaler          = 33,
		.interrupt_priority = 3,
		.tick_latency       = NRFX_RTC_US_TO_TICKS(2000, 32768),
		.reliable           = false,
	};
	
	// use RTC2 write in struct and interrupt function
	ret_code_t err_code = nrfx_rtc_init(&_rtc, &config, timer_callback_handler);
	APP_ERROR_CHECK(err_code);
	// enable RTC interrupt handle
	nrfx_rtc_tick_enable(&_rtc, true);
	nrfx_rtc_enable(&_rtc);
}

}
}