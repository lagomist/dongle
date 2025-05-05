#include "nrf_cli.h"
#include "nrf_log.h"
#include "sdk_common.h"
#include "usb_cli.h"
#include "dongle.h"

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
    uint16_t timeout = (argc == 2) ? atoi(argv[1]) : 10;
    dongle::ble_scan(timeout);
    nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "start scan %d sec\n", timeout);
}

static void cmd_ble_connect(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    CMD_ASSERT(argc >= 2);
    std::string_view name({argv[1], strlen(argv[1])});
    uint16_t timeout = (argc == 3) ? atoi(argv[2]) : 5;
    if (dongle::ble_connect(name, timeout) == 0) {
        nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "Connecting %s...\n", argv[1]);
    } else {
        nrf_cli_fprintf(p_cli, NRF_CLI_WARNING, "%s not found\n", argv[1]);
    }
}

static void cmd_ble_disconnect(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    dongle::ble_disconnect();
    nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "disconnected.\n");
}

NRF_CLI_CMD_REGISTER(nordic, nullptr, "Print Nordic Semiconductor logo.", cmd_nordic);
NRF_CLI_CPP_CREATE_STATIC_SUBCMD_SET(m_sub_ble,
    NRF_CLI_CMD(scan,   nullptr, "Scan ble device.", cmd_ble_scan),
    NRF_CLI_CMD(connect,   nullptr, "Connect ble device.", cmd_ble_connect),
    NRF_CLI_CMD(disconnect,   nullptr, "Disconnect ble device.", cmd_ble_disconnect),
    NRF_CLI_SUBCMD_SET_END
);
NRF_CLI_CMD_REGISTER(ble, &m_sub_ble, "ble host interface", cmd_ble);

}
