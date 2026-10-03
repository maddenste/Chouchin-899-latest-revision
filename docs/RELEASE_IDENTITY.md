# Firmware reference

The first public download is **WiFi Clock v2.0 — Public release 1**.
The V15/R16 names below are internal build identifiers retained in filenames.

WiFi Clock **v2.0 R16** and **HC32 V15**, 1 October 2026.
V1.0 refers to the older MM32/ESP board project, not this hardware revision.

| Image | Size | SHA-256 |
| --- | --- | --- |
| WiFi-Clock-v2.0-R16-20261001_APP.bin | 326160 bytes | E77836597F056C7627609B982298AACE6E8952CF9A5FDDBA80FA302699DE1707 |
| Chouchin-899-HC32-V15-20261001.bin | 65536 bytes | B1D557505163B973FD59B20AA80246DA853F2F4A3D3AC555D53715180C533A60 |
| Chouchin-899-HC32-V15-20261001.hex | 180236 bytes | 58A62FE21A1E9FE0E90373A1466F927EAA86EB69CE9F99067DEDA0186A4EEBE4 |
| WiFi-Clock-v2.0-R16-20261002_FULL.bin | 2097152 bytes | 54120864F2465D61D18D50E338DF1EDB006530271F15390885F808FD7BCC8D44 |

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
Packaging changed on 2 October; firmware code is unchanged. The clean image
has passed offline layout/preflight checks but has not been hardware-tested
with the old configuration sectors blank.

The v2.0 tag includes the public source, current upload script and polished
documentation. It does not supply the omitted vendor integration or make the
public tree independently rebuildable. Do not substitute a raw linker binary
for the packaged firmware.
