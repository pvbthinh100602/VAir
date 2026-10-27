# Xiaozhi-ESP32 Firmware for VAir Phase 2

**Status**: 📋 Planned - Not yet implemented

This directory will contain the custom firmware for Phase 2 of the VAir project: a voice-controlled ESP32-S3 device based on the [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) project.

## Overview

Phase 2 will transform VAir from a basic IR transmitter into a standalone voice-controlled device with:
- **ESP32-S3** (N16R8: 16MB Flash, 8MB PSRAM) for AI voice processing
- **INMP441** I2S digital microphone for voice input
- **MAX98357A** I2S amplifier + 3W speaker for audio output
- **IR LED** for controlling the Daikin AC unit
- **xiaozhi-esp32** firmware with custom board definition and AC control MCP tool

## Planned Architecture

### Hardware Platform
- **Board**: ESP32-S3-DevKitC-1 N16R8 (or custom PCB in future)
- **Firmware Base**: xiaozhi-esp32 (MIT licensed)
- **Build System**: ESP-IDF 6.0
- **IR Library**: jorgecis/IRremoteIDF ^1.0.2 (LGPL-2.1, component registry)

### Software Components

#### Custom Board Definition
File: `main/boards/vair-daikin.h` (to be created)

Based on `bread-compact-wifi` board from xiaozhi-esp32, customized for VAir hardware:
- GPIO mappings for I2S microphone, amplifier, and IR LED
- Hardware configuration for Vietnamese voice control

#### MCP Tool for AC Control
Function: `self.ac.set_state(power, temperature, mode)`

Implements AC control via IRremoteESP8266's `IRDaikin216` class:
- **power**: "on" or "off"
- **temperature**: 16-30°C
- **mode**: "cool", "dry", "fan", "auto"

This tool will be callable by the xiaozhi AI agent to control the AC based on voice commands.

### Proposed GPIO Mapping

⚠️ **These GPIO assignments are preliminary and not yet verified on real hardware.**

| Component | Signal | GPIO | Notes |
|-----------|--------|------|-------|
| IR Transmitter | TX | GPIO1 | Via transistor circuit (same as Phase 1) |
| IR Receiver | RX | GPIO2 | Optional, for feedback verification |
| INMP441 Mic | SCK | TBD | I2S clock |
| INMP441 Mic | WS | TBD | I2S word select |
| INMP441 Mic | SD | TBD | I2S data |
| MAX98357A Amp | DIN | TBD | I2S data input |
| MAX98357A Amp | BCLK | TBD | I2S bit clock |
| MAX98357A Amp | LRC | TBD | I2S left/right clock |

GPIO assignments will be finalized during Phase 2 hardware testing.

## Build Requirements

### Prerequisites
- **ESP-IDF 6.0** (Espressif IoT Development Framework)
- **IRremoteIDF** component from ESP registry: `jorgecis/IRremoteIDF` ^1.0.2
- **xiaozhi-esp32** repository cloned and configured
- **Python 3.12+** for ESP-IDF toolchain
- **Git** for cloning repositories

### Dependencies
- All standard xiaozhi-esp32 dependencies
- IRremoteESP8266 (via IRremoteIDF wrapper, LGPL-2.1)
- ESP32-S3 support in ESP-IDF

## Development Roadmap

### Milestone 1: Xiaozhi-ESP32 Setup
- [ ] Clone xiaozhi-esp32 repository
- [ ] Install ESP-IDF 6.0 toolchain
- [ ] Build and flash xiaozhi-esp32 to ESP32-S3 DevKit
- [ ] Verify basic voice agent functionality (English)

### Milestone 2: Hardware Integration
- [ ] Acquire ESP32-S3 N16R8 board and audio components
- [ ] Test INMP441 microphone (record and verify audio input)
- [ ] Test MAX98357A amplifier + speaker (play test tones)
- [ ] Finalize GPIO pinout based on testing

### Milestone 3: Custom Board Definition
- [ ] Create `vair-daikin.h` board file
- [ ] Configure GPIO mappings for all peripherals
- [ ] Set up I2S configuration for audio
- [ ] Add IR transmitter/receiver pins
- [ ] Test complete hardware stack

### Milestone 4: IR Control Integration
- [ ] Add IRremoteIDF component to project
- [ ] Verify DAIKIN216 class availability and compatibility
- [ ] Build test firmware with IRremoteIDF
- [ ] Test IR transmission from ESP32-S3

### Milestone 5: MCP Tool Implementation
- [ ] Implement `self.ac.set_state()` MCP tool
- [ ] Map voice commands to AC control parameters
- [ ] Test AC control via voice commands (English)
- [ ] Handle edge cases and errors

### Milestone 6: Testing and Refinement
- [ ] End-to-end testing: voice → AI → AC control
- [ ] Optimize latency and responsiveness
- [ ] Test various AC commands (temperature, mode, fan)
- [ ] Document working configuration

## Known Challenges

### Unverified Build
The IRremoteIDF component version 1.0.2+ with `IRDaikin216` class has not been tested on ESP-IDF 6.0 in this project. Compatibility verification is required.

### PSRAM Requirements
Voice processing in xiaozhi-esp32 may require 8MB PSRAM. The cheaper ESP32-S3 N8R2 variant (2MB PSRAM) has not been tested and may have insufficient memory.

### Protocol Compatibility
Phase 2 depends on Phase 1 confirming the Daikin ARC486A56 remote protocol. If the protocol is not DAIKIN216, alternative IR libraries or custom implementations may be needed.

### Licensing
IRremoteESP8266/IRremoteIDF is LGPL-2.1, which requires:
- Providing source code for LGPL components when distributing binaries
- Allowing users to replace LGPL components in the firmware

See [THIRD_PARTY.md](../../THIRD_PARTY.md) for full license compliance information.

## Resources

- [xiaozhi-esp32 GitHub Repository](https://github.com/78/xiaozhi-esp32)
- [IRremoteIDF Component Registry](https://components.espressif.com/components/jorgecis/IRremoteIDF)
- [IRremoteESP8266 Documentation](https://github.com/crankyoldgit/IRremoteESP8266)
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/)
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)

## Future: Custom PCB

Once the GPIO mapping and component selection are validated, a custom PCB design may be created to integrate:
- ESP32-S3 module
- INMP441 microphone breakout
- MAX98357A amplifier circuit
- Speaker connector
- IR LED driver circuit
- Power supply circuit (USB-C or battery)
- Status LEDs

PCB design files will be added to this directory when available.

## Contributing

Phase 2 development will begin once Phase 1 IR control is confirmed working. If you'd like to contribute:
- Test the hardware components independently
- Experiment with xiaozhi-esp32 firmware
- Document GPIO configurations for ESP32-S3 + audio setup
- Share findings in GitHub Issues or Discussions

---

**Last Updated**: October 26, 2026  
**Status**: Waiting for Phase 1 completion before implementation begins.
