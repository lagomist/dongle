# AGENTS.md

## Scope
- This file applies to the `dongle/` firmware project.
- The project targets the nRF52840 USB Dongle and uses Nordic nRF5 SDK plus the S140 SoftDevice.
- Application code is mostly C++17 in `main/`; Nordic SDK, startup, and many low-level modules remain C.
- Build orchestration is GNU Make based; there is no first-party CMake, Bazel, npm, or Python packaging workflow.

## Rule Files
- No Cursor rules were found in `.cursor/rules/`.
- No `.cursorrules` file was found.
- No Copilot instructions file was found at `.github/copilot-instructions.md`.
- If any of those files appear later, treat them as higher-priority repository guidance and update this file.

## Repository Map
- `main/main.cpp` wires startup, logging, timers, USB CLI, BLE client init, and the slice loop.
- `main/comm/` contains `gatt_client`, `dongle`, and `usb_cli`.
- `main/driver/` contains timer, PWM, formatting, and utility wrappers.
- `main/periph/` contains board-level behavior such as the breathing status LED.
- `main/config/` contains `sdk_config.h` and `nrfx_config.h`.
- `components/` contains the vendored nRF5 SDK; treat it as upstream unless a task explicitly requires a patch.
- `external/` contains third-party sources; avoid style-only edits there.
- `build/` contains generated objects, dependency files, and final firmware artifacts.

## Toolchain
- Compiler: `arm-none-eabi-gcc` and `arm-none-eabi-g++`.
- Default toolchain path comes from `config.mk` via `GCC_PATH`.
- Override toolchain location with `make GCC_PATH=/path/to/bin` when needed.
- Target CPU flags are Cortex-M4 hard-float.
- Link uses `nano.specs` and produces ELF, HEX, and BIN images.
- Flashing is J-Link based.

## Build Commands
- Full build: `make`
- Verbose build: `make VERBOSE=1`
- Clean artifacts: `make clean`
- Show supported targets: `make help`
- Flash application image: `make flash`
- Flash SoftDevice image: `make flash_softdevice`
- Open the Nordic config wizard for `sdk_config.h`: `make sdk_config`

## Build Outputs
- `build/nrf52840.elf`
- `build/nrf52840.hex`
- `build/nrf52840.bin`
- `build/*.o` and `build/*.d` for object and dependency files

## Lint And Format
- No `lint`, `format`, `check`, `clang-tidy`, or `cppcheck` target was found.
- No `.clang-format`, `.clang-tidy`, or `.editorconfig` file was found.
- There is no enforced autoformatter; agents must match nearby style manually.
- Avoid formatting-only churn, especially in mixed tab/space files.

## Tests
- No automated unit-test or integration-test framework was found.
- No `make test` or `make check` target exists.
- There is no true single-test runner in this repository.
- Verification is done with focused compilation plus on-device manual testing.

## Single-Test Equivalent
- Build one translation unit by targeting its object file directly.
- Examples: `make build/gatt_client.o`, `make build/dongle.o`, `make build/usb_cli.o`, `make build/main.o`
- Force a rebuild of one object with `make -B build/gatt_client.o`.
- Use this for localized compile verification, then prefer a full `make` before handoff.

## Manual Smoke Test
- If the SoftDevice is not present, run `make flash_softdevice` first.
- Flash the app with `make flash`.
- Connect to the USB CDC CLI and run a representative BLE flow.
- Typical commands: `ble scan 5`, `ble connect AA:BB:CC:DD:EE:FF`, `ble select FFF1`, `ble send test`
- Confirm scan output, connect/disconnect events, service discovery, notify enable, and data send behavior.

## Editing Boundaries
- Prefer edits under `main/`.
- Avoid touching `components/` or `external/` unless the task clearly requires it.
- Do not refactor vendored SDK code for style consistency.
- Do not include generated `build/` artifacts in changes.
- Treat J-Link scripts as generated support files unless the task is specifically about flashing.

## Language And ABI Rules
- C++ files compile with `-std=c++17`.
- Preserve the existing C/C++ split instead of converting files unnecessarily.
- Keep C-compatible callback signatures where Nordic APIs require them.
- Use `extern "C"` only where required by runtime or SDK boundaries.

## Includes And Imports
- Local project headers usually come first.
- Nordic SDK and platform headers usually come next.
- C and C++ standard headers usually come last.
- Use quoted includes for local headers such as `"gatt_client.h"`.
- Use angle brackets for standard library headers such as `<cstdint>` and `<string_view>`.
- Do not reorder includes just for preference if the file already has a stable pattern.
- Add only headers that are actually needed.

## Formatting
- Match the indentation of the file you are editing; this repo mixes tabs and spaces.
- Do not reindent entire files.
- Keep opening braces on the same line; that is the dominant style.
- Preserve existing spacing around casts, pointer stars, and initializer layout when editing nearby code.
- Keep blank-line rhythm modest and local; do not normalize a whole file.
- Comments are sparse; add them only when logic is not obvious.

## Naming
- Namespaces are short and lower-case or project-specific, such as `dongle`, `usb_cli`, and `Wrapper`.
- Types use PascalCase, for example `Task`, `Timer`, `AdvReport`, `GattDatabase`.
- Enums often use SDK-style event names such as `CONNECTED_EVT` and `SCAN_TIMEOUT_EVT`.
- File-scope statics commonly use a leading underscore, such as `_profile`, `_scan_param`, `_task_handle`.
- Constants often use `constexpr static` with upper-case identifiers.
- Handlers commonly end in `_handler` or `_callback`.

## Types
- Prefer fixed-width integer types such as `uint8_t`, `uint16_t`, and `uint32_t`.
- `std::string_view` is the preferred non-owning string input type in higher-level C++ code.
- Use raw pointers where Nordic APIs expect them.
- Prefer plain structs for wire-level or callback data.
- Avoid introducing exception-driven or allocation-heavy abstractions into hot embedded paths.

## Error Handling
- `ret_code_t` and Nordic error codes are the dominant error model.
- Use `APP_ERROR_CHECK(...)` for failures that should remain fatal during init or critical runtime setup.
- For recoverable operations, return negative values or Nordic error codes according to the local file convention.
- Validate connection handles, CCCD handles, buffer sizes, and UUID lookup results before acting.
- Keep warning logs for recoverable failures when the surrounding code already logs them.
- Do not silently ignore SDK failures.

## Logging And User Output
- Logging uses `NRF_LOG_*` macros and file-local module registration.
- Preserve the pattern `#define NRF_LOG_MODULE_NAME ...` followed by `NRF_LOG_MODULE_REGISTER()`.
- CLI-facing text goes through `usb_cli::write(...)`, `usb_cli::option_printf(...)`, or `nrf_cli_fprintf(...)`.
- Keep operator-facing messages short and concrete.
- Avoid chatty logs in fast paths unless the file already logs at that level.

## Embedded Design Patterns
- The firmware is event-driven and timer-driven, not RTOS based.
- Main-loop work is dispatched through `Wrapper::sliceProcess()`.
- Prefer explicit state transitions over implicit side effects.
- Respect fixed-size arrays and existing capacity limits in scan and GATT database storage.
- Avoid blocking calls except where the current startup or USB flow already uses them.
- Be conservative with heap allocation; stack or static storage is preferred when practical.

## Header And API Hygiene
- When adding public functionality, update the matching header in `main/*/include/`.
- Keep public interfaces small and firmware-oriented.
- Prefer extending existing modules like `dongle`, `usb_cli`, `Wrapper::BLE::Client`, or `Wrapper::Task` instead of bypassing them.
- Keep names and signatures compatible with surrounding code, even if they are not perfectly polished.

## Validation Expectations
- After code changes, run at least a focused object build for touched translation units when practical.
- Prefer a full `make` before considering the change complete.
- If behavior changed, describe the manual CLI sequence used or recommended.
- If hardware validation was not possible, say that explicitly.

## Avoid
- Do not assume a host-side test harness exists.
- Do not add a new formatting regime to the repository.
- Do not rewrite vendored SDK or third-party files without a task-specific reason.
- Do not introduce threads, exceptions, or host-only libraries into core firmware paths.
- Do not remove Nordic guardrails such as `APP_ERROR_CHECK(...)` without a strong embedded-specific reason.
