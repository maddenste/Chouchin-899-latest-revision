# Firmware reference

The first public download is **WiFi Clock v2.0 — Public release 1**.
The V15/R17 names below are internal build identifiers retained in filenames.

Current downloads: WiFi Clock **v2.0 R17**, 3 October 2026, and **HC32 V15**,
1 October 2026. The public release tag remains **v2.0**.
V1.0 refers to the older MM32/ESP board project, not this hardware revision.

| Image | Size | SHA-256 |
| --- | --- | --- |
| WiFi_Clock_v2_0_R17_20261003_APP.bin | 327184 bytes | C028F8CA7196795C05D7CABF8CF2E39B64DCDD06734DB542DB56A027D5C948A5 |
| Chouchin-899-HC32-V15-20261001.bin | 65536 bytes | B1D557505163B973FD59B20AA80246DA853F2F4A3D3AC555D53715180C533A60 |
| Chouchin-899-HC32-V15-20261001.hex | 180236 bytes | 58A62FE21A1E9FE0E90373A1466F927EAA86EB69CE9F99067DEDA0186A4EEBE4 |
| WiFi-Clock-v2.0-R17-20261003_FULL.bin | 2097152 bytes | 1F4908F9ACC9160CDF53CA3F5C5BEF96CBF4C9197700C87640C01F13592CE9A9 |

Only the HC32 HEX and clean Wi-Fi FULL are attached to
[v2.0](https://github.com/maddenste/Chouchin-899-latest-revision/releases/tag/v2.0).
APP and HC32 BIN identities remain above for developer/reference checks.
The HC32 installation was reconfirmed by the owner.
The local XHSC configuration selects the V15 HEX, and its latest log reports
programming and verification success; the log is kept private.

The Wi-Fi R16 write-only operation started at 20:17:50 BST and reported
Program success at 20:19:27 BST on 1 October 2026. No flash readback was requested.
Programmer success is not independent proof of on-chip byte equality or a
substitute for post-boot functional testing.

The public FULL contains the exact R17 APP at offset zero and FF throughout
the remainder, including the final 8 KiB. It contains no copied original data.
It is written directly at offset zero for a clean installation. Enter Wi-Fi
settings after restarting. No configuration-copy helper is needed.
R17 passed the full offline test/build pipeline, UART/reset timing and command
sequence regressions, and clean-image preflight. The status API reports
`TXW813 HC32-V15 R17`; the footer remains WiFi Clock v2.0.
R17 has not been flashed to the owner's clock; the hardware observations
above describe the previously installed R16 build.

Both the main-branch ZIP and the v2.0 release's "Source code (zip)" include
the guided Flash-WiFi.bat launcher and its required scripts. Download the
packaged firmware images separately from the release assets. Neither source
archive includes the omitted vendor integration or makes the public tree
independently rebuildable. Do not substitute a raw linker binary for the
packaged firmware.
