# Before you start

We used **Windows**. Other systems may work, but are untested; the supplied script is Windows-specific.

## Equipment

| Item | Purpose |
| --- | --- |
| HC32L130J8TA + TXW813-320 Chouchin-899 | Supported board |
| [CMSIS-DAP / DAPLink probe](https://www.ebay.co.uk/itm/254327616482) **with U_TX and U_RX UART pins** | HC32 upload |
| [CKLink Lite V2](https://www.aliexpress.com/item/1005009972319809.html) | TXW813 upload |
| **TENSTAR ROBOT TSP-03 fixed 3.3 V supply** | Power at battery terminals; label states 3.3 V DC / 3 W |
| Short wires, USB cables and multimeter | Connections and polarity checks |

The probes look similar but serve different chips. A SWD-only probe without UART pins cannot follow our HC32 procedure. Our CKLink reported App_ver 2.38.

These are the programmer listings Steve supplied for the hardware used, not
endorsements of other variants. Check the selected model and signal labels before
ordering; seller stock/listings can change. The eBay listing was accessible when
checked on 3 October 2026; the AliExpress page could not be independently reopened.
Steve used a **TENSTAR ROBOT TSP-03**, not an adjustable bench supply.
Its label states **100–240 V AC input** and **3.3 V DC / 3 W output**.
No exact purchase link has been supplied.

**Mains safety:** this module has mains-input terminals. This guide covers only
the low-voltage output connections, not mains wiring. For a new setup, use a
properly enclosed, regulated 3.3 V DC supply; do not operate exposed mains terminals.
Do not confuse the AC input with the +Vo/−Vo DC output.

Remove batteries. Connect supply **+3.3 V to battery +**, supply **negative to battery −**, and common programmer/board GND. Leave programmer 3V3/5V disconnected.

## Optional diagnostic equipment

We used a [DAOKAI 24 MHz, 8-channel USB logic analyser](https://www.amazon.co.uk/dp/B0CP952357)
with [PulseView](https://www.sigrok.org/wiki/Downloads) to inspect the inter-chip
UART messages. This is **not required for flashing**. The purchase link was
supplied by Steve; check the selected model, as listings can change.
See [UART diagnosis and wiring](TROUBLESHOOTING.md#optional-uart-diagnosis).

## Software downloads

| Tool | Official download | Version / files used |
| --- | --- | --- |
| XHSC MCU Programmer | [XHSC ISP V2.23 ZIP](https://oss-nc-beijing-2.cecloudcs.com/doc-xh/XHSC%20ISP%20V2.23.zip) · [vendor listing](https://www.xhsc.com.cn/product/1248.html) | V2.23; run XHSC.exe |
| PowerShell | Included with Windows | The guided launcher uses built-in Windows PowerShell 5.1; no extra installation |
| C-SKY / XuanTie CDK | [Official CDK page](https://www.xrvm.cn/community/download?id=4478329920585535488) | CKV2ElfMinilib 3.10.29; csky-elfabiv2-gdb.exe |
| C-SKY / XuanTie DebugServer | [Official DebugServer page](https://www.xrvm.cn/community/download?id=4453644425750450176) | User layer 5.18.10 and CKLink Windows driver |
| TXW flash algorithm | [Taixin FPV 2.5.4.7 V45354](https://taixin-semi.com/zh/downloads/TXW81x_FPV-v2.5.4.7) | TXW81X_FLASH_ALGORITHM.elf and matching .init |

Vendor pages may require sign-in. Taixin lists **TXW81x_FPV-v2.5.4.7-45354.zip**. Extract it and find the two algorithm files under **sdk/chip/txw81x**. No SDK build is needed.

Install the CKLink driver supplied with DebugServer. Use **C-SKY** GDB, not RISC-V GDB. CDK may bundle a different DebugServer version: do not assume it matches our tested separate installation. Installer layouts/download availability can change; stop and ask for help if the listed tools cannot be obtained.

Keil, compilers, Python and a logic analyser are **not required for uploading**.

## Project files

1. [Release downloads](https://github.com/maddenste/Chouchin-899-latest-revision/releases/tag/v2.0): HC32 V15 HEX, Wi-Fi R17 FULL BIN, SHA256SUMS.txt and user manual.
2. On [the repository main page](https://github.com/maddenste/Chouchin-899-latest-revision), select **Code → Download ZIP**. Extract the complete project; keep **Flash-WiFi.bat** beside the **tools** folder.
   Alternatively, use **“Source code (zip)”** on the v2.0 release page; it also
   includes the guided launcher. Download the firmware images separately.
3. Follow [the guided Wi-Fi upload](FLASHING.md). The launcher handles input-file paths for you; no PowerShell commands need editing.

Check downloaded firmware with PowerShell if desired:

~~~powershell
Get-FileHash 'C:\ClockFlash\Chouchin-899-HC32-V15-20261001.hex' -Algorithm SHA256
Get-FileHash 'C:\ClockFlash\WiFi-Clock-v2.0-R17-20261003_FULL.bin' -Algorithm SHA256
~~~

Compare against SHA256SUMS.txt. The Wi-Fi script also checks the image and algorithms automatically.

[Next: upload HC32 →](HC32.md)
