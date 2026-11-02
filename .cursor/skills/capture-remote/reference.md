# ARC486A56 frame map

Confirmed on 2026-11-03 from `captures/arc486a56/`. One button press is one raw block, normally 585 pulses. Least-significant bit first. Checksum is the low byte of the sum of the earlier bytes in that frame.

Frames 1 and 2 stayed the same on every labeled press:

```
11 DA 27 00 C5 00 00 D7
11 DA 27 00 42 00 00 54
```

Frame 3 is 19 bytes. Bytes that changed:

| Byte | Meaning | Captured values |
|------|---------|-----------------|
| 5 bit 0 | Power | 1 on, 0 off |
| 5 bits 4-6 | Mode | Cool `0x30`, fan-only `0x60`. Dry is not captured. |
| 6 | Temperature | Degrees C times 2. `0x34` is 26.0, `0x35` is 26.5. Fan-only sent `0x32` (25 C). |
| 8 high nibble | Fan | Level 3 `0x5`, level 4 `0x6`, level 5 `0x7`, auto `0xA`, night/sleep `0xB`. Level 1 is not captured. |
| 8 low nibble | Swing | `0xF` on, `0x0` off |
| 16 | Power-off marker seen in cool | `0x40` on the cool power-off capture, `0x00` while on |
| 18 | Checksum | Changes by the same amount as the edited bytes |

Stored captures: `power-off`, `power-on`, `temp-26.5`, `fan-4`, `swing-off`, `mode-fan`, `fan-5`, `fan-auto`, `night`.
