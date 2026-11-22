#pragma once

#include "esphome/components/climate_ir/climate_ir.h"

namespace esphome::casper_ykr {

// Timings from IRremoteESP8266 sendElectraAC. The library marks the
// carrier frequency as a guess and the sender as untested on hardware.
static constexpr uint32_t HEADER_MARK = 9166;
static constexpr uint32_t HEADER_SPACE = 4470;
static constexpr uint32_t BIT_MARK = 646;
static constexpr uint32_t ONE_SPACE = 1647;
static constexpr uint32_t ZERO_SPACE = 547;
static constexpr uint32_t MESSAGE_GAP = 100000;
static constexpr uint32_t CARRIER_FREQUENCY = 38000;

// Mode field, stored in the top 3 bits of byte 6.
static constexpr uint8_t MODE_COOL = 0b001;
static constexpr uint8_t MODE_DRY = 0b010;
static constexpr uint8_t MODE_FAN = 0b110;

// Fan field, stored in the top 3 bits of byte 4.
static constexpr uint8_t FAN_HIGH = 0b001;
static constexpr uint8_t FAN_MEDIUM = 0b010;
static constexpr uint8_t FAN_LOW = 0b011;
static constexpr uint8_t FAN_AUTO = 0b101;

// Vertical vane in the low 3 bits of byte 1.
// 5 is the fixed down position in published YKR-T captures.
// 0 is the Electra swing-on code. Neither value is from YKR-C/001.
static constexpr uint8_t VANE_DOWN = 0b101;
static constexpr uint8_t VANE_SWING = 0b000;

static constexpr uint8_t POWER_ON = 0x20;
static constexpr uint8_t SWING_H_OFF = 0xE0;

// Button codes from published YKR-T/011 and YKR-T/101 captures.
static constexpr uint8_t BUTTON_POWER = 0x45;
static constexpr uint8_t BUTTON_TEMP_UP = 0x40;
static constexpr uint8_t BUTTON_TEMP_DOWN = 0x41;
static constexpr uint8_t BUTTON_FAN = 0x44;
static constexpr uint8_t BUTTON_SWING_V = 0x42;

class CasperYkrClimate : public climate_ir::ClimateIR {
 public:
  CasperYkrClimate()
      : climate_ir::ClimateIR(16.0f, 32.0f, 1.0f, true, true,
                              {climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW, climate::CLIMATE_FAN_MEDIUM,
                               climate::CLIMATE_FAN_HIGH},
                              {climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_VERTICAL}) {}

  void setup() override;
  void loop() override;

 protected:
  void transmit_state() override;
  void handle_line_(char *line);

  uint8_t last_mode_{MODE_COOL};
  float last_temperature_{26};
  climate::ClimateFanMode last_fan_{climate::CLIMATE_FAN_AUTO};
  climate::ClimateSwingMode last_swing_{climate::CLIMATE_SWING_OFF};
  climate::ClimateMode last_power_mode_{climate::CLIMATE_MODE_OFF};
  char line_[48]{};
  uint8_t line_length_{0};
};

}  // namespace esphome::casper_ykr
