# Third-Party Components and Licenses

This project incorporates or depends on the following third-party open-source software. Please note the respective licenses and ensure compliance when using or distributing this project.

## Direct Dependencies

### IRremoteESP8266
- **License**: GNU Lesser General Public License v2.1 (LGPL-2.1)
- **Repository**: https://github.com/crankyoldgit/IRremoteESP8266
- **Usage**: IR protocol implementations (specifically targeting DAIKIN216 for Phase 2)
- **Note**: LGPL-2.1 requires that modifications to IRremoteESP8266 itself be released under LGPL-2.1, but allows linking from MIT-licensed code. When distributing binaries that include IRremoteESP8266, you must provide means for users to replace the IRremoteESP8266 component.

### xiaozhi-esp32
- **License**: MIT License
- **Repository**: https://github.com/78/xiaozhi-esp32
- **Usage**: Voice device firmware base (Phase 2)
- **Note**: This project uses xiaozhi-esp32 as a firmware foundation with custom board definitions and additional MCP tools for AC control.

### ESPHome
- **License**: MIT License (core) with various component licenses
- **Website**: https://esphome.io/
- **Repository**: https://github.com/esphome/esphome
- **Usage**: Firmware framework for Phase 1 ESP32 IR control

### jorgecis/IRremoteIDF
- **License**: LGPL-2.1 (ESP-IDF component wrapper for IRremoteESP8266)
- **Component Registry**: https://components.espressif.com/components/jorgecis/IRremoteIDF
- **Usage**: Planned for Phase 2 (ESP-IDF integration)
- **Version**: ^1.0.2
- **Note**: Same LGPL-2.1 obligations as IRremoteESP8266

## Indirect Dependencies

This project also depends on numerous packages from the ESPHome and ESP-IDF ecosystems. Please refer to:
- ESPHome component licenses: https://esphome.io/
- ESP-IDF licenses: https://github.com/espressif/esp-idf/blob/master/LICENSE

## Branding Notice

This project is independent and not officially affiliated with, endorsed by, or sponsored by:
- ESPHome or the ESPHome project
- Home Assistant or Nabu Casa, Inc.
- Xiaozhi or the xiaozhi-esp32 project authors
- Espressif Systems
- Daikin Industries, Ltd.

**Do not use the names, trademarks, logos, or branding of these projects or companies to promote this project.** References to these projects are for technical attribution and compatibility information only.

## License Compliance Summary

| Component | License | Obligation for Distribution |
|-----------|---------|------------------------------|
| VAir (this project) | MIT | Provide copyright notice and license |
| IRremoteESP8266 | LGPL-2.1 | Allow users to replace LGPL components; provide source for LGPL parts |
| xiaozhi-esp32 | MIT | Provide copyright notice and license |
| ESPHome | MIT | Provide copyright notice and license |
| IRremoteIDF | LGPL-2.1 | Same as IRremoteESP8266 |

## Updates

This document should be updated whenever third-party dependencies are added, removed, or changed. If you notice any missing attributions, please open an issue or pull request.

---

Last updated: October 26, 2026
