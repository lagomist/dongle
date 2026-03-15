#include "usb_cli.h"
#include "timer.h"
#include "utils.h"
#include "nrfx_clock.h"
#include "nrf_delay.h"
#include "app_error.h"
#include "app_util.h"
#include "nrf_drv_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#include "nrf_cli_cdc_acm.h"
#include "nrf_cli.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_backend_interface.h"
#include <stdint.h>
#include <string.h>
#define NRF_LOG_MODULE_NAME usb_cdc
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

namespace usb_cli {

constexpr static uint8_t const CLI_LOG_QUEUE_SIZE = 5;

static Wrapper::AppTimer::Task _task_handle;
static bool _usb_connected = false;
static bool _cli_started = false;
static bool _usb_log_requested = false;
static int32_t _usb_log_backend_id = NRF_LOG_BACKEND_INVALID_ID;

/** @brief Command line interface instance */
NRF_CLI_CDC_ACM_DEF(_cli_cdc_acm_transport);

static nrf_cli_ctx_t CONCAT_2(_usb_cdc_cli, _ctx);
NRF_LOG_BACKEND_CLI_DEF(CONCAT_2(_usb_cdc_cli, _log_backend), CLI_LOG_QUEUE_SIZE);
NRF_CLI_HISTORY_MEM_OBJ(_usb_cdc_cli);
static nrf_cli_t _usb_cdc_cli = {
	.p_name = "usb_cli:~$ ",
	.p_iface = &_cli_cdc_acm_transport.transport,
	.p_ctx = &CONCAT_2(_usb_cdc_cli, _ctx),
	.p_log_backend = NRF_CLI_BACKEND_PTR(_usb_cdc_cli),
	.p_fprintf_ctx = nullptr,
	.p_cmd_hist_mempool = NRF_CLI_MEMOBJ_PTR(_usb_cdc_cli),
};
NRF_FPRINTF_DEF(CONCAT_2(_usb_cdc_cli, _fprintf_ctx),
				&_usb_cdc_cli,
				CONCAT_2(_usb_cdc_cli, _ctx).printf_buff,
				NRF_CLI_PRINTF_BUFF_SIZE,
				false,
				nrf_cli_print_stream);

static void usbd_user_ev_handler(app_usbd_event_type_t event) {
    switch (event) {
        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            break;
        case APP_USBD_EVT_POWER_DETECTED:
            if (!nrf_drv_usbd_is_enabled()) {
                app_usbd_enable();
            }
            break;
        case APP_USBD_EVT_POWER_REMOVED:
			_usb_connected = false;
			nrf_log_backend_disable(_usb_cdc_cli.p_log_backend);
            app_usbd_stop();
            break;
        case APP_USBD_EVT_POWER_READY:
			_usb_connected = true;
            app_usbd_start();
			if (_usb_log_requested && _cli_started) {
				nrf_log_backend_enable(_usb_cdc_cli.p_log_backend);
			}
            break;
        default:
            break;
    }
}

static void usb_log_backend_register(void) {
	if (_usb_log_backend_id != NRF_LOG_BACKEND_INVALID_ID) {
		return;
	}

	_usb_log_backend_id = nrf_log_backend_add(_usb_cdc_cli.p_log_backend, NRF_LOG_SEVERITY_DEBUG);
	if (_usb_log_backend_id < 0) {
		APP_ERROR_CHECK(NRF_ERROR_NO_MEM);
	}
	nrf_log_backend_disable(_usb_cdc_cli.p_log_backend);
}

int option_printf(const char format[], ...) {
	va_list args;
	va_start(args, format);
	auto out = Utils::vsprint(format, args);
	va_end(args);
	nrf_cli_fprintf(&_usb_cdc_cli, NRF_CLI_OPTION, out.data());
	return 0;
}

int write(std::string_view str) {
	nrf_cli_fprintf(&_usb_cdc_cli, NRF_CLI_INFO, str.data());
	return 0;
}

int write(const char *p_fmt, ...) {
	va_list args;
    va_start(args, p_fmt);

	nrf_fprintf_fmt(_usb_cdc_cli.p_fprintf_ctx, p_fmt, &args);
    
    va_end(args);
    return 0;
}

void set_log_output(bool enable) {
	_usb_log_requested = enable;
	if (_usb_log_backend_id == NRF_LOG_BACKEND_INVALID_ID) {
		return;
	}
	if (enable && _usb_connected && _cli_started) {
		nrf_log_backend_enable(_usb_cdc_cli.p_log_backend);
	} else {
		nrf_log_backend_disable(_usb_cdc_cli.p_log_backend);
	}
}

bool log_output_enabled() {
	return (_usb_log_backend_id != NRF_LOG_BACKEND_INVALID_ID) &&
				nrf_log_backend_is_enabled(_usb_cdc_cli.p_log_backend);
}

static void process(void *arg) {
	nrf_cli_t *cdc_cli_inst = (nrf_cli_t *)arg;
	nrf_cli_process(cdc_cli_inst);
}

/**
 * @brief 初始化USB设备、电源（协议栈初始化之前调用）
 * 
 * @return int 
 */
int init(void) {
	ret_code_t ret;

	_usb_cdc_cli.p_fprintf_ctx = &_usb_cdc_cli_fprintf_ctx;
	static const app_usbd_config_t usbd_config = {
        .ev_handler = app_usbd_event_execute,
        .ev_state_proc = usbd_user_ev_handler,
    };

	ret = app_usbd_init(&usbd_config);
	APP_ERROR_CHECK(ret);

	app_usbd_class_inst_t const *class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&nrf_cli_cdc_acm);
	ret = app_usbd_class_append(class_cdc_acm);
	APP_ERROR_CHECK(ret);

	ret = nrf_cli_init(&_usb_cdc_cli, NULL, true, false, NRF_LOG_SEVERITY_NONE);
    APP_ERROR_CHECK(ret);
	usb_log_backend_register();

	_task_handle.create(process, Wrapper::AppTimer::CALL_IMMEDIATE, &_usb_cdc_cli);
	_task_handle.suspend();
	
	NRF_LOG_INFO("init success.");
	return 0;
}

/**
 * @brief 使能电源事件（协议栈初始化后调用）
 * 
 */
void enable() {
	ret_code_t ret = app_usbd_power_events_enable();
	APP_ERROR_CHECK(ret);
	/* Give some time for the host to enumerate and connect to the USB CDC port */
	nrf_delay_ms(1000);
	ret = nrf_cli_start(&_usb_cdc_cli);
    APP_ERROR_CHECK(ret);
	_cli_started = true;
	set_log_output(_usb_log_requested);
	_task_handle.activate();
	NRF_LOG_INFO("enabled.");
}

}
