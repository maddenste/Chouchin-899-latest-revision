# Current local release identifiers

WiFi Clock **v2.0 R16** and **HC32 V15**, 1 October 2026.
V1.0 refers to the older MM32/ESP board project, not this hardware revision.

| Image | Size | SHA-256 |
| --- | --- | --- |
| WiFi-Clock-v2.0-R16-20261001_APP.bin | 326160 bytes | E77836597F056C7627609B982298AACE6E8952CF9A5FDDBA80FA302699DE1707 |
| Chouchin-899-HC32-V15-20261001.bin | 65536 bytes | B1D557505163B973FD59B20AA80246DA853F2F4A3D3AC555D53715180C533A60 |
| Chouchin-899-HC32-V15-20261001.hex | 180236 bytes | 58A62FE21A1E9FE0E90373A1466F927EAA86EB69CE9F99067DEDA0186A4EEBE4 |

These identify the candidates attached to
[v2.0-rc1](https://github.com/maddenste/Chouchin-899-latest-revision/releases/tag/v2.0-rc1).
The HC32 installation was reconfirmed by the owner.
The local XHSC configuration selects the V15 HEX, and its latest log reports
programming and verification success; the log is kept private.

The Wi-Fi R16 write-only operation started at 20:17:50 BST and reported
Program success at 20:19:27 BST on 1 October 2026. No flash readback was requested.
Programmer success is not independent proof of on-chip byte equality or a
substitute for post-boot functional testing.

The FULL image is assembled locally using each board's own original factory
sectors. Never distribute Steve's or another owner's full 2 MiB image.

Source and binary release tags must match the exact approved build. Do not
attach a raw linker binary in place of the packaged APP.
