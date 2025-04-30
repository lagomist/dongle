#include "usb_cli.h"
#include "timer.h"
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
#include <stdint.h>
#include <string.h>
#define NRF_LOG_MODULE_NAME usb_cdc
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

namespace usb_cli {

constexpr static uint8_t const CLI_LOG_QUEUE_SIZE = 5;

static Wrapper::AppTimer::Task _task_handle;
static bool _usb_connected = false;

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
            app_usbd_stop();
            break;
        case APP_USBD_EVT_POWER_READY:
			_usb_connected = true;
            app_usbd_start();
            break;
        default:
            break;
    }
}


int write(const void *data, size_t length) {
	if (!_usb_connected) return -1;
	nrf_cli_print_stream(&_usb_cdc_cli, (const char*)data, length);
	return 0;
}

int write(std::string_view str) {
	return write(str.data(), str.size());
}

static void process(uint32_t arg) {
	nrf_cli_process(&_usb_cdc_cli);
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

	_task_handle.create(process, 0);
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
	_task_handle.activate();
	NRF_LOG_INFO("enabled.");
}

}
