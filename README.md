# Chouchin-899 WiFi Clock — newer HC32 / TXW board revision

**WiFi Clock · v2.0 — Modified by Steve Madden · 2026**

Unofficial firmware and reverse-engineering documentation for the **newer
Chouchin-899 / CH-899 board fitted with HDSC HC32L130J8TA and Taixin TXW813-320**.
Check both chip markings: the product name alone does not identify the electronics.

| Board family | Movement controller | Wi-Fi controller | Project |
| --- | --- | --- | --- |
| Older board | MM32SPIN family | ESP8285N08 / ESP-01F | [Original Chouchin-CH899-Firmware](https://github.com/maddenste/Chouchin-CH899-Firmware) |
| Newer board covered here | HDSC HC32L130J8TA | Taixin TXW813-320 | This project |

**ESP images cannot be uploaded to the TXW. MM32 flash algorithms must not be
used on the HC32.** This is not an Arduino build or a manufacturer-endorsed upgrade.
The PCB revision number is not established; compatibility is identified by chips.

## Board photographs

<img src="docs/photos/chouchin-899-newer-board-overview.jpg" alt="Newer Chouchin-899 board fitted with HC32L130 and TXW813-320" width="420">

The newer HC32/TXW board. See the [hardware guide](docs/HARDWARE.md#board-photographs)
for close-ups of both chips and the labelled programming pads.

## Candidate status

The current pair is **HC32 V15 + Wi-Fi v2.0 R16**. R16 updates the version and
attribution footer; backend behavior is unchanged from R14. V1.0 belongs to
the older board project. Its diagnostic build ID remains separately accessible.
The owner reports R14 works, and has confirmed movement, parking, a 17:10 daily
wake, forward and backward DST. The forward test was on R13; the backward result
was reported after R14 installation. Non-hourly/Disabled DST and the 48-hour soak
remain pending. See the [test ledger](docs/TESTING.md).

Download the candidate pair, manual and checksums from
[WiFi Clock v2.0-rc1](https://github.com/maddenste/Chouchin-899-latest-revision/releases/tag/v2.0-rc1).
HC32 V15 modifies the board's original firmware; it is not a complete rewrite.

## Features

- Local 2.4 GHz Wi-Fi setup, SSID scan, hidden/manual entry and saved SSID display.
- Two NTP servers; secondary joins after five seconds, first valid reply wins.
- Correlated NTP replies supported with local source-IP redirection.
- 40 timezones; Automatic/Disabled/custom POSIX DST and live preview.
- Daily update at any hour, minutes 00/10/20/30/40/50.
- Additional DST wake at the needed minute, without changing the saved daily time.
- Sweep, Burst and Hold&Start movement styles. Hold&Start replicates the
  characteristic Swiss railway station-clock movement: the seconds hand
  pauses at 12, then restarts as the minute hand jumps forward.
- Second hand battery saver: Off, Night parking (00:00–06:00), or On
  (reach 12 and stay parked, useful without a second hand fitted).
- WIFIAPPING browser keep-alive in AP and station modes.
- AP WiFi-Clock-Setup; DHCP hostname WiFi-Clock-ABCDEF, uppercase MAC suffix.
- Original HC32 battery checks retained; bench power is not battery-life validation.

## Start here

For everyday setup and operation, use the
[three-page WiFi Clock v2.0 user manual](docs/WiFi-Clock-User-Manual-v2.0.pdf).

The repository contains application code, the editable web page, tests and tools.
Vendor startup/build integration is omitted: this is not a complete reproducible
cross-build tree. The prebuilt candidates are separate release downloads.

1. [From-scratch walkthrough](docs/GETTING_STARTED.md).
2. [Board identification and pin connections](docs/HARDWARE.md).
3. [Hardware/software list and links](docs/TOOLS.md).
4. [TXW backup, power-on catch and programming](docs/FLASHING.md).
5. [HC32 V15 installation and limitations](docs/HC32.md).
6. [Build instructions](docs/BUILD.md).
7. [Setup and everyday use](docs/USER_GUIDE.md).

References: [journey](docs/HISTORY.md), [UART protocol](docs/PROTOCOL.md),
[architecture](docs/ARCHITECTURE.md), [troubleshooting](docs/TROUBLESHOOTING.md),
[testing](docs/TESTING.md), [release checklist](docs/RELEASE_CHECKLIST.md).

Review the [package inventory](docs/PACKAGE_CONTENTS.md) and
[release identifiers](docs/RELEASE_IDENTITY.md) before publication.

## What belongs in GitHub

Source, editable web page, tests, tools and documentation belong here.
Vendor SDK/tools, original firmware, raw captures, private credentials and
factory-containing personal images do not. Releases supply only our complete
HC32 V15 HEX and clean 2 MiB Wi-Fi R16 FULL, plus the user manual and a separate
checksum file. No compilation or manual image assembly is needed. After Wi-Fi
flashing, join WiFi-Clock-Setup and enter your Wi-Fi and clock settings.
Do not bulk-upload the investigation folder.
