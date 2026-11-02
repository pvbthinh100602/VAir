---
name: capture-remote
description: >-
  Capture, decode, and store one Daikin ARC486A56 button press from the VAir
  ESP32 USB log. Use when the user presses the air-conditioner remote, names a
  button or display state, or asks to capture, store, or compare an IR signal.
---

# Capture a remote button

Watch the live capture log, decode the new burst, and store it under `captures/arc486a56/`. Read [reference.md](reference.md) before explaining which byte changed.

## Listen

The capture image is `firmware/esphome/ir-capture.yaml`. The receiver is a VS1838B on GPIO35, inverted, with no pull-up. A ready log contains `IR capture ready` and `GPIO35 level=`.

The control image does not receive IR. If the open log says `UART control ready`, stop and tell the user to flash the capture image. Do not open a second reader on `/dev/ttyUSB0` while a log is already running.

Flash command, only when the port is free:

```bash
esphome run firmware/esphome/ir-capture.yaml --device /dev/ttyUSB0 --upload_speed 115200
```

Hold BOOT only if the upload stream stops. Stop ModemManager first if it has the port.

## Take one press

Ask for the display state, then one button. After the user says the press is done, read the terminal file that is running the log. Use the newest `Received Raw` block that was not stored yet. Pass `--after` with the timestamp of the previous stored burst so an older press is not reused.

```bash
python3 .cursor/skills/capture-remote/scripts/decode_capture.py \
  --log <terminal-file> \
  --after <HH:MM:SS.mmm> \
  --compare "<previous frame 3 hex>" \
  --write-raw captures/arc486a56/<name>.raw.txt
```

Store the press only when the output has 585 pulses, 3 frames, and `checksum=ok` on every frame. A short block is incomplete: ask for the same press again and do not write it.

`<name>` is lowercase and hyphenated, matching the existing files (`fan-5`, `fan-auto`, `night`).

## Write the note

Create `captures/arc486a56/<name>.txt` in this shape:

```text
Button: <button name>
Remote before press: <display state>
Remote after press: <what the user reported>
Captured: <YYYY-MM-DD local time from the log timestamp>
Source: USB log from firmware/esphome/ir-capture.yaml, receiver on GPIO35
Raw pulses: <name>.raw.txt (<count> pulses)

Decoded as standard 3-frame Daikin, least-significant bit first.

Frame 1 (8 bytes): <hex>
Frame 2 (8 bytes): <hex>
Frame 3 (19 bytes): <hex>

<Which earlier capture frames 1 and 2 match. Checksums match.>

Frame 3 differences from <previous capture>:
- Byte <n>: <old> -> <new>. <meaning from reference.md>
```

Reply with frame 3 and the bytes that changed. The capture proves what the remote sent. It does not prove the indoor unit accepted a later transmission.
