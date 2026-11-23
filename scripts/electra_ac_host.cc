// Host-side comparison of the Casper YKR frame with IRremoteESP8266 ELECTRA_AC.
//
// The Electra union follows the field order in ir_Electra.h. This file does
// not link that library. Bitfield allocation is whatever this GCC uses; the
// layout checks below fail if that allocation does not match the published
// Electra bytes. ESP32 firmware is also built with GCC, little-endian.
//
// Pulse timings are the constants in sendElectraAC. The expected pulse string
// is the one in test/ir_Electra_test.cpp, TestSendElectraAC.SendDataOnly.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

#include "casper_ykr_frame.h"

namespace {

constexpr int kLength = 13;

// Field order matches crankyoldgit/IRremoteESP8266 src/ir_Electra.h.
union ElectraProtocol {
  uint8_t raw[kLength];
  struct {
    uint8_t : 8;
    uint8_t SwingV : 3;
    uint8_t Temp : 5;
    uint8_t : 5;
    uint8_t SwingH : 3;
    uint8_t : 6;
    uint8_t SensorUpdate : 1;
    uint8_t : 1;
    uint8_t : 5;
    uint8_t Fan : 3;
    uint8_t : 6;
    uint8_t Turbo : 1;
    uint8_t Quiet : 1;
    uint8_t : 3;
    uint8_t IFeel : 1;
    uint8_t : 1;
    uint8_t Mode : 3;
    uint8_t SensorTemp : 8;
    uint8_t : 8;
    uint8_t : 2;
    uint8_t Clean : 1;
    uint8_t : 2;
    uint8_t Power : 1;
    uint8_t : 2;
    uint8_t : 8;
    uint8_t LightToggle : 8;
    uint8_t Sum : 8;
  };
};

constexpr uint8_t kSwingOn = 0b000;
constexpr uint8_t kSwingOff = 0b111;
constexpr uint8_t kFanAuto = 0b101;
constexpr uint8_t kFanLow = 0b011;
constexpr uint8_t kFanMed = 0b010;
constexpr uint8_t kFanHigh = 0b001;
constexpr uint8_t kCool = 0b001;
constexpr uint8_t kDry = 0b010;
constexpr uint8_t kHeat = 0b100;
constexpr uint8_t kFanMode = 0b110;
constexpr uint8_t kLightOff = 0x08;
constexpr uint8_t kLightOn = 0x15;

int g_failures = 0;

void fail(const std::string &message) {
  std::cerr << "FAIL " << message << "\n";
  g_failures++;
}

std::string hex_bytes(const uint8_t *data, int length = kLength) {
  std::string out;
  char buf[4];
  for (int i = 0; i < length; i++) {
    if (i != 0) {
      out += ' ';
    }
    std::snprintf(buf, sizeof(buf), "%02X", data[i]);
    out += buf;
  }
  return out;
}

void state_reset(ElectraProtocol *state) {
  for (int i = 1; i < kLength - 2; i++) {
    state->raw[i] = 0;
  }
  state->raw[0] = 0xC3;
  state->LightToggle = kLightOff;
}

void checksum(ElectraProtocol *state) {
  uint8_t sum = 0;
  for (int i = 0; i < kLength - 1; i++) {
    sum = static_cast<uint8_t>(sum + state->raw[i]);
  }
  state->Sum = sum;
}

void set_power(ElectraProtocol *state, bool on) { state->Power = on ? 1 : 0; }

void set_mode(ElectraProtocol *state, uint8_t mode) { state->Mode = mode; }

void set_temp(ElectraProtocol *state, uint8_t temp) {
  if (temp < 16) {
    temp = 16;
  } else if (temp > 32) {
    temp = 32;
  }
  state->Temp = static_cast<uint8_t>(temp - 8);
}

void set_fan(ElectraProtocol *state, uint8_t fan) { state->Fan = fan; }

void set_swing_v(ElectraProtocol *state, bool on) { state->SwingV = on ? kSwingOn : kSwingOff; }

void set_swing_h(ElectraProtocol *state, bool on) { state->SwingH = on ? kSwingOn : kSwingOff; }

ElectraProtocol electra_state(bool power, uint8_t mode, uint8_t temp, uint8_t fan, bool swing_v) {
  ElectraProtocol state{};
  state_reset(&state);
  set_power(&state, power);
  set_mode(&state, mode);
  set_temp(&state, temp);
  set_fan(&state, fan);
  set_swing_v(&state, swing_v);
  set_swing_h(&state, false);
  checksum(&state);
  return state;
}

std::string pulses(const uint8_t *data, int length = kLength) {
  std::string out = "f38000d50m9166s4470";
  for (int i = 0; i < length; i++) {
    for (int bit = 0; bit < 8; bit++) {
      const bool one = ((data[i] >> bit) & 0x01) != 0;
      out += "m646s";
      out += one ? "1647" : "547";
    }
  }
  out += "m646s100000";
  return out;
}

void expect_bytes(const char *name, const uint8_t *actual, const uint8_t *expected) {
  if (std::memcmp(actual, expected, kLength) != 0) {
    fail(std::string(name) + " bytes\n  actual   " + hex_bytes(actual) + "\n  expected " + hex_bytes(expected));
  } else {
    std::cout << "MATCH " << name << " " << hex_bytes(actual) << "\n";
  }
}

void expect_electra_diff_only_key(const char *name, const uint8_t *casper, const uint8_t *electra) {
  bool data_mismatch = false;
  for (int i = 0; i < 11; i++) {
    if (casper[i] != electra[i]) {
      data_mismatch = true;
      fail(std::string(name) + " byte " + std::to_string(i) + " casper=" + hex_bytes(casper + i, 1) +
           " electra=" + hex_bytes(electra + i, 1));
    }
  }
  if (casper[11] == electra[11]) {
    fail(std::string(name) + " byte 11 unexpectedly matches Electra light byte " + hex_bytes(electra + 11, 1));
  }
  if (!data_mismatch && casper[11] != electra[11]) {
    std::cout << "DIFF  " << name << " byte 11 casper=" << hex_bytes(casper + 11, 1)
              << " electra=" << hex_bytes(electra + 11, 1) << " (key code kept; light byte is Electra's)\n";
    std::cout << "      casper  " << hex_bytes(casper) << "\n";
    std::cout << "      electra " << hex_bytes(electra) << "\n";
  }
}

void check_layout() {
  if (sizeof(ElectraProtocol) != kLength) {
    fail("ElectraProtocol is " + std::to_string(sizeof(ElectraProtocol)) + " bytes, expected 13");
    return;
  }
  ElectraProtocol state{};
  state_reset(&state);
  set_temp(&state, 26);
  set_swing_v(&state, false);
  if (state.raw[1] != 0x97) {
    fail("temp 26 swing-off packed as " + hex_bytes(state.raw + 1, 1) + ", expected 97");
  }
  set_swing_v(&state, true);
  if (state.raw[1] != 0x90) {
    fail("temp 26 swing-on packed as " + hex_bytes(state.raw + 1, 1) + ", expected 90");
  }
  set_swing_h(&state, false);
  if (state.raw[2] != 0xE0) {
    fail("swing-h off packed as " + hex_bytes(state.raw + 2, 1) + ", expected E0");
  }
  set_fan(&state, kFanLow);
  if (state.raw[4] != 0x60) {
    fail("fan low packed as " + hex_bytes(state.raw + 4, 1) + ", expected 60");
  }
  state.Turbo = 1;
  state.Quiet = 1;
  if (state.raw[5] != 0xC0) {
    fail("turbo+quiet packed as " + hex_bytes(state.raw + 5, 1) + ", expected C0");
  }
  set_mode(&state, kCool);
  state.IFeel = 1;
  if (state.raw[6] != 0x28) {
    fail("cool+iFeel packed as " + hex_bytes(state.raw + 6, 1) + ", expected 28");
  }
  state.SensorUpdate = 1;
  if (state.raw[3] != 0x40) {
    fail("sensor-update packed as " + hex_bytes(state.raw + 3, 1) + ", expected 40");
  }
  set_power(&state, true);
  state.Clean = 1;
  if (state.raw[9] != 0x24) {
    fail("power+clean packed as " + hex_bytes(state.raw + 9, 1) + ", expected 24");
  }
  state.LightToggle = kLightOn;
  if (state.raw[11] != kLightOn) {
    fail("light toggle packed as " + hex_bytes(state.raw + 11, 1) + ", expected 15");
  }
  set_mode(&state, kHeat);
  if ((state.raw[6] & 0xE0) != (kHeat << 5)) {
    fail("heat mode packed as " + hex_bytes(state.raw + 6, 1));
  }
  if (g_failures == 0) {
    std::cout << "MATCH Electra bitfield layout (swing, temp, fan, mode, iFeel, clean, power, light)\n";
  }
}

void check_library_pulses() {
  const uint8_t data[kLength] = {0xC3, 0x87, 0xF6, 0x28, 0x60, 0x00, 0x20, 0x00, 0x00, 0x20, 0x00, 0x05, 0x0D};
  const char *expected =
      "f38000d50"
      "m9166s4470"
      "m646s1647m646s1647m646s547m646s547m646s547m646s547m646s1647m646s1647"
      "m646s1647m646s1647m646s1647m646s547m646s547m646s547m646s547m646s1647"
      "m646s547m646s1647m646s1647m646s547m646s1647m646s1647m646s1647m646s1647"
      "m646s547m646s547m646s547m646s1647m646s547m646s1647m646s547m646s547"
      "m646s547m646s547m646s547m646s547m646s547m646s1647m646s1647m646s547"
      "m646s547m646s547m646s547m646s547m646s547m646s547m646s547m646s547"
      "m646s547m646s547m646s547m646s547m646s547m646s1647m646s547m646s547"
      "m646s547m646s547m646s547m646s547m646s547m646s547m646s547m646s547"
      "m646s547m646s547m646s547m646s547m646s547m646s547m646s547m646s547"
      "m646s547m646s547m646s547m646s547m646s547m646s1647m646s547m646s547"
      "m646s547m646s547m646s547m646s547m646s547m646s547m646s547m646s547"
      "m646s1647m646s547m646s1647m646s547m646s547m646s547m646s547m646s547"
      "m646s1647m646s547m646s1647m646s1647m646s547m646s547m646s547m646s547"
      "m646s100000";
  const std::string actual = pulses(data);
  if (actual != expected) {
    fail("sendElectraAC pulse string does not match ir_Electra_test.cpp SendDataOnly");
    std::cerr << "  actual length " << actual.size() << " expected length " << std::strlen(expected) << "\n";
    const size_t n = std::min(actual.size(), std::strlen(expected));
    for (size_t i = 0; i < n; i++) {
      if (actual[i] != expected[i]) {
        std::cerr << "  first difference at " << i << "\n";
        break;
      }
    }
  } else {
    std::cout << "MATCH sendElectraAC pulses for the library SendDataOnly vector (" << actual.size()
              << " chars)\n";
  }
}

void check_captures_and_states() {
  using esphome::casper_ykr::BUTTON_FAN;
  using esphome::casper_ykr::BUTTON_POWER;
  using esphome::casper_ykr::BUTTON_SWING_V;
  using esphome::casper_ykr::BUTTON_TEMP_DOWN;
  using esphome::casper_ykr::BUTTON_TEMP_UP;
  using esphome::casper_ykr::Decision;
  using esphome::casper_ykr::encode;
  using esphome::casper_ykr::FAN_AUTO;
  using esphome::casper_ykr::FAN_HIGH;
  using esphome::casper_ykr::FAN_LOW;
  using esphome::casper_ykr::FAN_MEDIUM;
  using esphome::casper_ykr::Memory;
  using esphome::casper_ykr::MODE_COOL;
  using esphome::casper_ykr::MODE_DRY;
  using esphome::casper_ykr::MODE_FAN;
  using esphome::casper_ykr::Op;

  Memory memory{};
  const Decision power_on = encode(memory, Op::Cool, 26, FAN_AUTO, false);
  memory = power_on.next;
  const Decision temp_32 = encode(memory, Op::Cool, 32, FAN_AUTO, false);
  memory = temp_32.next;
  const Decision temp_16 = encode(memory, Op::Cool, 16, FAN_AUTO, false);
  memory = temp_16.next;
  const Decision fan_low = encode(memory, Op::Cool, 16, FAN_LOW, false);
  memory = fan_low.next;
  const Decision swing_on = encode(memory, Op::Cool, 16, FAN_LOW, true);
  memory = swing_on.next;
  const Decision dry = encode(memory, Op::Dry, 24, FAN_MEDIUM, true);
  memory = dry.next;
  const Decision fan_only = encode(memory, Op::Fan, 30, FAN_HIGH, false);
  memory = fan_only.next;
  const Decision power_off = encode(memory, Op::Off, 30, FAN_HIGH, false);

  const uint8_t ykr_32_auto_up[kLength] = {0xC3, 0xC7, 0xE0, 0x00, 0xA0, 0x00, 0x20, 0x00, 0x00, 0x20, 0x00, 0x40, 0x8A};
  const uint8_t ykr_16_auto_down[kLength] = {0xC3, 0x47, 0xE0, 0x00, 0xA0, 0x00, 0x20,
                                             0x00, 0x00, 0x20, 0x00, 0x41, 0x0B};
  expect_bytes("YKR-T/011 cool 32 auto swing-off temp-up", temp_32.frame, ykr_32_auto_up);
  expect_bytes("YKR-T/011 cool 16 auto swing-off temp-down", temp_16.frame, ykr_16_auto_down);

  if (power_on.button != BUTTON_POWER) {
    fail("power-on button");
  }
  if (temp_32.button != BUTTON_TEMP_UP || temp_16.button != BUTTON_TEMP_DOWN) {
    fail("temperature key selection");
  }
  if (fan_low.button != BUTTON_FAN || swing_on.button != BUTTON_SWING_V) {
    fail("fan or swing key selection");
  }
  if (dry.button != BUTTON_POWER || power_off.button != BUTTON_POWER) {
    fail("mode or power-off key selection");
  }
  if ((temp_32.frame[1] & 0x07) != 0x07 || (swing_on.frame[1] & 0x07) != 0x00) {
    fail("swing-off is not 7 or swing-on is not 0");
  }

  struct Case {
    const char *name;
    const uint8_t *frame;
    bool power;
    uint8_t mode;
    uint8_t temp;
    uint8_t fan;
    bool swing_v;
  };
  const Case cases[] = {
      {"power on cool 26 auto swing off", power_on.frame, true, kCool, 26, kFanAuto, false},
      {"cool 32 auto swing off", temp_32.frame, true, kCool, 32, kFanAuto, false},
      {"cool 16 auto swing off", temp_16.frame, true, kCool, 16, kFanAuto, false},
      {"cool 16 low swing off", fan_low.frame, true, kCool, 16, kFanLow, false},
      {"cool 16 low swing on", swing_on.frame, true, kCool, 16, kFanLow, true},
      {"dry 24 medium swing on", dry.frame, true, kDry, 24, kFanMed, true},
      {"fan 30 high swing off", fan_only.frame, true, kFanMode, 30, kFanHigh, false},
      {"power off keeping fan mode", power_off.frame, false, kFanMode, 30, kFanHigh, false},
  };
  for (const Case &item : cases) {
    const ElectraProtocol electra = electra_state(item.power, item.mode, item.temp, item.fan, item.swing_v);
    expect_electra_diff_only_key(item.name, item.frame, electra.raw);
    uint8_t aligned[kLength];
    std::memcpy(aligned, item.frame, kLength);
    aligned[11] = electra.raw[11];
    uint8_t sum = 0;
    for (int i = 0; i < 12; i++) {
      sum = static_cast<uint8_t>(sum + aligned[i]);
    }
    aligned[12] = sum;
    if (std::memcmp(aligned, electra.raw, kLength) != 0) {
      fail(std::string(item.name) + " still differs after replacing byte 11");
    } else if (pulses(aligned) != pulses(electra.raw)) {
      fail(std::string(item.name) + " pulses differ for identical bytes");
    }
  }
  if (fan_only.frame[6] != (MODE_FAN << 5) || dry.frame[6] != (MODE_DRY << 5) ||
      power_on.frame[6] != (MODE_COOL << 5)) {
    fail("mode bits were not written");
  }
  uint8_t fan_low_temp_down[kLength];
  esphome::casper_ykr::write_frame(true, MODE_COOL, 16, FAN_LOW, false, BUTTON_TEMP_DOWN, fan_low_temp_down);
  const uint8_t ykr_16_low_down[kLength] = {0xC3, 0x47, 0xE0, 0x00, 0x60, 0x00, 0x20, 0x00, 0x00, 0x20, 0x00, 0x41, 0xCB};
  expect_bytes("YKR-T/011 cool 16 low swing-off temp-down", fan_low_temp_down, ykr_16_low_down);
}

}  // namespace

int main() {
  std::cout << "Casper YKR vs IRremoteESP8266 ELECTRA_AC\n";
  check_layout();
  check_library_pulses();
  check_captures_and_states();
  if (g_failures != 0) {
    std::cerr << g_failures << " failure(s)\n";
    return 1;
  }
  std::cout << "All host comparisons passed.\n";
  return 0;
}
