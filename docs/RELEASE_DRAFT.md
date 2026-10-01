# WiFi Clock v2.0 — HC32 V15 / Wi-Fi R16

Release candidate for the newer Chouchin-899 with **HDSC HC32L130J8TA and
Taixin TXW813-320**. Not compatible with the older MM32/ESP board.

## Features

- Wi-Fi setup, saved-network display and manual SSID entry.
- Dual NTP servers: after five seconds both are listened to; first valid reply wins.
- Redirected DNS/NTP support, 40 timezones, custom POSIX rules and live DST preview.
- Daily updates in ten-minute steps and additional DST waking.
- Sweep, Burst and Hold&Start movement; second hand battery saver Off/Night parking/On.
- Browser keep-alive in AP and station modes.

Hold&Start replicates the characteristic Swiss railway station-clock movement.
This is an unofficial project, without manufacturer or railway endorsement.

## Downloads and installation

Download the Wi-Fi R16 APP, HC32 V15 HEX/BIN, three-page manual, SHA256SUMS and
companion notices. **Back up both chips before writing.** Build the TXW full image
using your own board's factory backup; no universal FULL image is supplied.
Use the chip-specific guides, not ESP/MM32 flashing tools. TXW scripts send one
write command without automatic retry or readback; keep power steady.

[Getting started](https://github.com/maddenste/Chouchin-899-latest-revision/blob/v2.0-rc1/docs/GETTING_STARTED.md) ·
[TXW flashing](https://github.com/maddenste/Chouchin-899-latest-revision/blob/v2.0-rc1/docs/FLASHING.md) ·
[HC32 installation](https://github.com/maddenste/Chouchin-899-latest-revision/blob/v2.0-rc1/docs/HC32.md)

## Validation and source scope

Movement modes, parking, indefinite battery saving, 17:10 daily waking and
forward/backward DST were owner-confirmed; the test ledger identifies revisions.
Non-hourly/Disabled DST boundaries, a 48-hour soak and genuine low-battery
measurements remain unconfirmed. Wi-Fi installation was programmer-reported,
without independent flash readback.

Published application/UI source, tests and helpers omit vendor startup/build
integration: this is **not a complete reproducible cross-build tree**.
HC32 V15 patches the original board firmware, rather than replacing it entirely.

[Older MM32/ESP board project](https://github.com/maddenste/Chouchin-CH899-Firmware)
