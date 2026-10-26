# VAir

**VAir** is an open-source voice-controlled air conditioner device project, initially built for Vietnamese home use. The goal is to enable natural voice control of a Daikin air conditioner using ESP32-based hardware and open-source components.

## Project Status

⚠️ **This is a work-in-progress project in Phase 1.** The current status:

- ✅ **Confirmed**: ESP32 + IR LED hardware setup with ESPHome
- ⚠️ **Not yet confirmed**: Daikin FTKB35 with remote ARC486A56 protocol (likely DAIKIN216, but needs IR capture verification)
- 🚧 **In Progress**: Hardware assembly and IR signal capture
- 📋 **Planned**: Voice control device (Phase 2) and self-hosted server (Phase 3)

See [docs/roadmap.md](docs/roadmap.md) for the complete development plan.

## Target Hardware

- **Air Conditioner**: Daikin FTKB35
- **Remote Control**: ARC486A56 (protocol unconfirmed, suspected DAIKIN216)
- **Phase 1 Controller**: ESP32 WROOM-32 + IR LED
- **Phase 2 Target**: ESP32-S3 voice device (Xiaozhi-style)

## Quick Start (Phase 1)

### Prerequisites

- ESP32 development board (ESP32-WROOM-32 recommended)
- IR LED (940nm), NPN transistor (2N2222 or S8050), resistors (100Ω, 1kΩ)
- Optional: VS1838B IR receiver module for signal capture
- Python 3.12-3.14 (for ESPHome 2026.x) or Python <3.12 (for ESPHome 2025.7.x)

### Hardware Setup

See [docs/wiring.md](docs/wiring.md) for complete wiring instructions and [docs/hardware.md](docs/hardware.md) for the bill of materials.

### Firmware Installation

1. **Clone this repository**:
   ```bash
   git clone https://github.com/pvbthinh100602/VAir.git
   cd VAir/firmware/esphome
   ```

2. **Create secrets file**:
   ```bash
   cp secrets.example.yaml secrets.yaml
   # Edit secrets.yaml with your WiFi credentials and API key
   ```

3. **Install ESPHome** (choose based on Python version):
   ```bash
   # For Python 3.12-3.14 (ESPHome 2026.x)
   pip install esphome

   # For Python <3.12 (ESPHome 2025.7.x)
   pip install "esphome==2025.7.5"
   ```

4. **Compile and flash**:
   ```bash
   esphome run daikin-ftkb35.yaml
   ```

5. **Capture IR signals** (important first step):
   Before attempting to control your AC, you should capture the actual IR signals from your remote to verify the protocol. See [docs/ir-capture.md](docs/ir-capture.md) for detailed instructions.

## Project Phases

### Phase 1: ESP32 + ESPHome IR Control (Current)
Basic IR transmitter using ESP32 and ESPHome. Goal: Verify that the ESP32 can control the Daikin AC unit.

### Phase 2: Voice Control Device
Custom ESP32-S3 board with microphone, speaker, and IR LED. Voice processing using xiaozhi-esp32 firmware with custom board definition and IR control via MCP tool.

### Phase 3: Self-Hosted Server
Vietnamese speech-to-text (STT) and text-to-speech (TTS) support using a self-hosted xiaozhi-esp32-server instance.

See [docs/roadmap.md](docs/roadmap.md) for detailed checklists and goals for each phase.

## Documentation

- [Hardware Bill of Materials](docs/hardware.md) - Components and shopping list
- [Wiring Guide](docs/wiring.md) - Circuit diagrams and connections
- [IR Signal Capture](docs/ir-capture.md) - How to capture and verify remote signals
- [Project Roadmap](docs/roadmap.md) - Development phases and progress

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

For third-party components and their licenses, see [THIRD_PARTY.md](THIRD_PARTY.md).

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## Acknowledgments

This project builds upon excellent open-source work:
- [ESPHome](https://esphome.io/) for the ESP32 firmware framework
- [IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266) for IR protocol implementations
- [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) for voice device inspiration

## Contact

Project maintained by Pham Viet Bao Thinh ([@pvbthinh100602](https://github.com/pvbthinh100602))

## Language Support

While this project is initially built for Vietnamese home use with planned Vietnamese voice control support, all project documentation, code, and comments are maintained in English to facilitate international collaboration.
