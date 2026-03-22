#include "nrf_cli.h"
#include "nrf_log.h"
#include "sdk_common.h"
#include "usb_cli.h"
#include "dongle.h"
#include "utils.h"
#include <cstring>

namespace cmds {

#define CMD_ASSERT(condition) do { if (!(condition)) { return nrf_cli_fprintf(p_cli, NRF_CLI_ERROR, "assert failed, condition '" #condition "'\n"); } } while (0)

static void cmd_nordic(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);

    if (nrf_cli_help_requested(p_cli)) {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }

    nrf_cli_fprintf(p_cli, NRF_CLI_OPTION,
                    "\n"
                    "            .co:.                   'xo,          \n"
                    "         .,collllc,.             'ckOOo::,..      \n"
                    "      .:ooooollllllll:'.     .;dOOOOOOo:::;;;'.   \n"
                    "   'okxddoooollllllllllll;'ckOOOOOOOOOo:::;;;,,,' \n"
                    "   OOOkxdoooolllllllllllllllldxOOOOOOOo:::;;;,,,'.\n"
                    "   OOOOOOkdoolllllllllllllllllllldxOOOo:::;;;,,,'.\n"
                    "   OOOOOOOOOkxollllllllllllllllllcccldl:::;;;,,,'.\n"
                    "   OOOOOOOOOOOOOxdollllllllllllllccccc::::;;;,,,'.\n"
                    "   OOOOOOOOOOOOOOOOkxdlllllllllllccccc::::;;;,,,'.\n"
                    "   kOOOOOOOOOOOOOOOOOOOkdolllllllccccc::::;;;,,,'.\n"
                    "   kOOOOOOOOOOOOOOOOOOOOOOOxdllllccccc::::;;;,,,'.\n"
                    "   kOOOOOOOOOOOOOOOOOOOOOOOOOOkxolcccc::::;;;,,,'.\n"
                    "   kOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOkdlc::::;;;,,,'.\n"
                    "   xOOOOOOOOOOOxdkOOOOOOOOOOOOOOOOOOOOxoc:;;;,,,'.\n"
                    "   xOOOOOOOOOOOdc::ldkOOOOOOOOOOOOOOOOOOOkdc;,,,''\n"
                    "   xOOOOOOOOOOOdc::;;,;cdkOOOOOOOOOOOOOOOOOOOxl;''\n"
                    "   .lkOOOOOOOOOdc::;;,,''..;oOOOOOOOOOOOOOOOOOOOx'\n"
                    "      .;oOOOOOOdc::;;,.       .:xOOOOOOOOOOOOd;.  \n"
                    "          .:xOOdc:,.              'ckOOOOkl'      \n"
                    "             .od'                    'xk,         \n"
                    "\n");

    nrf_cli_print(p_cli, "                Nordic Semiconductor              \n");
}

static void cmd_ble(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    if ((argc == 1) || nrf_cli_help_requested(p_cli)) {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }
    nrf_cli_fprintf(p_cli, NRF_CLI_WARNING, "%s unknown option\n", argv[1]);
}

static void cmd_ble_scan(nrf_cli_t const * p_cli, size_t argc, char **argv) {
	uint16_t timeout = 15;
	dongle::ScanMode mode = dongle::ScanMode::PHY_1M;
	if (argc >= 2) {
		if (strcmp(argv[1], "coded") == 0) {
			mode = dongle::ScanMode::PHY_CODED;
		} else if (strcmp(argv[1], "dual") == 0) {
			mode = dongle::ScanMode::PHY_DUAL;
		} else if (strcmp(argv[1], "1m") == 0) {
			mode = dongle::ScanMode::PHY_1M;
		} else {
			timeout = atoi(argv[1]);
		}
	}
	if (argc >= 3) {
		if (strcmp(argv[2], "coded") == 0) {
			mode = dongle::ScanMode::PHY_CODED;
		} else if (strcmp(argv[2], "dual") == 0) {
			mode = dongle::ScanMode::PHY_DUAL;
		} else if (strcmp(argv[2], "1m") != 0) {
			nrf_cli_fprintf(p_cli, NRF_CLI_WARNING, "scan mode must be 1m/coded/dual\n");
			return;
		}
	}
	dongle::ble_scan(timeout, mode);
	nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "start scan: %d sec, mode: %s\n", timeout, Wrapper::BLE::Client::scan_mode_name(mode));
}

static void cmd_ble_connect(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    if ((argc < 2) || nrf_cli_help_requested(p_cli)) {
        nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "Usage: ble connect <name|AA:BB:CC:DD:EE:FF> [timeout]\n");
        nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "Connect to device by name or MAC.\n");
        return;
    }
    std::string name(argv[1]);
    uint16_t timeout = (argc == 3) ? atoi(argv[2]) : 5;
    int res = dongle::ble_connect(name, timeout);
    if (res == 0) {
        nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "%s connecting ...\n", name.data());
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_WARNING, "[%s] %s connection failed.\n", NRF_LOG_ERROR_STRING_GET(res), name.data());
    }
}

static void cmd_ble_select(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    if ((argc != 2) || nrf_cli_help_requested(p_cli)) {
        nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "Usage: ble select <uuid_hex>\n");
        nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "Select characteristic by 16-bit UUID.\n");
        return;
    }
    uint16_t char_uuid = strtoul(argv[1], nullptr, 16);
    if (dongle::ble_select(char_uuid) < 0) {
        nrf_cli_fprintf(p_cli, NRF_CLI_WARNING, "UUID not found.\n");
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "Enable notify.\n");
    }
}

static void cmd_ble_send(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    if ((argc < 2) || nrf_cli_help_requested(p_cli)) {
        nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "Usage: ble send <payload>\n");
        nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "Send data string to connected device.\n");
        return;
    }
    Utils::OBuf content = argv[1];
    for (size_t i = 1; i < argc - 1; i++) {
        content += ' ';
        content += argv[1 + i];
    }
    if (dongle::ble_send(content) < 0) {
        nrf_cli_fprintf(p_cli, NRF_CLI_WARNING, "Send failed.\n");
    }
}

static void cmd_ble_disconnect(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    dongle::ble_disconnect();
    nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "disconnected.\n");
}

static void cmd_usb(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    if ((argc == 1) || nrf_cli_help_requested(p_cli)) {
        nrf_cli_help_print(p_cli, NULL, 0);
        return;
    }
    nrf_cli_fprintf(p_cli, NRF_CLI_WARNING, "%s unknown option\n", argv[1]);
}

static void cmd_usb_log(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    if ((argc == 1) || nrf_cli_help_requested(p_cli)) {
        nrf_cli_fprintf(p_cli, NRF_CLI_WARNING, "usb log command must be on/off/status\n");
        return;
    }

    if (strcmp(argv[1], "on") == 0) {
        usb_cli::set_log_output(true);
        nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "usb log output enabled\n");
        return;
    }

    if (strcmp(argv[1], "off") == 0) {
        usb_cli::set_log_output(false);
        nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "usb log output disabled\n");
        return;
    }

    if (strcmp(argv[1], "status") == 0) {
        nrf_cli_fprintf(p_cli,
                        NRF_CLI_NORMAL,
                        "usb log output: %s\n",
                        usb_cli::log_output_enabled() ? "enabled" : "disabled");
        return;
    }

    nrf_cli_fprintf(p_cli, NRF_CLI_WARNING, "usb log command must be on/off/status\n");
}

NRF_CLI_CMD_REGISTER(nordic, nullptr, "Print Nordic Semiconductor logo.", cmd_nordic);
NRF_CLI_CPP_CREATE_STATIC_SUBCMD_SET(m_sub_ble,
    NRF_CLI_CMD(scan, nullptr, "Scan for BLE devices. Usage: ble scan [timeout] [1m|coded|dual]", cmd_ble_scan),
    NRF_CLI_CMD(connect, nullptr, "Connect to a BLE device by name or MAC. Usage: ble connect <name|AA:BB:CC:DD:EE:FF> [timeout]", cmd_ble_connect),
    NRF_CLI_CMD(select, nullptr, "Select service by 16-bit UUID. Usage: ble select <uuid_hex>", cmd_ble_select),
    NRF_CLI_CMD(send, nullptr, "Send string data to BLE. Usage: ble send <payload>", cmd_ble_send),
    NRF_CLI_CMD(disconnect, nullptr, "Disconnect current BLE device. Usage: ble disconnect", cmd_ble_disconnect),
    NRF_CLI_SUBCMD_SET_END
);
NRF_CLI_CMD_REGISTER(ble, &m_sub_ble, "BLE host interface. Use 'ble scan', 'ble connect', ...", cmd_ble);

NRF_CLI_CPP_CREATE_STATIC_SUBCMD_SET(m_sub_usb,
    NRF_CLI_CMD(log, nullptr, "Control USB log output.", cmd_usb_log),
    NRF_CLI_SUBCMD_SET_END
);
NRF_CLI_CMD_REGISTER(usb, &m_sub_usb, "USB interface. Use 'usb log on|off|status'", cmd_usb);

}
