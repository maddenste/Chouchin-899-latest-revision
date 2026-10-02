# WiFi Clock v2.0 — HC32 V15 / Wi-Fi R16

For the newer **Chouchin-899 with HC32L130J8TA + TXW813-320** only.
Not compatible with the older MM32/ESP board.

## Downloads

- **HC32 V15 HEX** — complete 64 KiB modified movement firmware.
- **Wi-Fi R16 FULL BIN** — complete 2 MiB clean-install Wi-Fi firmware.
- **User manual** — setup and operation.
- **SHA256SUMS.txt** — checksums for these three downloads.

Only our modified firmware is supplied. No original firmware, private backups
or saved Wi-Fi credentials are included.

## Flashing

Back up your board first and keep power steady. Update only the chip that needs it.

**HC32:** load the V15 HEX in XHSC MCU Programmer V2.23. Select
HC32L13x8/HC32F030x8, 115200 baud and your COM port. Use Chip Erase with
Erase, Program and Verify checked, Encrypt unchecked. BOOT must be held at
3.3 V during programming. After success, power off, disconnect BOOT and restart.
[Wiring and full HC32 instructions](https://github.com/maddenste/Chouchin-899-latest-revision/blob/main/docs/HC32.md)

**Wi-Fi:** write the FULL BIN to TXW flash at **0x000000**, replacing the entire
2 MiB. No image-building or settings-copy helper is needed.
[TXW wiring, tools and power-on catch instructions](https://github.com/maddenste/Chouchin-899-latest-revision/blob/main/docs/FLASHING.md)

**After Wi-Fi flashing, settings are empty.** Restart, join **WiFi-Clock-Setup**,
open **http://192.168.4.1/** and enter your Wi-Fi and clock settings.

## Changelog

- HC32 V15: movement/battery-saver selectors, indefinite second-hand parking,
  minute-precision waking and overnight restart fix; battery checks retained.
- Wi-Fi R16: dual NTP, redirected DNS/NTP, timezone/POSIX DST, live DST preview,
  daily wake in ten-minute steps, movement modes and browser keep-alive.
  Footer: WiFi Clock · v2.0 — Modified by Steve Madden · 2026.
- 2 October: clean full-image download and simpler flashing documentation.
  Firmware code is unchanged.

The clean FULL's layout and installer preflight are checked offline; blanking the
old configuration sectors has not yet been hardware-tested. Existing movement,
parking, daily-wake and forward/backward DST tests are recorded in the repository.
