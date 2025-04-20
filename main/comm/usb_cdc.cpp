
#include "usb_cdc.h"

#include "nrf_log.h"
#include "nrf_drv_usbd.h"
#include "nrf_delay.h"

#include "app_error.h"
#include "app_util.h"
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"
#include <stdint.h>
#include <string.h>

namespace usb_cdc {

// USB DEFINES START
static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const *p_inst,
									app_usbd_cdc_acm_user_event_t event);

#define CDC_ACM_COMM_INTERFACE 0
#define CDC_ACM_COMM_EPIN NRF_DRV_USBD_EPIN2

#define CDC_ACM_DATA_INTERFACE 1
#define CDC_ACM_DATA_EPIN NRF_DRV_USBD_EPIN1
#define CDC_ACM_DATA_EPOUT NRF_DRV_USBD_EPOUT1

static char _cdc_data_array[1024];

/** @brief CDC_ACM class instance */
APP_USBD_CDC_ACM_GLOBAL_DEF(_app_cdc_acm,
							cdc_acm_user_ev_handler,
							CDC_ACM_COMM_INTERFACE,
							CDC_ACM_DATA_INTERFACE,
							CDC_ACM_COMM_EPIN,
							CDC_ACM_DATA_EPIN,
							CDC_ACM_DATA_EPOUT,
							APP_USBD_CDC_COMM_PROTOCOL_AT_V250);


// USB CODE START
static bool _usb_connected = false;

/** @brief User event handler @ref app_usbd_cdc_acm_user_ev_handler_t */
static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const *p_inst, app_usbd_cdc_acm_user_event_t event) {
	app_usbd_cdc_acm_t const *p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);
	switch (event) {
	case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN: {
		/*Set up the first transfer*/
		app_usbd_cdc_acm_read(&_app_cdc_acm, _cdc_data_array, 1);
		NRF_LOG_INFO("CDC ACM port opened");
		break;
	}

	case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
		NRF_LOG_INFO("CDC ACM port closed");
		break;

	case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
		break;

	case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:	{
		ret_code_t ret;
		/* Get amount of data transferred */
		size_t size = app_usbd_cdc_acm_rx_size(p_cdc_acm);
		/* Fetch data until internal buffer is empty */
		ret = app_usbd_cdc_acm_read(&_app_cdc_acm, _cdc_data_array, size);
		if (ret == NRF_SUCCESS) {
			
		}

		break;
	}
	default:
		break;
	}
}

static void usbd_user_ev_handler(app_usbd_event_type_t event) {
	switch (event) {
	case APP_USBD_EVT_DRV_SUSPEND:
		break;

	case APP_USBD_EVT_DRV_RESUME:
		break;

	case APP_USBD_EVT_STARTED:
		break;

	case APP_USBD_EVT_STOPPED:
		app_usbd_disable();
		break;

	case APP_USBD_EVT_POWER_DETECTED:
		NRF_LOG_INFO("USB power detected");
		if (!nrf_drv_usbd_is_enabled()) {
			app_usbd_enable();
		}
		break;

	case APP_USBD_EVT_POWER_REMOVED: {
		NRF_LOG_INFO("USB power removed");
		_usb_connected = false;
		app_usbd_stop();
	}
	break;

	case APP_USBD_EVT_POWER_READY: {
		NRF_LOG_INFO("USB ready");
		_usb_connected = true;
		app_usbd_start();
	}
	break;

	default:
		break;
	}
}

// USB CODE END

int write(void *data, size_t length) {
	ret_code_t ret = app_usbd_cdc_acm_write(&_app_cdc_acm, data, length);
	return ret;
}

void process() {
	while (app_usbd_event_queue_process());
}

int init(void) {
	ret_code_t ret;
	static const app_usbd_config_t usbd_config = {
        .ev_state_proc = usbd_user_ev_handler
    };

	app_usbd_serial_num_generate();

	ret = app_usbd_init(&usbd_config);
	APP_ERROR_CHECK(ret);

	app_usbd_class_inst_t const *class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&_app_cdc_acm);
	ret = app_usbd_class_append(class_cdc_acm);
	APP_ERROR_CHECK(ret);

	ret = app_usbd_power_events_enable();
	APP_ERROR_CHECK(ret);

	NRF_LOG_INFO("init success.");
	return 0;
}


}
