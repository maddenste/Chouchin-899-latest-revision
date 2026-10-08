# Project background

This project supports the newer Chouchin-CH899 board with **HC32L130J8TA +
TXW813-320**. The older MM32/ESP hardware has a separate firmware project.

The Wi-Fi application provides NTP synchronisation, timezone and daylight
saving rules, a local settings page and the UART interface to the movement
controller. HC32 patches add selectable hand movement and second-hand parking.

The TXW813 upload procedure catches its boot-time debug window using CKLink
and the matching flash algorithm. Original recovery backups and investigation
captures are kept privately.

Current firmware: **HC32 V21 + Wi-Fi R17**. See [downloads](RELEASE_IDENTITY.md),
[architecture](ARCHITECTURE.md) and [source scope](BUILD.md).
