#include "casper_ykr.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include <driver/uart.h>

namespace esphome::casper_ykr {

static const char *const TAG = "casper_ykr";

static void send_byte(remote_base::RemoteTransmitData *data, uint8_t value) {
  for (uint8_t mask = 1; mask != 0; mask <<= 1) {
    data->mark(BIT_MARK);
    data->space((value & mask) != 0 ? ONE_SPACE : ZERO_SPACE);
  }
}

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
    case climate::CLIMATE_FAN_AUTO:
      return "auto";
    case climate::CLIMATE_FAN_LOW:
      return "low";
    case climate::CLIMATE_FAN_MEDIUM:
      return "medium";
    case climate::CLIMATE_FAN_HIGH:
      return "high";
    default:
      return "unknown";
  }
}

void CasperYkrClimate::setup() {
  climate_ir::ClimateIR::setup();
  ESP_LOGI(TAG, "UART control ready. Type 'help' and press Enter.");
  ESP_LOGI(TAG, "YKR-C/001 frame is unconfirmed. It follows ELECTRA_AC except byte 11, which is the YKR key code.");
}

void CasperYkrClimate::loop() {
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

void CasperYkrClimate::handle_line_(char *line) {
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
    ESP_LOGI(TAG, "  temp 26");
    ESP_LOGI(TAG, "  fan auto|low|medium|high");
    ESP_LOGI(TAG, "  swing on|off");
    ESP_LOGI(TAG, "  test");
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
  if (strcmp(line, "status") == 0) {
    const char *fan = this->fan_mode.has_value() ? fan_name(*this->fan_mode) : "none";
    const char *swing = this->swing_mode == climate::CLIMATE_SWING_VERTICAL ? "on" : "off";
    ESP_LOGI(TAG, "mode=%s temp=%.0f fan=%s swing=%s", mode_name(this->mode), this->target_temperature, fan, swing);
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
    if (strcmp(arg, "auto") == 0) {
      fan = climate::CLIMATE_FAN_AUTO;
    } else if (strcmp(arg, "low") == 0) {
      fan = climate::CLIMATE_FAN_LOW;
    } else if (strcmp(arg, "medium") == 0) {
      fan = climate::CLIMATE_FAN_MEDIUM;
    } else if (strcmp(arg, "high") == 0) {
      fan = climate::CLIMATE_FAN_HIGH;
    } else {
      ESP_LOGI(TAG, "Fan must be auto, low, medium, or high");
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

static uint8_t fan_bits_of(climate::ClimateFanMode fan) {
  switch (fan) {
    case climate::CLIMATE_FAN_LOW:
      return FAN_LOW;
    case climate::CLIMATE_FAN_MEDIUM:
      return FAN_MEDIUM;
    case climate::CLIMATE_FAN_HIGH:
      return FAN_HIGH;
    case climate::CLIMATE_FAN_AUTO:
    default:
      return FAN_AUTO;
  }
}

static Op op_of(climate::ClimateMode mode) {
  switch (mode) {
    case climate::CLIMATE_MODE_DRY:
      return Op::Dry;
    case climate::CLIMATE_MODE_FAN_ONLY:
      return Op::Fan;
    case climate::CLIMATE_MODE_COOL:
      return Op::Cool;
    case climate::CLIMATE_MODE_OFF:
      return Op::Off;
    default:
      return Op::Other;
  }
}

void CasperYkrClimate::transmit_state() {
  const float clamped = clamp(this->target_temperature, 16.0f, 32.0f);
  const uint8_t temperature = static_cast<uint8_t>(std::lround(clamped));
  const uint8_t fan_bits = fan_bits_of(this->fan_mode.value_or(climate::CLIMATE_FAN_AUTO));
  const bool swing_vertical = this->swing_mode == climate::CLIMATE_SWING_VERTICAL;
  const Decision decision = encode(this->memory_, op_of(this->mode), temperature, fan_bits, swing_vertical);
  this->memory_ = decision.next;

  const uint8_t *frame = decision.frame;
  ESP_LOGI(TAG, "Frame %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", frame[0], frame[1], frame[2],
           frame[3], frame[4], frame[5], frame[6], frame[7], frame[8], frame[9], frame[10], frame[11], frame[12]);

  auto transmit = this->transmitter_->transmit();
  auto *data = transmit.get_data();
  data->set_carrier_frequency(CARRIER_FREQUENCY);
  data->mark(HEADER_MARK);
  data->space(HEADER_SPACE);
  for (uint8_t byte : decision.frame) {
    send_byte(data, byte);
  }
  data->mark(BIT_MARK);
  data->space(MESSAGE_GAP);
  transmit.perform();
}

}  // namespace esphome::casper_ykr
