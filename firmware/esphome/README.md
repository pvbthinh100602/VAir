# ESPHome Firmware for VAir Phase 1

This directory contains the ESPHome configuration for Phase 1 of the VAir project: ESP32-based IR control of the Daikin FTKB35 air conditioner.

## Prerequisites

### Hardware
- ESP32 development board (ESP32-WROOM-32 recommended)
- IR LED (940nm) with transistor driver circuit (see [docs/wiring.md](../../docs/wiring.md))
- Optional: VS1838B IR receiver for signal capture (see [docs/ir-capture.md](../../docs/ir-capture.md))
- USB cable for programming

### Software

ESPHome installation depends on your Python version:

#### For ESPHome 2026.x (Latest)
Requires **Python 3.12 - 3.14**

```bash
pip install esphome
```

#### For ESPHome 2025.7.x (Stable)
Works with **Python <3.12**

```bash
pip install "esphome==2025.7.5"
```

**Note**: ESPHome 2025.7.x does not support the `non_blocking` option in `remote_transmitter`. This option has been removed from the included configuration for compatibility.

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/pvbthinh100602/VAir.git
cd VAir/firmware/esphome
```

### 2. Create Secrets File

Copy the example secrets file and edit it with your credentials:

```bash
cp secrets.example.yaml secrets.yaml
```

Edit `secrets.yaml` and fill in:
- `wifi_ssid`: Your WiFi network name
- `wifi_password`: Your WiFi password
- `ap_password`: Fallback access point password (choose any password)
- `api_encryption_key`: Will be generated on first run (see below)

### 3. Generate API Encryption Key

On first run, ESPHome will generate an API encryption key for you:

```bash
esphome config daikin-ftkb35.yaml
```

Look for output like:
```
INFO Reading configuration daikin-ftkb35.yaml...
INFO Generating new API key...
  api_encryption_key: "AbCdEf1234567890AbCdEf1234567890="
```

Copy this key into your `secrets.yaml` file under `api_encryption_key`.

### 4. Validate Configuration

Check that the configuration is valid:

```bash
esphome config daikin-ftkb35.yaml
```

Should complete without errors.

### 5. Compile and Flash

#### First Flash (USB Cable Required)

For the first flash, connect the ESP32 via USB:

```bash
esphome run daikin-ftkb35.yaml
```

Select the serial port when prompted (usually `/dev/ttyUSB0` on Linux, `COM3` on Windows, `/dev/cu.usbserial-*` on macOS).

ESPHome will:
1. Compile the firmware
2. Flash it to the ESP32 via USB
3. Show logs from the device

#### Subsequent Updates (Over-The-Air)

After the first flash, you can update wirelessly:

```bash
esphome run daikin-ftkb35.yaml
```

Select the network device (e.g., `daikin-ftkb35.local`) instead of the serial port.

## Configuration Options

### Protocol

Captures from the ARC486A56 remote are standard 3-frame Daikin. `daikin-ftkb35.yaml` only sends those frames through the local `daikin_ftkb` platform. Heat and heat/cool are disabled. Reading and decoding a remote press is `ir-capture.yaml`, a separate image.

Fan level 3 is the climate fan mode Medium. Fan level 4 is Middle. Fan level 5 is High. Fan auto and night/sleep are captured. Temperature steps are 0.5°C. Vertical swing matches the captured swing bit. Dry mode, fan level 1, and actually changing the air conditioner have not been confirmed on this unit.

### UART control

`daikin-ftkb35.yaml` reads commands from the USB serial port. `esphome logs` only prints output, so use a terminal that sends keystrokes. Stop any open log first.

```bash
python3 -m serial.tools.miniterm --echo --eol LF /dev/ttyUSB0 115200
```

Type a command and press Enter. Each command sends one infrared frame on GPIO13.

```text
help
status
off
mode cool 26.5
mode dry 26
mode fan
temp 26.5
fan medium
fan middle
swing on
swing off
test
replay on
replay off
```

`fan medium` is captured fan level 3. `fan middle` is captured fan level 4. `fan high` is captured fan level 5 (byte 8 high nibble 7). `fan auto` is captured as nibble A. `fan quiet` is the captured night/sleep mode, nibble B. `test` turns the IR carrier on for one second. `replay on` and `replay off` send the exact captured remote frames.

## Casper YKR-C/001 (unconfirmed)

`casper-tc09is35.yaml` sends a 13-byte ELECTRA_AC frame on GPIO13 for the YKR-C/001 remote. The nameplate was given as TC-00IS35; Casper publishes that series as TC-09IS35. Timings, bit order, checksum, and the power, mode, temperature, fan, and swing fields follow IRremoteESP8266 `IRElectraAc`. Byte 11 is the YKR key code from published YKR-T/011 captures, not Electra's light-toggle byte. YKR-C/001 has not been captured, and the indoor unit has not been tested. The comparison is in [docs/casper-electra.md](../../docs/casper-electra.md).

```bash
python3 -m serial.tools.miniterm --echo --eol LF /dev/ttyUSB0 115200
```

```text
help
status
off
mode cool 26
mode dry 26
mode fan
temp 26
fan low
fan medium
fan high
fan auto
swing on
swing off
test
```

`test` turns the IR carrier on for one second. The log line `Frame` is the 13 bytes just sent. Compare that line with a capture from `ir-capture.yaml` before trusting it.

## Usage

### Viewing Logs

To view real-time logs from the device:

```bash
esphome logs daikin-ftkb35.yaml
```

Or select the device from the ESPHome Dashboard (if running).

### Integration with Home Assistant

Once flashed and connected to WiFi, the device will automatically appear in Home Assistant (if ESPHome integration is set up).

You can also use the device without Home Assistant by accessing its web interface:
- Open `http://daikin-ftkb35.local` in your browser (mDNS must be supported)
- Or use the IP address shown in logs

## Troubleshooting

### ESP32 Won't Flash

- **Check USB cable**: Ensure it's a data cable, not just charging
- **Install drivers**: CH340/CP2102 drivers may be needed
- **Hold BOOT button**: Some ESP32 boards require holding BOOT while connecting USB
- **Check port permissions**: On Linux, add your user to the `dialout` group

### Device Won't Connect to WiFi

- **Check credentials**: Verify `wifi_ssid` and `wifi_password` in `secrets.yaml`
- **Signal strength**: Move ESP32 closer to router during setup
- **2.4GHz only**: ESP32 does not support 5GHz WiFi
- **Fallback AP**: Look for `Daikin-FTKB35 Fallback` WiFi network if connection fails

### IR Commands Not Working

- **Verify wiring**: Double-check IR LED circuit (see [wiring.md](../../docs/wiring.md))
- **Test LED**: Use phone camera to see if IR LED lights up
- **Check protocol**: Capture real remote signals first (see [ir-capture.md](../../docs/ir-capture.md))
- **Distance/angle**: Ensure clear line of sight to AC receiver
- **Power supply**: Use adequate 5V supply (>500mA)

### Logs Show Errors

- **Strapping pin warning**: GPIO15 warning is normal (safe to use with proper configuration)
- **Brownout detector**: Power supply may be insufficient, use powered USB hub
- **WiFi disconnects**: Check router settings, DHCP reservation recommended

## Python Version Compatibility

| ESPHome Version | Python Requirement | Notes |
|-----------------|-------------------|-------|
| 2025.7.x | <3.12 | Stable, tested with this project |
| 2026.10.x | 3.12 - 3.14 | Latest features, requires newer Python |

If you see errors about Python version, either:
- Install a compatible ESPHome version (see above)
- Use a Python virtual environment with the correct version

## Advanced Configuration

### Custom GPIO Pins

To use different GPIO pins, edit the `pin:` values in `daikin-ftkb35.yaml`:

```yaml
remote_transmitter:
  pin: GPIO13  # IR LED signal

remote_receiver:
  pin:
    number: GPIO35  # VS1838B OUT. Input-only, no pull-up.
```

**Avoid strapping pins during boot**: GPIO0, GPIO2, GPIO5, GPIO12, GPIO15

### Adjusting Tolerance

If signal capture is unreliable, adjust tolerance:

```yaml
remote_receiver:
  tolerance: 55%  # Try 50%, 60%, or 65%
```

### Home Assistant Discovery

By default, ESPHome devices are auto-discovered in Home Assistant. To disable:

```yaml
api:
  encryption:
    key: !secret api_encryption_key
  reboot_timeout: 0s  # Disable automatic reboot on API disconnect
```

## Onboard LED blink (bring-up)

`blink.yaml` prints `blink` on the serial log once per second. On the ESP32-D0WD-V3 board used for bring-up there is no GPIO user LED. The LED by the USB chip is the CP2102 data LED, and it flashes when that log line is sent. The red power LED stays on. No Wi-Fi or `secrets.yaml` is required.

```bash
source .venv/bin/activate
esphome run firmware/esphome/blink.yaml --device /dev/ttyUSB0 --upload_speed 115200
```

## IR capture

`ir-capture.yaml` listens on GPIO35 and prints each remote burst on the USB log. Wire the VS1838B VCC to 3.3V, GND to GND, and OUT to GPIO35. GPIO35 is input-only, so the config does not enable a pull-up. No Wi-Fi or `secrets.yaml` is required.

```bash
esphome run firmware/esphome/ir-capture.yaml --device /dev/ttyUSB0 --upload_speed 115200
```

Press one button on the Daikin remote. A DAIKIN216 capture is two frames; a standard DAIKIN capture is three. See [docs/ir-capture.md](../../docs/ir-capture.md).

## Files in This Directory

- **daikin-ftkb35.yaml**: Main ESPHome configuration for the captured Daikin protocol
- **components/daikin_ftkb/**: IR sender built from the ARC486A56 captures
- **casper-tc09is35.yaml**: Unconfirmed sender for a Casper unit with remote YKR-C/001
- **components/casper_ykr/**: 13-byte ELECTRA_AC frame builder for that remote, with the YKR key byte kept
- **ir-capture.yaml**: IR receiver logger for remote capture
- **blink.yaml**: Onboard LED blink for board bring-up
- **secrets.example.yaml**: Template for credentials (copy to secrets.yaml)
- **secrets.yaml**: Your actual credentials (gitignored, never commit this!)
- **README.md**: This file

## Next Steps

1. ✅ Flash firmware to ESP32
2. ✅ Capture IR signals from the original remote ([guide](../../docs/ir-capture.md))
3. ✅ Protocol is 3-frame Daikin, with frame bytes recorded in `captures/arc486a56/`
4. 📋 Test AC control (power on/off, temperature, mode) with the IR LED on GPIO13
5. 📋 Confirm dry mode and fan level 1

## Resources

- [ESPHome Documentation](https://esphome.io/)
- [ESPHome Climate Components](https://esphome.io/components/climate/)
- [ESPHome Remote Transmitter](https://esphome.io/components/remote_transmitter.html)
- [ESPHome Remote Receiver](https://esphome.io/components/remote_receiver.html)
- [VAir Project Roadmap](../../docs/roadmap.md)

---

**Need Help?** Open an issue on the [VAir GitHub repository](https://github.com/pvbthinh100602/VAir/issues).

**Last Updated**: November 3, 2026
