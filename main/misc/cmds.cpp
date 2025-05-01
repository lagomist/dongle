#include "nrf_cli.h"
#include "nrf_log.h"
#include "sdk_common.h"
#include "usb_cli.h"
#include "dongle.h"

namespace cmds {

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
}

static void cmd_ble_scan(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    uint16_t timeout = (argc == 2) ? atoi(argv[1]) : 10;
    dongle::ble_scan(timeout);
    nrf_cli_fprintf(p_cli, NRF_CLI_NORMAL, "start scan %d sec\n", timeout);
}

NRF_CLI_CMD_REGISTER(nordic, nullptr, "Print Nordic Semiconductor logo.", cmd_nordic);
NRF_CLI_CPP_CREATE_STATIC_SUBCMD_SET(m_sub_ble,
    NRF_CLI_CMD(scan,   nullptr, "Print all entered parameters.", cmd_ble_scan),
    NRF_CLI_SUBCMD_SET_END
);
NRF_CLI_CMD_REGISTER(ble, &m_sub_ble, "ble host interface", cmd_ble);

}
