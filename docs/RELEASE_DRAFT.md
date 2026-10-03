# WiFi Clock v2.0 — HC32 V15 / Wi-Fi R16

For **Chouchin-899 with HC32L130J8TA + TXW813-320** only.
Not compatible with the older MM32/ESP board.

## Install

**[Start here: equipment, software downloads and step-by-step upload guide](https://github.com/maddenste/Chouchin-899-latest-revision/blob/main/docs/GETTING_STARTED.md)**

We used Windows and a **3.3 V bench supply connected to the battery terminals**.
Install the prepared files using the documented UART and CKLink procedures; no compiling is required.

After Wi-Fi flashing, enter your settings: join **WiFi-Clock-Setup**, open **http://192.168.4.1/** and select **Save settings**.

## Downloads

- **Chouchin-899-HC32-V15-20261001.hex** — complete 64 KiB movement firmware.
- **WiFi-Clock-v2.0-R16-20261002_FULL.bin** — complete 2 MiB clean Wi-Fi image.
- **WiFi-Clock-User-Manual-v2.0.pdf** — setup and operation.
- **SHA256SUMS.txt** — separate checksums.

No original firmware or saved Wi-Fi credentials are included.

## Changelog

- HC32 V15: movement/battery-saver selection, indefinite second-hand parking, minute-precision waking and overnight restart fix; original battery checks retained.
- Wi-Fi R16: dual NTP, redirected DNS/NTP support, timezone/POSIX DST, live DST preview, ten-minute daily-wake settings and browser keep-alive.
- Footer: WiFi Clock · v2.0 — Modified by Steve Madden · 2026.
- Documentation reorganised into a simple upload walkthrough with software links and wiring diagrams. Firmware code unchanged.

The clean FULL's layout and installer preflight are checked offline; its blank old-configuration sectors have not yet been hardware-tested. [Test record](https://github.com/maddenste/Chouchin-899-latest-revision/blob/main/docs/TESTING.md).
