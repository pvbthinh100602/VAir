# VAir Project Roadmap

This document tracks the development phases and progress for the VAir voice-controlled air conditioner project.

## Development Philosophy

The VAir project follows an incremental approach:
1. **Verify** basic IR control before adding complexity
2. **Test** each component independently before integration
3. **Document** findings and unconfirmed assumptions clearly
4. **Iterate** based on real-world testing results

---

## Phase 1: ESP32 + ESPHome IR Control

**Goal**: Establish basic infrared control of the Daikin AC unit using ESP32 and ESPHome firmware.

**Status**: 🚧 In Progress

### Objectives

- [x] Define hardware requirements (ESP32, IR LED, transistor circuit)
- [x] Source components locally in HCMC
- [x] Create ESPHome configuration for Daikin AC
- [ ] Build physical circuit on breadboard
- [ ] Flash ESPHome firmware to ESP32
- [ ] Capture IR signals from original remote (ARC486A56)
- [ ] Verify protocol type (DAIKIN216 vs. standard DAIKIN)
- [ ] Test AC response to ESP32 IR commands
- [ ] Document confirmed protocol and working commands
- [ ] Validate power on/off control
- [ ] Validate temperature control (cooling mode)
- [ ] Validate fan speed control
- [ ] Validate mode switching (cool/dry/fan)

### Key Deliverables

- ✅ Complete wiring guide and BOM
- ✅ ESPHome YAML configuration file
- ✅ IR signal capture documentation
- [ ] Confirmed protocol specification for ARC486A56
- [ ] Tested and working IR control setup
- [ ] Troubleshooting guide for common issues

### Known Uncertainties

⚠️ **Protocol Not Confirmed**: The Daikin ARC486A56 remote protocol is assumed to be DAIKIN216 based on similarity to ARC484A4, but this has not been verified with actual signal captures.

⚠️ **ESPHome Native Support**: ESPHome's built-in `platform: daikin` sends 3-frame DAIKIN commands. If ARC486A56 uses DAIKIN216 (2-frame), alternative approaches will be needed:
- Raw IR code replay
- Alternative climate platforms (daikin_arc, heatpumpir)
- Custom component using IRremoteESP8266

### Success Criteria

Phase 1 is complete when:
- IR protocol is captured and confirmed
- ESP32 can reliably turn the AC on/off
- ESP32 can change temperature settings
- Documentation reflects actual tested behavior

---

## Phase 2: Voice Control Device (Xiaozhi-based)

**Goal**: Build a standalone voice-controlled device using ESP32-S3 with microphone, speaker, and IR control.

**Status**: 📋 Planned

### Objectives

- [ ] Acquire Phase 2 hardware (ESP32-S3 N16R8, INMP441, MAX98357A, speaker)
- [ ] Test audio input (INMP441 microphone)
- [ ] Test audio output (MAX98357A amplifier + speaker)
- [ ] Clone and build xiaozhi-esp32 firmware
- [ ] Create custom board definition (based on bread-compact-wifi)
- [ ] Integrate IRremoteESP8266 via jorgecis/IRremoteIDF component
- [ ] Implement MCP tool: `self.ac.set_state(power, temperature, mode)`
- [ ] Test IR transmission from ESP32-S3
- [ ] Verify xiaozhi-esp32 AI voice agent functionality
- [ ] Map GPIO pins for production board design
- [ ] Test voice commands in English
- [ ] Integrate AC control into voice agent workflow
- [ ] Design and order custom PCB (optional)
- [ ] Test complete system: voice → AI → IR → AC

### Key Deliverables

- [ ] Custom board definition file for xiaozhi-esp32
- [ ] MCP tool implementation for AC control
- [ ] ESP-IDF build configuration (IDF 6.0)
- [ ] GPIO pinout documentation
- [ ] Voice command examples and testing guide
- [ ] (Optional) Custom PCB design files

### Dependencies

- **Phase 1 completion**: IR protocol must be confirmed and working
- **Hardware procurement**: ESP32-S3 and audio components
- **xiaozhi-esp32 setup**: Build environment for ESP-IDF 6.0
- **IRremoteIDF compatibility**: Version 1.0.2+ with DAIKIN216 support

### Technical Challenges

⚠️ **Unverified Build**: IRremoteIDF 1.0.2 with IRDaikin216 class has not been tested on ESP-IDF 6.0 in this project.

⚠️ **GPIO Mapping**: Proposed GPIO assignments (IR TX: GPIO1, IR RX: GPIO2) are untested on real hardware.

⚠️ **PSRAM Requirements**: Voice processing may require 8MB PSRAM (N16R8 board); cheaper N8R2 boards with 2MB PSRAM are unverified.

⚠️ **Audio Quality**: INMP441 + MAX98357A combination is widely used but not yet tested in this specific configuration.

### Success Criteria

Phase 2 is complete when:
- Voice commands can control the AC through the device
- Audio input and output quality is acceptable
- IR control works reliably from the ESP32-S3
- System is stable for extended operation
- English voice commands are recognized accurately

---

## Phase 3: Self-Hosted Server + Vietnamese Language

**Goal**: Deploy a self-hosted xiaozhi-esp32-server and add Vietnamese speech support.

**Status**: 📋 Planned

### Objectives

- [ ] Set up server infrastructure (Linux VM or dedicated hardware)
- [ ] Deploy xinnan-tech/xiaozhi-esp32-server
- [ ] Configure ESP32-S3 device to connect to self-hosted server
- [ ] Test server communication and voice processing
- [ ] Research Vietnamese STT (Speech-to-Text) models
- [ ] Research Vietnamese TTS (Text-to-Speech) models
- [ ] Integrate Vietnamese STT into server
- [ ] Integrate Vietnamese TTS into server
- [ ] Test Vietnamese voice commands
- [ ] Research Vietnamese wake word detection
- [ ] Implement Vietnamese wake word (if feasible)
- [ ] Optimize latency for local processing
- [ ] Document server setup and configuration
- [ ] Create user guide for Vietnamese voice commands

### Key Deliverables

- [ ] Server deployment guide (Docker + configuration)
- [ ] Vietnamese STT/TTS integration documentation
- [ ] Vietnamese voice command examples
- [ ] Network configuration guide (local server connection)
- [ ] Performance benchmarking (latency, accuracy)
- [ ] (Optional) Vietnamese wake word implementation

### Dependencies

- **Phase 2 completion**: Voice device must be working with English
- **Server hardware**: Adequate compute for STT/TTS processing
- **Vietnamese models**: Availability of quality open-source models
- **Network setup**: Reliable local network between device and server

### Technical Challenges

⚠️ **Vietnamese STT/TTS**: Availability and quality of open-source Vietnamese speech models is TBD. May need to evaluate:
- VAIS (Vietnamese AI Speech) models
- Vinbrain speech models
- International models with Vietnamese support (Whisper, etc.)

⚠️ **Wake Word**: Vietnamese wake word detection is challenging and may require custom training.

⚠️ **Latency**: Self-hosted processing must be fast enough for good user experience (target: <2 seconds response time).

⚠️ **Server Resources**: STT/TTS may require GPU acceleration for acceptable performance.

### Success Criteria

Phase 3 is complete when:
- Self-hosted server is operational and stable
- Vietnamese voice commands work with good accuracy
- Response latency is acceptable for interactive use
- System can run 24/7 without intervention
- User documentation is complete and tested

---

## Future Enhancements (Post-Phase 3)

These are ideas for future development beyond the three main phases:

### Hardware Improvements
- [ ] Custom PCB with integrated components
- [ ] 3D-printed enclosure design
- [ ] Battery backup for power outage resilience
- [ ] Temperature/humidity sensor integration
- [ ] Status LED indicators

### Software Features
- [ ] Multi-room support (control multiple AC units)
- [ ] Scheduling and automation (time-based control)
- [ ] Energy usage tracking
- [ ] Integration with home automation platforms (Home Assistant, MQTT)
- [ ] Mobile app for monitoring and control
- [ ] Over-the-air (OTA) firmware updates
- [ ] Voice command customization

### AI and Voice Improvements
- [ ] Natural language understanding (more flexible commands)
- [ ] Context awareness (remember previous settings)
- [ ] Multi-language support (English + Vietnamese simultaneously)
- [ ] Voice personalization (recognize different household members)
- [ ] Offline voice processing (no server required)

### Compatibility
- [ ] Support for other Daikin remote models
- [ ] Support for other AC brands (Mitsubishi, LG, Panasonic, etc.)
- [ ] Protocol documentation for DIY community

---

## Project Metrics and Tracking

### Completed Milestones
- ✅ Project planning and documentation structure (Oct 2026)
- ✅ Hardware BOM and sourcing guide (Oct 2026)
- ✅ Phase 1 ESPHome configuration (Oct 2026)

### In Progress
- 🚧 Phase 1 hardware assembly and testing

### Next Up
- ⏳ IR protocol verification
- ⏳ First successful AC control test

### Timeline Considerations

This is an open-source hobby project with no fixed deadlines. Progress depends on:
- Hardware availability and delivery times
- Testing and debugging duration
- Community contributions and feedback
- Personal time availability of the maintainer

**Note**: As a cloud agent project, specific calendar estimates (days/weeks) are not provided. Each phase will be completed as technical challenges are resolved and hardware is validated.

---

## Contributing to the Roadmap

If you're working on VAir or a similar project:

1. **Share findings**: Open GitHub issues to report protocol confirmations, hardware compatibility, or bugs
2. **Submit improvements**: Pull requests for documentation, code, or circuit designs are welcome
3. **Suggest features**: Use GitHub Discussions for feature ideas or roadmap suggestions
4. **Help others**: Answer questions and share your experience in Issues

## References and Resources

- [ESPHome Documentation](https://esphome.io/)
- [IRremoteESP8266 Library](https://github.com/crankyoldgit/IRremoteESP8266)
- [xiaozhi-esp32 Project](https://github.com/78/xiaozhi-esp32)
- [xiaozhi-esp32-server](https://github.com/xinnan-tech/xiaozhi-esp32-server)
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/)
- [Daikin IR Protocol Info](https://github.com/crankyoldgit/IRremoteESP8266/blob/master/SupportedProtocols.md#daikin)

---

**Last Updated**: October 26, 2026  
**Current Phase**: Phase 1 - Hardware assembly and IR verification
