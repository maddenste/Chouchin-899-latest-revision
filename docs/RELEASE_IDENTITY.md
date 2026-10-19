# Firmware downloads

Current release: **[HC32 V21 + Wi-Fi R17](https://github.com/maddenste/Chouchin-899-latest-revision/releases/tag/v21)**.

| File | Target | SHA-256 |
| --- | --- | --- |
| Chouchin-899-HC32-V21.hex | HC32 movement controller | F353485E01706E045FCD1EFF4D12C1A519781396F0305DCEFBD0849E48909E35 |
| WiFi-Clock-v2.0-R17-20261003_FULL.bin | TXW813 Wi-Fi controller | 1F4908F9ACC9160CDF53CA3F5C5BEF96CBF4C9197700C87640C01F13592CE9A9 |

Use the **HEX** with the [HC32 flashing guide](HC32.md).
No separate raw HC32 BIN is needed for installation.
The **Wi-Fi FULL BIN** is the required TXW813 installation image, not an HC32 image.
The Wi-Fi FULL is a clean 2 MiB installation image and clears saved settings.

**SHA256SUMS.txt** covers both firmware downloads. Download firmware separately
from the project ZIP. [Equipment and software](TOOLS.md) · [Wi-Fi upload](FLASHING.md).

The V21 download has been updated in place, so the tag and filename alone do
not identify an older saved copy. Use the current checksum above.
[Technical revision history and verification](TESTING.md#hc32-v21-branch-repair)
are recorded separately.
