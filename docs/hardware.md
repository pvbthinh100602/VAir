# Hardware Bill of Materials (BOM)

This document lists all the hardware components needed for the VAir project, organized by phase.

**Important Notes:**
- Prices are in Vietnamese Dong (VND)
- Prices were checked on October 25-26, 2026 in Ho Chi Minh City
- Prices do not include shipping
- Shop locations are provided for local (HCMC) sourcing
- Component specifications may vary; verify compatibility before purchasing

## Phase 1: ESP32 + IR Control (~175,000 VND)

This phase requires basic ESP32 hardware and IR components for testing AC control.

### Components

| Component | Specification | Quantity | Price (VND) | Shop | Notes |
|-----------|--------------|----------|-------------|------|-------|
| ESP32 Development Board | NodeMCU-32S, 38-pin, Type-C | 1 | 135,000 | Caka | ESP32-WROOM-32 based |
| IR LED | 940nm, 5mm | 1 | 1,000 | Caka | Infrared transmitter |
| NPN Transistor | 2N2222 or S8050, TO-92 | 1 | 1,000 | Caka | Check pinout with multimeter |
| Resistor | 100Ω | 1 | (pack) | Caka | For current limiting |
| Resistor | 1kΩ | 1 | (pack) | Caka | For transistor base |
| Resistor Pack | 100Ω + 1kΩ + 10kΩ assorted | 1 pack | 10,000 | Caka | Multiple values included |
| Perforated PCB | 5x7 cm | 1 | 9,000 | Caka | For prototyping |
| Female Header Pins | 1x40 pin strip | 2 | 6,000 each | Caka | For ESP32 socket |
| IR Receiver Module | VS1838B (38kHz) | 1 | 3,000 | Caka | Optional: for signal capture |
| Wire | 30AWG | 1 roll | 10,000 | Caka | For connections |

**Phase 1 Total: ~175,000 VND**

### Optional Tools
- Multimeter (for testing and verifying transistor pinout)
- Smartphone with camera (for testing IR LED visibility)
- Soldering iron and solder (if making permanent connections)

## Phase 2: Voice Control Device (~375,000 VND)

This phase adds voice recognition capability with microphone and speaker.

### Components

| Component | Specification | Quantity | Price (VND) | Shop | Notes |
|-----------|--------------|----------|-------------|------|-------|
| ESP32-S3 Dev Board | ESP32-S3-DevKitC-1 N16R8 (16MB Flash, 8MB PSRAM) | 1 | 239,000 | IC Đây Rồi | Recommended option |
| *Alternative* | ESP32-S3 N8R2 (8MB Flash, 2MB PSRAM) | 1 | 195,000 | Caka | Cheaper, only 2MB PSRAM (unverified) |
| I2S Microphone | INMP441 | 1 | 45,000 | Hshop | Digital I2S MEMS mic |
| I2S Audio Amplifier | MAX98357A | 1 | 40,000 | Hshop | 3W Class D amplifier |
| Speaker | 4Ω, 3W, 40mm diameter | 1 | 22,000 | Caka | Compact full-range |
| IR LED | 940nm, 5mm | 1 | 1,000 | Caka | Infrared transmitter |
| NPN Transistor | 2N2222 or S8050 | 1 | 1,000 | Caka | For IR LED driving |
| Resistors | 100Ω, 1kΩ (from pack) | - | (included) | Caka | From Phase 1 pack |
| Misc. Hardware | - | - | ~27,000 | Caka | PCB, headers, wire |

**Phase 2 Total: ~375,000 VND** (with N16R8 board)

### Proposed GPIO Mapping (Phase 2, Unverified)
- **IR TX**: GPIO1 (via transistor)
- **IR RX**: GPIO2 (for feedback/verification)
- **I2S Microphone (INMP441)**: SCK, WS, SD pins (to be confirmed)
- **I2S Amplifier (MAX98357A)**: DIN, BCLK, LRC pins (to be confirmed)

**Note**: GPIO assignments are preliminary and not yet tested on real hardware.

## Phase 3: Self-Hosted Server

Phase 3 requires a server (can be existing hardware) for:
- Self-hosted xiaozhi-esp32-server instance
- Vietnamese STT/TTS models (future)
- Network connectivity to ESP32-S3 device

**No additional component costs** if using existing server hardware.

## Shopping Locations (Ho Chi Minh City)

| Shop Name | Address | Notes |
|-----------|---------|-------|
| Caka | 40/12 Lữ Gia, Quận 11 | General electronics, good starter kit source |
| IC Đây Rồi | Thủ Đức | Premium ESP32-S3 boards |
| Hshop | 269/20 Lý Thường Kiệt | I2S audio components |
| Banlinhkien | 71 Ngô Quyền, Quận 10 | General components |
| Điện tử Á Châu | 248 Tân Phước, Quận 10 | General electronics |
| PYWORLD | 104/51 Thành Thái, Quận 10 | Development boards |

## Sourcing Notes

- **Local vs. Online**: These prices are for local HCMC shops. Online marketplaces (Shopee, Lazada) may have different prices.
- **Quality**: IR LEDs should be 940nm for optimal compatibility with AC receivers. Verify with the seller.
- **Transistor Pinout**: TO-92 transistors can have different pinouts (EBC vs. CBE). Always verify with a multimeter before soldering.
- **ESP32-S3 PSRAM**: The N16R8 variant with 8MB PSRAM is recommended for voice processing. The cheaper N8R2 (2MB PSRAM) is untested for this application.

## Future Updates

As components are tested and validated, this BOM will be updated with:
- Confirmed GPIO mappings for Phase 2
- Power supply requirements and recommendations
- Enclosure/case options
- Additional testing equipment recommendations

---

**Last Updated**: October 26, 2026  
**Prices Subject to Change**: Verify current pricing with shops before purchasing.
