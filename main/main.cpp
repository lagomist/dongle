#include "timer.h"
#include "led.h"
#include "gatt_client.h"
#include "dongle.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

static void poweron_init(void) {
	ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

	NRF_LOG_INFO("APP starting ...");
}



extern "C" int main(void) {
	poweron_init();
	Wrapper::BLE::Client::init();

	Wrapper::AppTimer::init();
	RuningStatus::init();
	dongle::init();


	NRF_LOG_INFO("enter slice processing ...");
	while(true)	{
		Wrapper::AppTimer::sliceProcess();
	}
}

