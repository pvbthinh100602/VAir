# IR Signal Capture Guide

Before attempting to control your Daikin air conditioner with the ESP32, it's essential to capture the actual IR signals from the original remote control. This verifies the protocol and ensures compatibility.

## Why Capture IR Signals?

The Daikin ARC486A56 remote control's protocol is **not officially confirmed** in documentation. While it's suspected to be **DAIKIN216**, we need to capture actual signals to verify:

1. **Protocol type**: DAIKIN216 (2 frames) vs. DAIKIN (3 frames)
2. **Frame structure**: Header timing, frame spacing, byte counts
3. **Command format**: How temperature, mode, and fan settings are encoded

## Required Hardware

- ESP32 with IR receiver module (VS1838B or similar 38kHz receiver)
- The original Daikin ARC486A56 remote control
- USB connection to computer for viewing logs

## Hardware Setup

### Wiring the IR Receiver

Connect the VS1838B receiver module to your ESP32:

| VS1838B Pin | ESP32 Pin | Notes |
|-------------|-----------|-------|
| VCC | 3.3V | Power supply (can use 5V with voltage divider on OUT) |
| GND | GND | Ground |
| OUT | GPIO15 | Signal output (see note below) |

**GPIO Note**: GPIO15 is a strapping pin and will generate a warning in ESPHome, but it works correctly with the `inverted: true` and `pullup: true` settings. Alternative pins: GPIO16, GPIO18, or other non-strapping GPIOs.

See [wiring.md](wiring.md) for the complete circuit diagram including the transmitter circuit.

## ESPHome Configuration

### Enable the IR Receiver

Edit `firmware/esphome/daikin-ftkb35.yaml` and uncomment the `remote_receiver` section:

```yaml
# --- IR Receiver (OPTIONAL, for capturing real remote signals) ---
# Requires 38kHz receiver module (VS1838B / TSOP / HS0038B), OUT pin connected to GPIO15, VCC to 3.3V, GND to GND.
# Uncomment to use, reflash, point the remote at the module, and view logs (esphome logs daikin-ftkb35.yaml).
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
  # Daikin sends over 300 pulses per frame; if logs are truncated, uncomment the next 2 lines:
  # receive_symbols: 384
  # rmt_symbols: 512
  # GPIO15 is a strapping pin (ESPHome will warn, but it works); can use GPIO16/GPIO18 instead.
```

### Flash the Updated Configuration

```bash
cd firmware/esphome
esphome run daikin-ftkb35.yaml
```

## Capturing Signals

### Step 1: Start Log Monitoring

After flashing, start monitoring the logs:

```bash
esphome logs daikin-ftkb35.yaml
```

Or connect via Home Assistant and view the logs from the device page.

### Step 2: Capture Remote Signals

With logs running:

1. **Point the remote** at the IR receiver module (VS1838B)
2. **Press a button** on the remote (e.g., Power On, Temperature +/-)
3. **Observe the log output** - you should see captured IR data

### Step 3: Interpret the Output

The logs will show the captured IR protocol data. Look for entries like:

```
[D][remote.raw:041]: Received Raw:
  HEADER: 3440us, -1750us
  DATA: ...
```

or

```
[D][remote.aeha:XYZ]: Received AEHA:
  ...
```

## Understanding Daikin Protocols

### DAIKIN vs. DAIKIN216

| Protocol | Frame Count | Total Bytes | Frame Structure | Example Remotes |
|----------|-------------|-------------|-----------------|-----------------|
| **DAIKIN** | 3 frames | 35 bytes | 8 + 8 + 19 bytes, ~29ms apart | ARC433, ARC470 |
| **DAIKIN216** | 2 frames | 27 bytes | 8 + 19 bytes, ~29ms apart | ARC484A4 |

### Header Timing

Both protocols share similar header characteristics:
- **Header mark**: ~3440 µs
- **Header space**: ~1750 µs
- **Frame gap**: ~29 ms (29,000 µs) between frames

### What to Look For

When analyzing your captured signals, identify:

1. **Number of frames**: Count how many distinct data bursts occur
   - 2 frames → Likely DAIKIN216
   - 3 frames → Likely DAIKIN (standard)

2. **Frame spacing**: Measure the gap between frames
   - Should be approximately 29 milliseconds

3. **Header structure**: First pulses should match
   - Mark: ~3440 µs
   - Space: ~1750 µs

4. **Total pulse count**: 
   - DAIKIN: Over 400 pulses (3 frames × 8 bits/byte × frames)
   - DAIKIN216: ~300 pulses (2 frames)

### Example Log Output

#### DAIKIN216 (Expected for ARC486A56)

```
[D][remote.raw:041]: Received Raw:
  HEADER: 3440us, -1750us
  DATA: [8 bytes of data]
  GAP: ~29000us
  HEADER: 3440us, -1750us
  DATA: [19 bytes of data]
```

#### DAIKIN (Standard 3-frame)

```
[D][remote.raw:041]: Received Raw:
  HEADER: 3440us, -1750us
  DATA: [8 bytes of data]
  GAP: ~29000us
  HEADER: 3440us, -1750us
  DATA: [8 bytes of data]
  GAP: ~29000us
  HEADER: 3440us, -1750us
  DATA: [19 bytes of data]
```

## Troubleshooting Signal Capture

### No Signals Captured

**Symptoms**: No log output when pressing remote buttons

**Solutions**:
- Verify receiver wiring (check VCC, GND, OUT connections)
- Test receiver: The OUT pin should be HIGH (~3.3V) when idle, pulse LOW when receiving IR
- Check remote battery
- Reduce distance between remote and receiver (try 10-20 cm)
- Ensure remote is pointing directly at receiver dome

### Truncated Logs

**Symptoms**: Log message "Received Raw: ..." appears cut off

**Solutions**:
Uncomment these lines in `remote_receiver` configuration:

```yaml
receive_symbols: 384
rmt_symbols: 512
```

These increase the buffer size for long Daikin transmissions (over 300 pulses).

### Inconsistent Captures

**Symptoms**: Different patterns each time the same button is pressed

**Solutions**:
- Increase `tolerance` in receiver config (try 60% or 65%)
- Check for IR interference from other sources (sunlight, other remotes, fluorescent lights)
- Ensure stable power supply to ESP32

### Boot Issues

**Symptoms**: ESP32 fails to boot or enters bootloop with receiver connected

**Solutions**:
- GPIO15 is a strapping pin - the receiver OUT should not be externally pulled HIGH during boot
- The configuration uses `inverted: true` and `pullup: true` to mitigate this
- If problems persist, move receiver to GPIO16 or GPIO18
- Disconnect receiver during initial flash, reconnect after boot

## Next Steps After Capture

### If Protocol is DAIKIN216

The current ESPHome `platform: daikin` in the climate configuration sends 3-frame DAIKIN commands. If your captures show DAIKIN216 (2 frames), you'll need to:

1. **Option A**: Use raw IR code transmission
   - Capture specific commands (power on, temp up, temp down, etc.)
   - Store as raw IR codes in ESPHome
   - Replay these codes via the remote_transmitter

2. **Option B**: Use IRremoteESP8266 library directly (Phase 2)
   - The IRDaikin216 class in IRremoteESP8266 library supports DAIKIN216
   - This will be integrated in Phase 2 with the xiaozhi-esp32 firmware

3. **Option C**: Try alternative ESPHome climate platforms
   - Try `platform: daikin_arc` (for Japanese ARC43xxx remotes)
   - Try `platform: heatpumpir` with `protocol: daikin` or `daikin_arc417`

### If Protocol is Standard DAIKIN

Great! The existing ESPHome climate platform should work correctly. You can:
- Keep the current `platform: daikin` configuration
- Proceed with testing AC control (see main README.md)

## Documenting Your Findings

When you successfully capture signals, please document:

1. **Protocol confirmed**: DAIKIN216 or DAIKIN (standard)
2. **Sample raw captures**: For power on, power off, temp changes
3. **Header timings**: Verify they match expected values
4. **Frame count and spacing**: Document what you observed

Consider opening a GitHub issue to share findings and help other ARC486A56 users!

## Reference Links

- [ESPHome Remote Receiver](https://esphome.io/components/remote_receiver.html)
- [IRremoteESP8266 Supported Protocols](https://github.com/crankyoldgit/IRremoteESP8266/blob/master/SupportedProtocols.md)
- [Daikin AC IR Protocol Analysis](https://github.com/crankyoldgit/IRremoteESP8266/blob/master/src/ir_Daikin.cpp)

---

**Last Updated**: October 26, 2026
