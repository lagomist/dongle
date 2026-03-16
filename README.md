# nRF52840 USB Dongle

A BLE host / central firmware project for the **nRF52840 USB Dongle**.

This project builds a USB dongle firmware based on **Nordic nRF5 SDK** and **S140 SoftDevice**.  
After flashing, the dongle enumerates as a **USB CDC serial CLI** on the host side and provides a small command-line tool to:

- scan nearby BLE peripherals
- connect to a target device by **name** or **MAC address**
- discover GATT services and characteristics
- select a characteristic by UUID and enable notifications
- send string payloads to the selected characteristic
- print received notification data back to the USB CLI

## Functional highlights

### 1. USB CDC command-line interface
The firmware exposes a USB CDC ACM CLI as the main control interface.

- USB power / connection events are handled by `usb_cli`
- The CLI is started after USB is ready
- Log and command output are both printed through the CLI
- This makes the dongle easy to use from a PC without extra UART wiring

Related files:

- `main/comm/usb_cli.cpp`
- `main/misc/cmds.cpp`

### 2. BLE central / host role
The project acts as a **BLE central**, not a peripheral.

Capabilities implemented in the current code:

- active scanning for nearby BLE devices
- collecting advertising reports with device name, address and RSSI
- connecting to a target device with timeout control
- disconnecting from the current link
- GATT MTU request
- GATT primary service / characteristic / descriptor discovery
- enabling notifications through CCCD
- writing data to the selected characteristic
- receiving notifications and printing payload to the CLI

Related files:

- `main/comm/gatt_client.cpp`
- `main/comm/dongle.cpp`

### 3. Event-driven lightweight framework
The firmware uses a small internal slice-based timer/task framework instead of a full RTOS.

- `Wrapper::Task` is used for periodic tasks
- `Wrapper::Timer` is used for delayed / one-shot event handling
- the main loop continuously calls `Wrapper::sliceProcess()`

This keeps the firmware structure simple and deterministic for small dongle utilities.

Related files:

- `main/driver/timer.cpp`
- `main/main.cpp`

### 4. Running status LED effect
A breathing LED effect is implemented to indicate the firmware is alive.

- PWM-based LED brightness control
- periodic duty-cycle update
- useful as a simple runtime heartbeat indicator

Related files:

- `main/periph/led.cpp`
- `main/driver/pwm.cpp`

### 5. Modern C++ application layer
The application layer is written in **C++17**, while the lower layers rely on the Nordic SDK C libraries.

- application logic in C++
- Nordic SDK / SoftDevice integration in C
- simple wrapper style for timers, BLE and peripherals

## CLI commands

The current CLI command set is implemented in `main/misc/cmds.cpp`.

### Scan devices

```bash
ble scan
ble scan 5
```

- default timeout: 5 seconds
- after scan timeout, the firmware prints the discovered device list

### Connect to a device

By device name:

```bash
ble connect MyDevice
```

By MAC address:

```bash
ble connect AA:BB:CC:DD:EE:FF
```

With custom timeout:

```bash
ble connect MyDevice 10
```

### Discover and select a characteristic

```bash
ble select FFF1
```

Behavior:

- searches the discovered GATT database for the specified characteristic UUID
- requests MTU = 512
- enables notification on the characteristic CCCD

### Send data

```bash
ble send hello
ble send hello nordic world
```

### Disconnect

```bash
ble disconnect
```

## Typical workflow

1. Flash the firmware to the nRF52840 USB Dongle
2. Plug the dongle into a PC
3. Open the USB CDC serial terminal
4. Scan for nearby BLE devices
5. Connect to a target device by name or MAC
6. Inspect printed services / characteristics after service discovery
7. Select a characteristic UUID
8. Send data and observe received notifications in the CLI

Example:

```bash
ble scan 5
ble connect AA:BB:CC:DD:EE:FF
ble select FFF1
ble send test
```

## Project structure

```text
.
├── build/                  # build output
├── components/             # Nordic nRF5 SDK and third-party sources
├── doc/                    # documents / reference files
├── external/               # external project content
├── main/
│   ├── comm/               # BLE client, dongle logic, USB CLI
│   ├── config/             # sdk_config and nrfx config
│   ├── driver/             # timer, pwm, utilities
│   ├── misc/               # CLI command registration
│   ├── periph/             # board-level peripherals such as LED
│   ├── stubs/              # minimal syscall stubs
│   └── main.cpp            # application entry
├── Makefile                # build script
├── partitions.ld           # linker script
├── jlink_flash.jlink       # J-Link flashing script
└── jlink_flash_softdevice.jlink
```

## Build environment

The project is built with:

- **nRF5 SDK** (`components/nRF5_SDK`)
- **S140 SoftDevice**
- **arm-none-eabi-gcc** toolchain
- **GNU Make**
- **C++17**

`config.mk` currently defines the compiler path:

```make
GCC_PATH := /home/lagomist/Applications/Toolchains/arm-none-eabi/bin/
```

If your toolchain is installed elsewhere, update `config.mk` or export the compiler path before building.

## Build

In the project root:

```bash
make
```

Expected outputs:

- `build/nrf52840.elf`
- `build/nrf52840.hex`
- `build/nrf52840.bin`

## Flash

The repository already includes J-Link command files:

- `jlink_flash.jlink`
- `jlink_flash_softdevice.jlink`

A typical flashing flow is:

1. flash SoftDevice if needed
2. flash the application image

Example commands depend on your local J-Link installation, for example:

```bash
JLinkExe -device nRF52840_xxAA -if swd -speed 4000 -CommanderScript jlink_flash_softdevice.jlink
JLinkExe -device nRF52840_xxAA -if swd -speed 4000 -CommanderScript jlink_flash.jlink
```

## Current implementation notes

Current code behavior inferred from the repository:

- scan results are cached in a fixed-size buffer (`30` devices max)
- GATT database storage is currently limited to up to `5` services
- each service stores up to `5` characteristics
- received BLE payload is copied to a local buffer and printed as text
- the current CLI is designed primarily for debugging, bring-up and protocol verification

## Known limitations / considerations

- the README does not yet document exact board pin usage beyond the status LED
- notification payload printing currently assumes text-friendly data
- characteristic selection is UUID-based and assumes the target UUID is unique enough for the use case
- scan / database buffers are statically sized
- build and flash steps may require local environment adjustment

## Main source entry points

- `main/main.cpp` — application startup and main loop
- `main/comm/dongle.cpp` — high-level BLE workflow glue
- `main/comm/gatt_client.cpp` — BLE central and GATT client implementation
- `main/comm/usb_cli.cpp` — USB CDC CLI implementation
- `main/misc/cmds.cpp` — shell command definitions
- `main/periph/led.cpp` — breathing status LED

## Target use cases

This project is suitable for:

- BLE peripheral bring-up and debugging
- quick GATT interaction from a USB dongle
- protocol verification for custom BLE services
- using nRF52840 dongle as a compact BLE host bridge

