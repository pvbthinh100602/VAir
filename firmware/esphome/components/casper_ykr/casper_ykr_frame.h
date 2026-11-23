#pragma once

#include <stdint.h>

namespace esphome::casper_ykr {

// Timings from IRremoteESP8266 sendElectraAC (ir_Electra.cpp). The library
// marks the carrier frequency as a guess and the gap as a guess, and it
// marks the sender as untested on hardware.
static constexpr uint32_t HEADER_MARK = 9166;
static constexpr uint32_t HEADER_SPACE = 4470;
static constexpr uint32_t BIT_MARK = 646;
static constexpr uint32_t ONE_SPACE = 1647;
static constexpr uint32_t ZERO_SPACE = 547;
static constexpr uint32_t MESSAGE_GAP = 100000;
static constexpr uint32_t CARRIER_FREQUENCY = 38000;

// Mode field, top 3 bits of byte 6. Same codes as kElectraAcCool / Dry / Fan.
// Heat is kElectraAcHeat (0b100). This unit is cool-only, so heat is not sent.
static constexpr uint8_t MODE_COOL = 0b001;
static constexpr uint8_t MODE_DRY = 0b010;
static constexpr uint8_t MODE_FAN = 0b110;

// Fan field, top 3 bits of byte 4. Same codes as kElectraAcFan*.
static constexpr uint8_t FAN_HIGH = 0b001;
static constexpr uint8_t FAN_MEDIUM = 0b010;
static constexpr uint8_t FAN_LOW = 0b011;
static constexpr uint8_t FAN_AUTO = 0b101;

// Vertical vane, low 3 bits of byte 1.
// ELECTRA_AC uses 0 for swing and 7 for swing off. A YKR-T/011 capture
// labeled swing-off uses 7 (byte 1 values 0xC7 and 0x47). Value 5 is a
// fixed blow-down position on that remote, not the swing-off code.
static constexpr uint8_t SWING_V_ON = 0b000;
static constexpr uint8_t SWING_V_OFF = 0b111;

// Horizontal swing off: top 3 bits of byte 2 are kElectraAcSwingOff.
// The climate entity does not offer left/right swing, so this stays off.
static constexpr uint8_t SWING_H_OFF = 0xE0;

static constexpr uint8_t POWER_ON = 0x20;

// Byte 11 is the key that changed on YKR-T remotes.
// ELECTRA_AC stores a light-toggle code here instead (0x08 off, 0x15 on).
// Its encoder does not emit these key codes. YKR-T/011 captures use 0x40,
// 0x41, 0x44, and 0x45, and a frame whose byte 11 is 0x08 was reported not
// to control that remote. YKR-C/001 has not been captured. 0x42 is the
// vertical-swing key used here; it was not in those published tables.
static constexpr uint8_t BUTTON_POWER = 0x45;
static constexpr uint8_t BUTTON_TEMP_UP = 0x40;
static constexpr uint8_t BUTTON_TEMP_DOWN = 0x41;
static constexpr uint8_t BUTTON_FAN = 0x44;
static constexpr uint8_t BUTTON_SWING_V = 0x42;

enum class Op : uint8_t { Off = 0, Cool = 1, Dry = 2, Fan = 3, Other = 4 };

struct Memory {
  uint8_t mode_bits{MODE_COOL};
  uint8_t temperature{26};
  uint8_t fan_bits{FAN_AUTO};
  bool swing_vertical{false};
  Op op{Op::Off};
};

struct Decision {
  uint8_t frame[13];
  uint8_t button;
  Memory next;
};

inline uint8_t mode_bits_for(Op op, uint8_t previous_mode_bits) {
  switch (op) {
    case Op::Dry:
      return MODE_DRY;
    case Op::Fan:
      return MODE_FAN;
    case Op::Cool:
      return MODE_COOL;
    case Op::Off:
    case Op::Other:
    default:
      return previous_mode_bits;
  }
}

inline uint8_t select_button(bool mode_or_power_changed, uint8_t temperature, uint8_t previous_temperature,
                             bool fan_changed, bool swing_changed) {
  if (mode_or_power_changed) {
    return BUTTON_POWER;
  }
  if (temperature > previous_temperature) {
    return BUTTON_TEMP_UP;
  }
  if (temperature < previous_temperature) {
    return BUTTON_TEMP_DOWN;
  }
  if (fan_changed) {
    return BUTTON_FAN;
  }
  if (swing_changed) {
    return BUTTON_SWING_V;
  }
  return BUTTON_POWER;
}

inline void write_frame(bool power_on, uint8_t mode_bits, uint8_t temperature, uint8_t fan_bits, bool swing_vertical,
                        uint8_t button, uint8_t out[13]) {
  if (temperature < 16) {
    temperature = 16;
  } else if (temperature > 32) {
    temperature = 32;
  }
  const uint8_t vane = swing_vertical ? SWING_V_ON : SWING_V_OFF;
  out[0] = 0xC3;
  out[1] = static_cast<uint8_t>(((temperature - 8) << 3) | vane);
  out[2] = SWING_H_OFF;
  out[3] = 0x00;
  out[4] = static_cast<uint8_t>(fan_bits << 5);
  out[5] = 0x00;
  out[6] = static_cast<uint8_t>(mode_bits << 5);
  out[7] = 0x00;
  out[8] = 0x00;
  out[9] = power_on ? POWER_ON : 0x00;
  out[10] = 0x00;
  out[11] = button;
  uint8_t checksum = 0;
  for (int i = 0; i < 12; i++) {
    checksum = static_cast<uint8_t>(checksum + out[i]);
  }
  out[12] = checksum;
}

// prev is the state after the previous transmission. op/temperature/fan_bits/
// swing_vertical are the state about to be sent.
inline Decision encode(const Memory &prev, Op op, uint8_t temperature, uint8_t fan_bits, bool swing_vertical) {
  Decision decision;
  const uint8_t mode_bits = mode_bits_for(op, prev.mode_bits);
  const bool mode_or_power_changed = (op != prev.op) || (mode_bits != prev.mode_bits);
  decision.button = select_button(mode_or_power_changed, temperature, prev.temperature, fan_bits != prev.fan_bits,
                                  swing_vertical != prev.swing_vertical);
  write_frame(op != Op::Off, mode_bits, temperature, fan_bits, swing_vertical, decision.button, decision.frame);
  decision.next = prev;
  decision.next.temperature = temperature;
  decision.next.fan_bits = fan_bits;
  decision.next.swing_vertical = swing_vertical;
  decision.next.op = op;
  if (op != Op::Off) {
    decision.next.mode_bits = mode_bits;
  }
  return decision;
}

}  // namespace esphome::casper_ykr
