# Chouchin-CH899 · WiFi Clock v2.0

![Chouchin-CH899 WiFi Clock v2.0 — HC32L130 and TXW813](docs/assets/social-preview.jpg)

Wi-Fi time synchronisation, configurable hand movement and a simple local settings page.

**Check inside your clock first:** this upgrade is for the **HC32 board**. CH899 clocks can look identical outside but have different electronics. [Compare your board with the identification photos](docs/HARDWARE.md#board-photographs) before installing.

<img src="docs/photos/chouchin-899-newer-board-overview.jpg" alt="Newer Chouchin-CH899 board" width="350">

[Example Chouchin-CH899 Wi-Fi clock movement](https://www.hr-clockparts.com/clock-movement/wifi-clock-movement.html) — for identification only, not a confirmed compatible purchase. The listing does not identify the board revision; check for **HC32L130J8TA + TXW813-320** before buying for this project.

## Is your clock the MM32 version?

If your board has an MM32 chip and an ESP-01F Wi-Fi module, choose one of these projects instead. [See the MM32 identification photos](https://github.com/maddenste/Chouchin-CH899-MM32-ESP8285/blob/main/docs/HARDWARE.md) if you are unsure.

- **[Want movement styles and seconds-hand parking? Choose WiFi Clock v1.5](https://github.com/maddenste/Chouchin-CH899-MM32-ESP8285)** — replaces both the hand-movement and Wi-Fi firmware on the MM32 board.
- **[Want to keep the factory hand movement? Choose the Wi-Fi-only upgrade](https://github.com/maddenste/Chouchin-CH899-Firmware)** — changes the Wi-Fi setup and internet time synchronisation, leaving the movement firmware untouched.

**v1.5 and v2.0 are for different boards.** The version number does not make v2.0 suitable for an MM32 clock. Follow your chosen project's installation guide and use its firmware files together.

## Ready to install?

Download **[Chouchin-899-HC32-V21.hex](https://github.com/maddenste/Chouchin-899-latest-revision/releases/download/v21/Chouchin-899-HC32-V21.hex)** for the **HC32 movement controller** and **[WiFi-Clock-v2.0-R17-20261003_FULL.bin](https://github.com/maddenste/Chouchin-899-latest-revision/releases/download/v21/WiFi-Clock-v2.0-R17-20261003_FULL.bin)** for the **TXW813 Wi-Fi chip**.

These are the only two firmware files needed for installation: **HC32 HEX**
and **Wi-Fi FULL BIN**. No separate raw HC32 BIN is needed.

If you experience any bugs or unexpected behaviour, please [let me know through GitHub Issues](https://github.com/maddenste/Chouchin-899-latest-revision/issues). Include your firmware versions, settings and a description of what happened; for a stopped clock, note the hand positions and LED behaviour.

**[Start here: equipment, software and flashing steps →](docs/GETTING_STARTED.md)**

No compiling or firmware editing is required. Install the two prepared files, then enter your Wi-Fi settings.

**Flashing instructions for both chips:** follow the [HC32 flashing guide](docs/HC32.md)
and [Wi-Fi flashing guide](docs/FLASHING.md). The Wi-Fi guide includes a double-click
launcher that guides you through file selection and catching the chip at power-on—no
commands to edit.

- [Firmware downloads, full changelog and checksums](https://github.com/maddenste/Chouchin-899-latest-revision/releases/tag/v21)
- [Current user guide](docs/USER_GUIDE.md)
- [Printable user manual (PDF)](docs/WiFi-Clock-User-Manual-v2.0.pdf)
- [Help / report a problem](docs/TROUBLESHOOTING.md)

We used **Windows 11**. These instructions cover Windows 11; other systems may work with suitable tools, but have not been verified.

## Make it your clock

- Wi-Fi network, two NTP servers, timezone and daylight saving rules.
- Daily updates in ten-minute steps, plus an extra wake for DST changes.
- Sweep, Burst or Hold&Start hand movement.
- Second hand battery saver: Off, Night parking or On.

Hold&Start recreates the characteristic Swiss railway station-clock motion: the seconds hand pauses at 12, then restarts as the minute hand advances.

[Watch an example of the Hold&Start movement](https://youtube.com/shorts/2L59QxfKAD8)

<img src="docs/screenshots/setup-desktop.png" alt="WiFi Clock settings page" width="700">

[Research, source notes and test results](docs/BACKGROUND.md) are available separately.

Unofficial project · WiFi Clock v2.0 — Modified by Steve Madden · 2026.
