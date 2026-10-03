# Firmware reference

The first public download is **WiFi Clock v2.0 — Public release 1**.
The V15/R16 names below are internal build identifiers retained in filenames.

WiFi Clock **v2.0 R16** and **HC32 V15**, 1 October 2026.
V1.0 refers to the older MM32/ESP board project, not this hardware revision.

| Image | Size | SHA-256 |
| --- | --- | --- |
| WiFi_Clock_v2_0_R16_20261003_Fixes_APP.bin | 326160 bytes | 07E750565AA9F3AB3ABA5328B0A11BE8E8A4223AD010C38CBB71F5B8431AB4A2 |
| Chouchin-899-HC32-V15-20261001.bin | 65536 bytes | B1D557505163B973FD59B20AA80246DA853F2F4A3D3AC555D53715180C533A60 |
| Chouchin-899-HC32-V15-20261001.hex | 180236 bytes | 58A62FE21A1E9FE0E90373A1466F927EAA86EB69CE9F99067DEDA0186A4EEBE4 |
| WiFi-Clock-v2.0-R16-20261002_FULL.bin | 2097152 bytes | 3CDA5DB1040F64533186499A1FBCF4348E996FC672021EE9A7FFC936A08FB708 |

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

The public FULL contains the exact R16 APP at offset zero and FF throughout
the remainder, including the final 8 KiB. It contains no copied original data.
It is written directly at offset zero for a clean installation. Enter Wi-Fi
settings after restarting. No configuration-copy helper is needed.
The Wi-Fi download was rebuilt on 3 October with custom-DST year-boundary and
offset validation fixes, plus full-length NTP hostname responses. Its public
filename and revision are retained; identify this rebuild by SHA-256.
It passed the full offline test/build pipeline and clean-image preflight.
The rebuild has not been flashed to the owner's clock; the hardware observations
above describe the previously installed R16 build.

Use the main branch for the updated public source and upload script; the v2.0
tag retains the original release snapshot. Neither supplies the omitted vendor integration or makes the
public tree independently rebuildable. Do not substitute a raw linker binary
for the packaged firmware.
