#pragma once

#include "esphome/components/climate_ir/climate_ir.h"

namespace esphome::daikin_ftkb {

// Timings measured from the ARC486A56 remote on 2026-11-03.
static constexpr uint32_t HEADER_MARK = 3490;
static constexpr uint32_t HEADER_SPACE = 1750;
static constexpr uint32_t BIT_MARK = 430;
static constexpr uint32_t ONE_SPACE = 1300;
static constexpr uint32_t ZERO_SPACE = 440;
static constexpr uint32_t PREAMBLE_GAP = 25000;
static constexpr uint32_t FRAME_GAP = 2270;
static constexpr uint32_t CARRIER_FREQUENCY = 38000;

// Frame 3 mode byte, with the power bit clear.
static constexpr uint8_t MODE_COOL = 0x30;
static constexpr uint8_t MODE_DRY = 0x20;
static constexpr uint8_t MODE_FAN = 0x60;
static constexpr uint8_t POWER_ON = 0x01;

// High nibble of frame 3 byte 8.
// Captured: level 3 = 5, level 4 = 6, level 5 = 7, auto = A, night/sleep = B.
// Level 1 uses the standard Daikin code and is not confirmed.
static constexpr uint8_t FAN_1 = 0x30;
static constexpr uint8_t FAN_3 = 0x50;
static constexpr uint8_t FAN_4 = 0x60;
static constexpr uint8_t FAN_5 = 0x70;
static constexpr uint8_t FAN_AUTO = 0xA0;
static constexpr uint8_t FAN_QUIET = 0xB0;
static constexpr uint8_t SWING_VERTICAL = 0x0F;

class DaikinFtkbClimate : public climate_ir::ClimateIR {
 public:
  DaikinFtkbClimate()
      : climate_ir::ClimateIR(10.0f, 30.0f, 0.5f, true, true,
                              {climate::CLIMATE_FAN_QUIET, climate::CLIMATE_FAN_AUTO, climate::CLIMATE_FAN_LOW,
                               climate::CLIMATE_FAN_MEDIUM, climate::CLIMATE_FAN_MIDDLE, climate::CLIMATE_FAN_HIGH},
                              {climate::CLIMATE_SWING_OFF, climate::CLIMATE_SWING_VERTICAL}) {}

  void setup() override;
  void loop() override;

 protected:
  void transmit_state() override;
  void handle_line_(char *line);

  uint8_t last_mode_{MODE_COOL};
  char line_[48]{};
  uint8_t line_length_{0};
};

}  // namespace esphome::daikin_ftkb
