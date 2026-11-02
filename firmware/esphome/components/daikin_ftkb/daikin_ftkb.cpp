#include "daikin_ftkb.h"
#include "daikin_ftkb_raw.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include <driver/uart.h>

namespace esphome::daikin_ftkb {

static void send_byte(remote_base::RemoteTransmitData *data, uint8_t value) {
  for (uint8_t mask = 1; mask != 0; mask <<= 1) {
    data->mark(BIT_MARK);
    data->space((value & mask) != 0 ? ONE_SPACE : ZERO_SPACE);
  }
}

static void send_frame(remote_base::RemoteTransmitData *data, const uint8_t *bytes, size_t length,
                       uint32_t trailing_space) {
  data->mark(HEADER_MARK);
  data->space(HEADER_SPACE);
  for (size_t i = 0; i < length; i++) {
    send_byte(data, bytes[i]);
  }
  data->mark(BIT_MARK);
  data->space(trailing_space);
}

static const char *const TAG = "daikin_ftkb";

static const char *mode_name(climate::ClimateMode mode) {
  switch (mode) {
    case climate::CLIMATE_MODE_OFF:
      return "off";
    case climate::CLIMATE_MODE_COOL:
      return "cool";
    case climate::CLIMATE_MODE_DRY:
      return "dry";
    case climate::CLIMATE_MODE_FAN_ONLY:
      return "fan";
    default:
      return "unknown";
  }
}

static const char *fan_name(climate::ClimateFanMode mode) {
  switch (mode) {
    case climate::CLIMATE_FAN_QUIET:
      return "quiet";
    case climate::CLIMATE_FAN_AUTO:
      return "auto";
    case climate::CLIMATE_FAN_LOW:
      return "low";
    case climate::CLIMATE_FAN_MEDIUM:
      return "medium";
    case climate::CLIMATE_FAN_MIDDLE:
      return "middle";
    case climate::CLIMATE_FAN_HIGH:
      return "high";
    default:
      return "unknown";
  }
}

static void send_raw(remote_base::RemoteTransmitData *data, const int16_t *pulses, size_t count) {
  data->set_carrier_frequency(CARRIER_FREQUENCY);
  for (size_t i = 0; i < count; i++) {
    const int16_t value = pulses[i];
    if (value >= 0) {
      data->mark(value);
    } else {
      data->space(static_cast<uint32_t>(-value));
    }
  }
}

void DaikinFtkbClimate::setup() {
  climate_ir::ClimateIR::setup();
  ESP_LOGI(TAG, "UART control ready. Type 'help' and press Enter.");
}

void DaikinFtkbClimate::loop() {
  // The logger owns UART0 through the ESP-IDF driver. Arduino Serial does not see those bytes.
  uint8_t raw = 0;
  while (uart_read_bytes(UART_NUM_0, &raw, 1, 0) > 0) {
    const char ch = static_cast<char>(raw);
    if (ch == '\r' || ch == '\n') {
      this->line_[this->line_length_] = '\0';
      if (this->line_length_ > 0) {
        this->handle_line_(this->line_);
      }
      this->line_length_ = 0;
      continue;
    }
    if (this->line_length_ + 1 < sizeof(this->line_)) {
      this->line_[this->line_length_++] = ch;
    }
  }
}

void DaikinFtkbClimate::handle_line_(char *line) {
  while (*line == ' ') {
    line++;
  }
  for (char *p = line; *p != '\0'; p++) {
    if (*p >= 'A' && *p <= 'Z') {
      *p = static_cast<char>(*p - 'A' + 'a');
    }
  }
  size_t length = strlen(line);
  while (length > 0 && line[length - 1] == ' ') {
    line[--length] = '\0';
  }

  auto call = this->make_call();
  bool transmit = false;

  if (strcmp(line, "help") == 0) {
    ESP_LOGI(TAG, "Commands:");
    ESP_LOGI(TAG, "  off");
    ESP_LOGI(TAG, "  mode cool [temperature]");
    ESP_LOGI(TAG, "  mode dry [temperature]");
    ESP_LOGI(TAG, "  mode fan");
    ESP_LOGI(TAG, "  temp 26.5");
    ESP_LOGI(TAG, "  fan quiet|auto|low|medium|middle|high");
    ESP_LOGI(TAG, "  swing on|off");
    ESP_LOGI(TAG, "  test");
    ESP_LOGI(TAG, "  replay on|off");
    ESP_LOGI(TAG, "  status");
    return;
  }
  if (strcmp(line, "test") == 0) {
    auto transmit = this->transmitter_->transmit();
    auto *data = transmit.get_data();
    data->set_carrier_frequency(CARRIER_FREQUENCY);
    data->mark(1000000);
    data->space(1000);
    transmit.perform();
    ESP_LOGI(TAG, "LED test: 1 second carrier. Look at the LED with a phone camera.");
    return;
  }
  if (strcmp(line, "replay on") == 0 || strcmp(line, "replay off") == 0) {
    const bool power_on = strcmp(line, "replay on") == 0;
    const int16_t *pulses = power_on ? kCapturedPowerOn : kCapturedPowerOff;
    const size_t count = power_on ? sizeof(kCapturedPowerOn) / sizeof(kCapturedPowerOn[0])
                                  : sizeof(kCapturedPowerOff) / sizeof(kCapturedPowerOff[0]);
    auto transmit = this->transmitter_->transmit();
    send_raw(transmit.get_data(), pulses, count);
    transmit.perform();
    ESP_LOGI(TAG, "Replayed captured %s frame", power_on ? "power-on" : "power-off");
    return;
  }
  if (strcmp(line, "status") == 0) {
    const char *fan = this->fan_mode.has_value() ? fan_name(*this->fan_mode) : "none";
    const char *swing = this->swing_mode == climate::CLIMATE_SWING_VERTICAL ? "on" : "off";
    ESP_LOGI(TAG, "mode=%s temp=%.1f fan=%s swing=%s", mode_name(this->mode), this->target_temperature, fan, swing);
    return;
  }
  if (strcmp(line, "off") == 0) {
    call.set_mode(climate::CLIMATE_MODE_OFF);
    transmit = true;
  } else if (strncmp(line, "mode ", 5) == 0) {
    const char *arg = line + 5;
    char mode[8] = {};
    float temperature = 0;
    const int count = sscanf(arg, "%7s %f", mode, &temperature);
    if (strcmp(mode, "cool") == 0) {
      call.set_mode(climate::CLIMATE_MODE_COOL);
    } else if (strcmp(mode, "dry") == 0) {
      call.set_mode(climate::CLIMATE_MODE_DRY);
    } else if (strcmp(mode, "fan") == 0) {
      call.set_mode(climate::CLIMATE_MODE_FAN_ONLY);
    } else {
      ESP_LOGI(TAG, "Mode must be cool, dry, or fan");
      return;
    }
    if (count == 2) {
      call.set_target_temperature(temperature);
    }
    transmit = true;
  } else if (strncmp(line, "temp ", 5) == 0) {
    const float temperature = strtof(line + 5, nullptr);
    call.set_target_temperature(temperature);
    transmit = true;
  } else if (strncmp(line, "fan ", 4) == 0) {
    const char *arg = line + 4;
    climate::ClimateFanMode fan;
    if (strcmp(arg, "quiet") == 0) {
      fan = climate::CLIMATE_FAN_QUIET;
    } else if (strcmp(arg, "auto") == 0) {
      fan = climate::CLIMATE_FAN_AUTO;
    } else if (strcmp(arg, "low") == 0) {
      fan = climate::CLIMATE_FAN_LOW;
    } else if (strcmp(arg, "medium") == 0) {
      fan = climate::CLIMATE_FAN_MEDIUM;
    } else if (strcmp(arg, "middle") == 0) {
      fan = climate::CLIMATE_FAN_MIDDLE;
    } else if (strcmp(arg, "high") == 0) {
      fan = climate::CLIMATE_FAN_HIGH;
    } else {
      ESP_LOGI(TAG, "Fan must be quiet, auto, low, medium, middle, or high");
      return;
    }
    call.set_fan_mode(fan);
    transmit = true;
  } else if (strncmp(line, "swing ", 6) == 0) {
    const char *arg = line + 6;
    if (strcmp(arg, "on") == 0) {
      call.set_swing_mode(climate::CLIMATE_SWING_VERTICAL);
    } else if (strcmp(arg, "off") == 0) {
      call.set_swing_mode(climate::CLIMATE_SWING_OFF);
    } else {
      ESP_LOGI(TAG, "Swing must be on or off");
      return;
    }
    transmit = true;
  } else {
    ESP_LOGI(TAG, "Unknown command. Type 'help'.");
    return;
  }

  if (transmit) {
    call.perform();
    ESP_LOGI(TAG, "Sent");
  }
}

void DaikinFtkbClimate::transmit_state() {
  // Frames 1 and 2 were identical in every ARC486A56 capture.
  const uint8_t frame1[8] = {0x11, 0xDA, 0x27, 0x00, 0xC5, 0x00, 0x00, 0xD7};
  const uint8_t frame2[8] = {0x11, 0xDA, 0x27, 0x00, 0x42, 0x00, 0x00, 0x54};

  uint8_t mode_byte = this->last_mode_;
  switch (this->mode) {
    case climate::CLIMATE_MODE_DRY:
      mode_byte = MODE_DRY;
      break;
    case climate::CLIMATE_MODE_FAN_ONLY:
      mode_byte = MODE_FAN;
      break;
    case climate::CLIMATE_MODE_COOL:
      mode_byte = MODE_COOL;
      break;
    case climate::CLIMATE_MODE_OFF:
    default:
      break;
  }
  if (this->mode != climate::CLIMATE_MODE_OFF) {
    this->last_mode_ = mode_byte;
  }

  uint8_t temperature;
  if (mode_byte == MODE_FAN) {
    // The remote sent 25 C when it entered fan-only mode.
    temperature = 0x32;
  } else {
    const float clamped = clamp(this->target_temperature, 10.0f, 30.0f);
    temperature = static_cast<uint8_t>(std::lround(clamped * 2.0f));
  }

  uint8_t fan = FAN_3;
  switch (this->fan_mode.value_or(climate::CLIMATE_FAN_MEDIUM)) {
    case climate::CLIMATE_FAN_QUIET:
      fan = FAN_QUIET;
      break;
    case climate::CLIMATE_FAN_LOW:
      fan = FAN_1;
      break;
    case climate::CLIMATE_FAN_MEDIUM:
      fan = FAN_3;
      break;
    case climate::CLIMATE_FAN_MIDDLE:
      fan = FAN_4;
      break;
    case climate::CLIMATE_FAN_HIGH:
      fan = FAN_5;
      break;
    case climate::CLIMATE_FAN_AUTO:
      fan = FAN_AUTO;
      break;
    default:
      break;
  }
  if (this->swing_mode == climate::CLIMATE_SWING_VERTICAL) {
    fan |= SWING_VERTICAL;
  }

  uint8_t frame3[19] = {0x11, 0xDA, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x06, 0x60, 0x00, 0x00, 0xC5, 0x00, 0x08, 0x00};
  frame3[5] = mode_byte | (this->mode == climate::CLIMATE_MODE_OFF ? 0x00 : POWER_ON);
  frame3[6] = temperature;
  frame3[8] = fan;
  // Power-off from cool set this byte to 0x40. Other modes were not captured off.
  frame3[16] = this->mode == climate::CLIMATE_MODE_OFF ? 0x40 : 0x00;

  uint8_t checksum = 0;
  for (int i = 0; i < 18; i++) {
    checksum += frame3[i];
  }
  frame3[18] = checksum;
  ESP_LOGI(TAG, "Frame %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X",
           frame3[0], frame3[1], frame3[2], frame3[3], frame3[4], frame3[5], frame3[6], frame3[7], frame3[8],
           frame3[9], frame3[10], frame3[11], frame3[12], frame3[13], frame3[14], frame3[15], frame3[16], frame3[17],
           frame3[18]);

  auto transmit = this->transmitter_->transmit();
  auto *data = transmit.get_data();
  data->set_carrier_frequency(CARRIER_FREQUENCY);

  // Six zero bits, then the 25 ms gap seen before the first header.
  for (int i = 0; i < 6; i++) {
    data->mark(BIT_MARK);
    data->space(ZERO_SPACE);
  }
  data->mark(BIT_MARK);
  data->space(PREAMBLE_GAP);

  send_frame(data, frame1, sizeof(frame1), FRAME_GAP);
  send_frame(data, frame2, sizeof(frame2), FRAME_GAP);
  send_frame(data, frame3, sizeof(frame3), ZERO_SPACE);

  transmit.perform();
}

}  // namespace esphome::daikin_ftkb
