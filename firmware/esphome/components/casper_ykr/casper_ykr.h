#pragma once

#include "esphome/components/climate_ir/climate_ir.h"

#include "casper_ykr_frame.h"

namespace esphome::casper_ykr {

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

  Memory memory_{};
  char line_[48]{};
  uint8_t line_length_{0};
};

}  // namespace esphome::casper_ykr
