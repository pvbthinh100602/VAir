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

### Enabling the IR Receiver (for Signal Capture)

To capture signals from your original remote (recommended before testing AC control):

1. Wire the VS1838B receiver module as described in [docs/wiring.md](../../docs/wiring.md)
2. Edit `daikin-ftkb35.yaml` and uncomment the `remote_receiver:` section
3. Reflash the firmware
4. Follow the capture guide in [docs/ir-capture.md](../../docs/ir-capture.md)

### Disabling Heat Mode

If your remote only supports COOL/DRY/FAN modes (standard for Vietnamese market):

Uncomment this line in the `climate:` section:

```yaml
supports_heat: false
```

### Trying Alternative Protocols

If the default `platform: daikin` doesn't work, try:

**Option 1: Daikin ARC (Japanese remotes)**
```yaml
climate:
  - platform: daikin_arc
    name: "Daikin FTKB35 Air Conditioner"
```

**Option 2: HeatpumpIR**
```yaml
climate:
  - platform: heatpumpir
    protocol: daikin  # or daikin_arc417, daikin_arc480
    horizontal_default: auto
    vertical_default: auto
    name: "Daikin FTKB35 Air Conditioner"
```

See the [ESPHome Climate documentation](https://esphome.io/components/climate/) for more options.

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
  pin: GPIO4  # Change to your preferred GPIO

remote_receiver:
  pin:
    number: GPIO15  # Change to your preferred GPIO
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

## Files in This Directory

- **daikin-ftkb35.yaml**: Main ESPHome configuration
- **secrets.example.yaml**: Template for credentials (copy to secrets.yaml)
- **secrets.yaml**: Your actual credentials (gitignored, never commit this!)
- **README.md**: This file

## Next Steps

1. ✅ Flash firmware to ESP32
2. 📋 Capture IR signals from original remote ([guide](../../docs/ir-capture.md))
3. 📋 Verify protocol type (DAIKIN vs DAIKIN216)
4. 📋 Test AC control (power on/off, temperature, mode)
5. 📋 Document findings and update configuration if needed

## Resources

- [ESPHome Documentation](https://esphome.io/)
- [ESPHome Climate Components](https://esphome.io/components/climate/)
- [ESPHome Remote Transmitter](https://esphome.io/components/remote_transmitter.html)
- [ESPHome Remote Receiver](https://esphome.io/components/remote_receiver.html)
- [VAir Project Roadmap](../../docs/roadmap.md)

---

**Need Help?** Open an issue on the [VAir GitHub repository](https://github.com/pvbthinh100602/VAir/issues).

**Last Updated**: October 26, 2026
