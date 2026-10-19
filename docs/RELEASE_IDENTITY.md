# Firmware downloads

Current release: **[HC32 V21 (R3 branch repair) + Wi-Fi R17](https://github.com/maddenste/Chouchin-899-latest-revision/releases/tag/v21)**.

| File | Target | SHA-256 |
| --- | --- | --- |
| Chouchin-899-HC32-V21.hex | HC32 movement controller, repaired revision | F353485E01706E045FCD1EFF4D12C1A519781396F0305DCEFBD0849E48909E35 |
| WiFi-Clock-v2.0-R17-20261003_FULL.bin | TXW813 Wi-Fi controller | 1F4908F9ACC9160CDF53CA3F5C5BEF96CBF4C9197700C87640C01F13592CE9A9 |

Use the **HEX** with the [HC32 flashing guide](HC32.md).
The redundant raw HC32 BIN has been removed; it is not needed for installation.
The **Wi-Fi FULL BIN** is the required TXW813 installation image, not an HC32 image.
The Wi-Fi FULL is a clean 2 MiB installation image and clears saved settings.
Leave already-correct Wi-Fi firmware alone.

**SHA256SUMS.txt** covers both firmware downloads. Download firmware separately
from the project ZIP. [Equipment and software](TOOLS.md) · [Wi-Fi upload](FLASHING.md).

The HC32 download is identical to the owner-tested
`Chouchin-899-HC32-V21-R3-FIX.hex`, renamed to retain the existing public download
URL. Only eight bytes differ from the original V21 image. No experimental
smoothing is included. See [repair verification](TESTING.md#hc32-v21-branch-repair).

Earlier V21 HEX downloads had SHA-256
`12C82774B8C4E7205B484A2EB6930FC18F6E705172DEE53E2B03C10C9690ED18`
and do **not** include this repair. The release tag remains `v21`; use the
current checksum to identify the repaired file. Wi-Fi R17 is byte-for-byte unchanged.
