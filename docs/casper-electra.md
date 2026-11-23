# Casper YKR-C/001 compared with ELECTRA_AC

The Casper sender in `firmware/esphome/components/casper_ykr/` is still unconfirmed. YKR-C/001 is not listed by IRremoteESP8266, no capture from this remote is in the repository, and the indoor unit has not been tested. This note records how the sender compares with `IRElectraAc` (`src/ir_Electra.cpp` and `src/ir_Electra.h` in [IRremoteESP8266](https://github.com/crankyoldgit/IRremoteESP8266)), and which difference was changed.

`ELECTRA_AC` is the library's name for the 104-bit / 13-byte, 38 kHz frame used by the YKR remote family it has seen: AUX YKR-T/011, Centek YKR-P/002E, Electra YKR-M/003E, Electrolux YKR-H/531E, and Subtropic YKR-H/102E. The library marks `sendElectraAC` as alpha and the 38 kHz carrier as a guess. The message gap is `kDefaultMessageGap` (100000 us), also marked as a guess.

## What matches

| Item | ELECTRA_AC | casper_ykr |
| --- | --- | --- |
| Carrier | 38000 Hz, 50% duty | 38000 Hz, 50% duty in the YAML |
| Header | mark 9166, space 4470 | same |
| Bit | mark 646; one-space 1647; zero-space 547 | same |
| Trailer | mark 646, then space 100000 | same |
| Bit order | least-significant bit first in each byte | same |
| Length | 13 bytes, one transmission | same |
| Byte 0 | `0xC3` | `0xC3` |
| Byte 1 temperature | bits 3-7 hold degrees C minus 8, range 16-32 | same |
| Byte 1 swing on | `0b000` | `0b000` |
| Byte 2 swing horizontal | off is `0xE0` (`0b111` in bits 5-7) | always `0xE0` |
| Byte 3 | 0 unless bit 6, the sensor-update flag | 0 |
| Byte 4 fan | bits 5-7: high `001`, medium `010`, low `011`, auto `101` | same |
| Byte 5 | bit 6 turbo, bit 7 quiet | 0 |
| Byte 6 mode | bits 5-7: cool `001`, dry `010`, heat `100`, fan `110`; bit 3 is iFeel | cool, dry, and fan match; iFeel stays 0; heat is not sent |
| Byte 7 | sensor temperature, 0 when iFeel is off | 0 |
| Byte 8 | 0 | 0 |
| Byte 9 | bit 5 power (`0x20` on); bit 2 clean | power matches; clean stays 0 |
| Byte 10 | 0 | 0 |
| Byte 12 | low 8 bits of the sum of bytes 0-11 | same |

Left/right swing, iFeel, iClean, turbo, quiet, and the display light are not controls on this climate entity. Their Electra bits stay at the off value. The unit advertises iFeel and iClean; those bits are unconfirmed for YKR-C/001. Heat exists in ELECTRA_AC as `0b100` and is omitted here because the TC-09IS35 is cool-only.

## What was different

| Item | ELECTRA_AC | casper_ykr before | casper_ykr now |
| --- | --- | --- | --- |
| Byte 1 swing off | `0b111` | `0b101` | `0b111` |
| Byte 11 | light toggle: `0x08` off, `0x15` on | YKR key code `0x45` / `0x40` / `0x41` / `0x44` / `0x42` | unchanged |

Swing-off was aligned with ELECTRA_AC. `0b101` is a fixed blow-down position in a later YKR-T/011 table, not the swing-off code. The library's own YKR-T/011 samples labeled "swing vertical: off" use `0b111` (byte 1 `0xC7` at 32 C and `0x47` at 16 C, issue 778). No YKR-C/001 capture says otherwise.

Byte 11 was kept. ELECTRA_AC calls that byte a light toggle, and `stateReset()` stores `0x08`. The same YKR-T/011 captures store the key that changed: `0x40` temperature up, `0x41` temperature down, `0x44` fan, `0x45` power. A 2026 report on that remote (issue 2272) says the library frame, whose byte 11 is `0x08`, does not control the unit, while frames with a key code do. The library's own constructor test had to rewrite a captured byte 11 before the bytes would match the encoder. `0x42` is still the vertical-swing key used by this sender; it was not in those two published tables.

A mode change reuses the power key `0x45`. No separate mode-key code was in the YKR-T tables used here.

## Host diff

`python3 scripts/compare_casper_electra.py` compiles `scripts/electra_ac_host.cc`. That program builds Electra bytes with the same bitfield order as `ir_Electra.h`, builds Casper bytes with `casper_ykr_frame.h` (the header the firmware calls), and builds the `sendElectraAC` pulse string.

The pulse string matches `TestSendElectraAC.SendDataOnly` in `test/ir_Electra_test.cpp` (`f38000d50`, header `m9166s4470`, LSB bits, trailer `m646s100000`). Replacing Casper byte 11 with Electra's `0x08` and recomputing the checksum makes the two pulse strings identical. The timings do not differ.

These states differ only in byte 11 and the checksum:

| State | Casper | Electra |
| --- | --- | --- |
| Power on, cool 26, fan auto, swing off | `C3 97 E0 00 A0 00 20 00 00 20 00 45 5F` | `C3 97 E0 00 A0 00 20 00 00 20 00 08 22` |
| Cool 32, fan auto, swing off, temp up | `C3 C7 E0 00 A0 00 20 00 00 20 00 40 8A` | `C3 C7 E0 00 A0 00 20 00 00 20 00 08 52` |
| Cool 16, fan auto, swing off, temp down | `C3 47 E0 00 A0 00 20 00 00 20 00 41 0B` | `C3 47 E0 00 A0 00 20 00 00 20 00 08 D2` |
| Cool 16, fan low, swing off, fan key | `C3 47 E0 00 60 00 20 00 00 20 00 44 CE` | `C3 47 E0 00 60 00 20 00 00 20 00 08 92` |
| Cool 16, fan low, swing on | `C3 40 E0 00 60 00 20 00 00 20 00 42 C5` | `C3 40 E0 00 60 00 20 00 00 20 00 08 8B` |
| Dry 24, fan medium, swing on | `C3 80 E0 00 40 00 40 00 00 20 00 45 08` | `C3 80 E0 00 40 00 40 00 00 20 00 08 CB` |
| Fan-only 30, fan high, swing off | `C3 B7 E0 00 20 00 C0 00 00 20 00 45 9F` | `C3 B7 E0 00 20 00 C0 00 00 20 00 08 62` |
| Power off, fan-only settings kept | `C3 B7 E0 00 20 00 C0 00 00 00 00 45 7F` | `C3 B7 E0 00 20 00 C0 00 00 00 00 08 42` |

The 32 C and 16 C rows are the issue 778 YKR-T/011 captures, including the checksum. Cool 16, fan low, swing off, temperature-down is the third capture from that issue: `C3 47 E0 00 60 00 20 00 00 20 00 41 CB`.

Published YKR-T/011 raw timings sit near 9000 / 4500 / 550 / 1710 / 600 rather than the library's 9166 / 4470 / 646 / 1647 / 547. The sender keeps the library constants. The library decoder accepts both.

## ESPHome `electra` climate

[mistic100/ESPHome-IRremoteESP8266](https://github.com/mistic100/ESPHome-IRremoteESP8266) wraps `IRElectraAc` and sends it with the same header, bit timings, LSB order, and trailer. It was not swapped in.

- Its byte 11 stays at Electra's light-off value `0x08`. That is the byte the YKR-T/011 report says does not control the unit.
- `CLIMATE_MODE_AUTO` is passed to `setMode()` as `kElectraAcFanAuto` (`0b101`). That value is not a mode, so the library falls through to auto (`0b000`).
- The USB command parser in `casper-tc09is35.yaml` would have to be rewritten.
- The component tracks the library's default branch unless a git ref is pinned.

The local component is the smaller one that still builds and still emits the key byte those captures contain.

## Confidence

Low for YKR-C/001 itself. The corrected swing-off field and the kept key byte match published YKR-T/011 frames, and the rest of the frame matches `IRElectraAc`. That is evidence about a related remote, not about this one. Capture one button on the real YKR-C/001 with `ir-capture.yaml` before treating the frame as confirmed.
