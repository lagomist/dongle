#include "nrf_cli.h"
#include "nrf_log.h"
#include "sdk_common.h"

namespace cmds {

static void cmd_python(nrf_cli_t const * p_cli, size_t argc, char **argv) {
    UNUSED_PARAMETER(argc);
    UNUSED_PARAMETER(argv);
    nrf_cli_error(p_cli, "Nice joke ;)");
}

NRF_CLI_CMD_REGISTER(python, NULL, "python", cmd_python);

}
