# Firmware downloads

Current release: **[HC32 V21 + Wi-Fi R17](https://github.com/maddenste/Chouchin-899-latest-revision/releases/tag/v21)**.

| File | Target | SHA-256 |
| --- | --- | --- |
| Chouchin-899-HC32-V21.hex | HC32 movement controller | 12C82774B8C4E7205B484A2EB6930FC18F6E705172DEE53E2B03C10C9690ED18 |
| Chouchin-899-HC32-V21.bin | HC32 raw image | 52788FB1D3DCC4B4671CE7011F67F61BBE0BAF53010CFDA9358149978CAAFB8A |
| WiFi-Clock-v2.0-R17-20261003_FULL.bin | TXW813 Wi-Fi controller | 1F4908F9ACC9160CDF53CA3F5C5BEF96CBF4C9197700C87640C01F13592CE9A9 |

Use the **HEX** with the [HC32 flashing guide](HC32.md). The HC32 BIN is
the same 64 KiB image in raw format; do not use it for the Wi-Fi chip.
The Wi-Fi FULL is a clean 2 MiB installation image and clears saved settings.
Leave already-correct Wi-Fi firmware alone.

**SHA256SUMS.txt** covers all three downloads. Download firmware separately
from the project ZIP. [Equipment and software](TOOLS.md) · [Wi-Fi upload](FLASHING.md).
