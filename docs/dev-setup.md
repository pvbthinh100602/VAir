# VAir Development Environment Setup

This guide explains how to set up a complete development environment for working on the VAir project, including ESPHome firmware development, validation scripts, and testing workflows.

## Quick Start (3 Commands)

```bash
# 1. Clone the repository
git clone https://github.com/pvbthinh100602/VAir.git
cd VAir

# 2. Run the automated setup script
bash scripts/setup-dev.sh

# 3. Activate the virtual environment
source .venv/bin/activate
```

That's it! Your development environment is ready.

## What the Setup Script Does

The `scripts/setup-dev.sh` script is **idempotent** (safe to run multiple times) and performs:

1. **Python Version Detection**: Searches for Python 3.12-3.14 on your system
   - Tries: `python3.14`, `python3.13`, `python3.12`, `python3`, `python`
   - Fails with clear installation instructions if no compatible version found
   - No sudo required

2. **Virtual Environment**: Creates `.venv/` at the repository root
   - Reuses existing `.venv` if present
   - Isolates dependencies from system Python

3. **Dependency Installation**: Installs from `requirements-dev.txt`
   - ESPHome 2026.10.0 (pinned version)
   - yamllint 1.35.1 for YAML validation
   - All dependencies with pinned versions for reproducibility

4. **Secrets File**: Creates `firmware/esphome/secrets.yaml` if it doesn't exist
   - Generates a secure API encryption key automatically
   - Uses dummy WiFi credentials (edit before flashing!)
   - Never overwrites existing secrets file

5. **Next Steps**: Prints helpful commands to run next

## Python Version Compatibility Matrix

| ESPHome Version | Python Requirement | Notes |
|-----------------|-------------------|-------|
| **2026.10.0** (current) | **3.12 - 3.14** | Latest features, includes `non_blocking` default change warning |
| 2025.7.5 | <3.12 | Stable, does NOT support `non_blocking` option in `remote_transmitter` |

### Installing Python 3.12-3.14

#### Ubuntu/Debian
```bash
# Option 1: System package (if available)
sudo apt update
sudo apt install python3.13 python3.13-venv

# Option 2: Deadsnakes PPA (for older Ubuntu versions)
sudo apt install software-properties-common
sudo add-apt-repository ppa:deadsnakes/ppa
sudo apt update
sudo apt install python3.12 python3.12-venv
```

#### macOS (Homebrew)
```bash
brew install python@3.13
# or
brew install python@3.12
```

#### Fedora/RHEL
```bash
sudo dnf install python3.13
```

### Using ESPHome 2025.7.5 with Older Python

If you only have Python 3.11 or earlier available:

1. Edit `requirements-dev.txt`:
   ```txt
   esphome==2025.7.5
   yamllint==1.35.1
   ```

2. Run setup again:
   ```bash
   bash scripts/setup-dev.sh
   ```

**Note**: ESPHome 2025.7.5 does NOT support the `non_blocking` option, which is why our YAML configuration omits it (for compatibility).

## ESPHome Configuration Workflow

### 1. Edit WiFi Credentials

Before flashing to hardware, edit the secrets file:

```bash
nano firmware/esphome/secrets.yaml
```

Update with your actual WiFi network:
```yaml
wifi_ssid: "YourActualSSID"
wifi_password: "YourActualPassword"
```

### 2. Validate Configuration

Check that your YAML is valid:

```bash
esphome config firmware/esphome/daikin-ftkb35.yaml
```

Expected output:
```
INFO ESPHome 2026.10.0
INFO Reading configuration daikin-ftkb35.yaml...
INFO Configuration is valid!
```

### 3. Compile Firmware

Compile without flashing (generates binary):

```bash
esphome compile firmware/esphome/daikin-ftkb35.yaml
```

Compiled firmware will be in `.esphome/build/daikin-ftkb35/.pioenvs/daikin-ftkb35/firmware.bin`

### 4. Flash to ESP32

#### First Flash (USB Cable Required)

Connect ESP32 via USB and run:

```bash
esphome run firmware/esphome/daikin-ftkb35.yaml
```

Select the serial port when prompted:
- **Linux**: Usually `/dev/ttyUSB0` or `/dev/ttyACM0`
- **macOS**: Usually `/dev/cu.usbserial-*` or `/dev/cu.SLAB_USBtoUART`
- **Windows**: Usually `COM3` or similar

#### USB Permissions Fix (Linux)

If you get a permission error:

```bash
# Add your user to the dialout group
sudo usermod -a -G dialout $USER

# Log out and log back in for changes to take effect
# Or use: newgrp dialout
```

#### Over-The-Air (OTA) Updates

After the first flash, you can update wirelessly:

```bash
esphome run firmware/esphome/daikin-ftkb35.yaml
```

Select the network device (e.g., `daikin-ftkb35.local`) instead of the serial port.

**Requirements for OTA**:
- ESP32 must be connected to WiFi
- Your computer must be on the same network
- mDNS must be working (usually automatic on Linux/macOS)

### 5. View Logs

Monitor real-time logs from the device:

```bash
esphome logs firmware/esphome/daikin-ftkb35.yaml
```

Or use the serial port directly:
```bash
esphome logs firmware/esphome/daikin-ftkb35.yaml --device /dev/ttyUSB0
```

## IR Signal Capture Workflow

To capture signals from the Daikin remote (essential for protocol verification):

### 1. Enable IR Receiver in Configuration

Edit `firmware/esphome/daikin-ftkb35.yaml` and uncomment the `remote_receiver:` block:

```yaml
remote_receiver:
  id: rcvr
  pin:
    number: GPIO15
    inverted: true
    mode:
      input: true
      pullup: true
  dump:
    - raw
    - aeha
  tolerance: 55%
```

### 2. Wire IR Receiver Module

Connect VS1838B receiver:
- VCC → ESP32 3.3V
- GND → ESP32 GND
- OUT → ESP32 GPIO15

See [docs/wiring.md](wiring.md) for detailed circuit.

### 3. Reflash Firmware

```bash
esphome run firmware/esphome/daikin-ftkb35.yaml
```

### 4. Capture Signals

Start logging and press buttons on the remote:

```bash
esphome logs firmware/esphome/daikin-ftkb35.yaml
```

Point the Daikin remote at the receiver and press buttons. Look for output like:

```
[D][remote.raw:041]: Received Raw:
  HEADER: 3440us, -1750us
  DATA: ...
```

### 5. Analyze Protocol

See [docs/ir-capture.md](ir-capture.md) for detailed analysis guide:
- DAIKIN vs DAIKIN216 identification (2 vs 3 frames)
- Header timing verification (~3440/1750µs)
- Frame spacing (~29ms)

## Running Validation Checks

The `scripts/check.sh` script runs all the same checks that CI runs:

```bash
bash scripts/check.sh
```

### What It Checks

1. **ESPHome Configuration**: Validates YAML syntax and configuration
2. **YAML Linting**: Checks formatting and style (with `.yamllint` config)
3. **Language Check**: Ensures no Vietnamese characters in code/docs (except allowed files like `docs/hardware.md` for shop names)

### Before Every Commit

Always run the validation script before committing:

```bash
# Make changes
nano firmware/esphome/daikin-ftkb35.yaml

# Validate
bash scripts/check.sh

# If checks pass, commit
git add .
git commit -m "Update ESPHome configuration"
```

## Known Gotchas and Workarounds

### 1. `non_blocking` Option Warning

**Symptom**: ESPHome 2026.10.0 shows a warning about `non_blocking` not being set.

**Explanation**: 
- ESPHome 2026.10.0 changed the default behavior for `remote_transmitter`
- Previously defaulted to blocking mode, now defaults to non-blocking
- ESPHome 2025.7.x does NOT support this option at all

**Workaround**: Our configuration omits `non_blocking` for compatibility with 2025.7.x. The warning is harmless in 2026.x (non-blocking is the correct mode).

### 2. Daikin Platform is Unconfirmed for ARC486A56

**Issue**: The configuration uses `platform: daikin` (3-frame protocol), but the ARC486A56 remote's protocol is **not confirmed**.

**Suspected Protocol**: DAIKIN216 (2-frame), based on similarity to ARC484A4.

**What This Means**:
- The ESP32 will transmit 3-frame DAIKIN commands
- If the AC actually uses DAIKIN216 (2-frame), it won't respond
- **You must capture the real remote signal first** to confirm

**Solutions if It Doesn't Work**:
1. Try `platform: daikin_arc` (Japanese ARC43xxx remotes)
2. Try `platform: heatpumpir` with `protocol: daikin`
3. Use raw IR code replay (capture and store exact codes)
4. Wait for Phase 2 (IRremoteESP8266 with DAIKIN216 support)

### 3. ESPHome Has No Native DAIKIN216 Support

**Limitation**: ESPHome's built-in climate platforms only support:
- `daikin` - 3-frame protocol (ARC433/ARC470)
- `daikin_arc` - Japanese ARC43xxx protocol
- `heatpumpir` - Various protocols via HeatpumpIR library

**DAIKIN216 Not Included**: The 2-frame DAIKIN216 protocol is not natively supported.

**Phase 2 Solution**: Use ESP-IDF with IRremoteESP8266 library directly (has `IRDaikin216` class).

### 4. GPIO15 Strapping Pin Warning

**Warning**: ESPHome shows a warning about GPIO15 being a strapping pin.

**Is It Safe?**: Yes, with the correct configuration (`inverted: true`, `pullup: true`).

**Alternative**: Use GPIO16 or GPIO18 if you experience boot issues.

### 5. ESP32 Won't Boot with Receiver Connected

**Possible Cause**: GPIO15 must not be pulled HIGH during boot.

**Solution**: The VS1838B OUT pin is normally HIGH, but our config uses `inverted: true` to handle this. If problems persist, use a different GPIO.

## Testing Without Hardware

### What You CAN Test Without Hardware

✅ **ESPHome Configuration Validation**:
```bash
esphome config firmware/esphome/daikin-ftkb35.yaml
```

✅ **Firmware Compilation**:
```bash
esphome compile firmware/esphome/daikin-ftkb35.yaml
```

✅ **YAML Linting**:
```bash
yamllint firmware/esphome/*.yaml
```

✅ **Language Checks**:
```bash
bash scripts/check.sh
```

✅ **Documentation Review**: All docs can be read and validated

### What You CANNOT Test Without Hardware

❌ **IR LED Transmission**: Requires physical IR LED and transistor circuit

❌ **IR Signal Reception**: Requires VS1838B receiver module

❌ **WiFi Connection**: Requires ESP32 connected to WiFi

❌ **AC Control**: Requires Daikin FTKB35 AC unit

❌ **Protocol Verification**: Requires capturing signals from ARC486A56 remote

❌ **OTA Updates**: Requires ESP32 on network

## Troubleshooting

### Python Version Issues

**Error**: `ERROR: Python 3.12-3.14 not found`

**Solution**: Install Python 3.12 or 3.13 (see Python installation section above)

### ESPHome Installation Failed

**Error**: `pip install esphome` fails

**Solution**: 
1. Ensure you're in the virtual environment: `source .venv/bin/activate`
2. Upgrade pip: `python -m pip install --upgrade pip`
3. Check Python version: `python --version` (should be 3.12-3.14)

### Secrets File Not Found

**Error**: `esphome config` fails with secrets not found

**Solution**: 
1. Run setup script: `bash scripts/setup-dev.sh`
2. Or create manually: `cp firmware/esphome/secrets.example.yaml firmware/esphome/secrets.yaml`

### Permission Denied on Serial Port (Linux)

**Error**: `[Errno 13] Permission denied: '/dev/ttyUSB0'`

**Solution**:
```bash
sudo usermod -a -G dialout $USER
# Log out and log back in, or run: newgrp dialout
```

### ESPHome Command Not Found

**Error**: `bash: esphome: command not found`

**Solution**: Activate the virtual environment:
```bash
source .venv/bin/activate
```

### Check Script Fails on Vietnamese Characters

**Error**: `Vietnamese characters found in unexpected files`

**Why**: The project requires English-only documentation and code (except shop names in `docs/hardware.md`).

**Solution**: 
1. Remove Vietnamese text from the flagged files
2. If the file legitimately needs Vietnamese text (e.g., shop names), add it to the `ALLOWED_FILES` array in `scripts/check.sh`

## Advanced Topics

### Custom ESPHome Components

To add custom components (e.g., external IRremoteESP8266 integration):

1. Create `firmware/esphome/custom_components/`
2. Add component code
3. Reference in YAML with `external_components:`

See [ESPHome Custom Components](https://esphome.io/custom/custom_component.html) for details.

### Multi-Environment Setup

To use both ESPHome 2025.7.x and 2026.x:

```bash
# Create separate venvs
python3.11 -m venv .venv-legacy
python3.13 -m venv .venv-latest

# Activate the one you need
source .venv-legacy/bin/activate  # For 2025.7.x
# or
source .venv-latest/bin/activate  # For 2026.x
```

### CI/CD Integration

The GitHub Actions workflow (`.github/workflows/esphome-validate.yml`) calls the same `scripts/check.sh` script, ensuring local and CI validation never drift.

To run exactly what CI runs:
```bash
bash scripts/check.sh
```

## Next Steps

After setting up your development environment:

1. ✅ [Assemble Phase 1 hardware](hardware.md) - BOM and shopping guide
2. ✅ [Wire the circuit](wiring.md) - IR LED and receiver connections
3. ✅ [Capture IR signals](ir-capture.md) - Verify ARC486A56 protocol
4. ✅ Flash firmware and test AC control
5. ✅ Document findings and update roadmap

## Resources

- [ESPHome Documentation](https://esphome.io/)
- [ESPHome CLI Commands](https://esphome.io/guides/cli.html)
- [ESPHome Climate Components](https://esphome.io/components/climate/)
- [VAir Project Roadmap](roadmap.md)

---

**Last Updated**: October 26, 2026  
**For AI Agents**: See [AGENTS.md](../AGENTS.md) for automated setup instructions.
