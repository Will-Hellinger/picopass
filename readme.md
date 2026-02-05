# PicoPass

A Bluetooth Low Energy (BLE) proximity discovery system for Raspberry Pi Pico W devices.

## Features

- **Automatic Discovery**: Devices find each other without user intervention
- **Low Power**: Uses BLE for efficient battery operation
- **Visual Feedback**: LED flashes when another PicoPass device is detected
- **Burst Mode**: Rapid switching between advertising and scanning for optimal discovery

## How It Works

1. Each Picopass device flickers back and forth between scanning and advertising rapidly.

2. The intervals are random and they go back and forth so fast they they will eventually overlap.

3. Devices only recognize others with the matching "PicoPass" identifier

## Future Plans

- Picopass server and service
    - Picopass server will provide frontend to see who you've connected with and verify the connection was legit
    - Service runs on machines with bluetooth support which forwards info to picopass server

- Picopass Client/Config Manager 
    - Configure your picopass device without needing to rebuild the firmware with flags


## Hardware Requirements

- Raspberry Pi Pico W (with CYW43439 wireless chip)
- USB cable for programming and power
- Optional: External power source for portable operation

## Building

### Prerequisites
- [Pico SDK 2.2.0](https://github.com/raspberrypi/pico-sdk/releases/tag/2.2.0)
- ARM GNU Toolchain (`gcc-arm-none-eabi`)
- CMake 3.13+
- Make 4.4.1+

### Build Steps
```bash
git clone https://github.com/Will-Hellinger/picopass
cd picopass

cd picopass-pico
make
```

## Configuration

Key parameters in `main.c`:

```c
#define RSSI_THRESHOLD   -100    // Minimum signal strength for detection
static const uint8_t PICO_SECRET[8] = { 'P','I','C','O','P','A','S','S' };
```

- **RSSI_THRESHOLD**: Adjust detection range (This is subjective, it's based on resistance)
- **PICO_SECRET**: Change to create separate device groups

## Usage

1. **Flash** the firmware to multiple Pico W devices
2. **Power on** the devices
3. **Observe** LED flashes when devices detect each other
4. **Monitor** serial output (115200 baud) for detailed logs

## Serial Output

Connect to the device via USB serial to see detailed operation:

```
=== PICOPASS STARTUP ===
Device MAC: 00:00:00:00:00:00
Random seed: 1072201339
RSSI threshold: -70 dBm
Starting PicoPASS BLE burst mode...
Your advertising data (16 bytes): 02 01 06 0C FF FF FF 50 49 43 4F 50 41 53 53 01 
Expected secret: 50 49 43 4F 50 41 53 53 
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
