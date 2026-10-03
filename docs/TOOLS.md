# Before you start

We used **Windows**. Other systems may work, but are untested; the supplied script is Windows-specific.

## Equipment

| Item | Purpose |
| --- | --- |
| HC32L130J8TA + TXW813-320 Chouchin-899 | Supported board |
| CMSIS-DAP / DAPLink probe **with U_TX and U_RX UART pins** | HC32 upload |
| **CKLink Lite V2** | TXW813 upload |
| **3.3 V bench supply** | Power at battery terminals |
| Short wires, USB cables and multimeter | Connections and polarity checks |

The probes look similar but serve different chips. A SWD-only probe without UART pins cannot follow our HC32 procedure. Our CKLink reported App_ver 2.38.

Remove batteries. Connect bench **+3.3 V to battery +**, bench **negative to battery −**, and common programmer/board GND. Leave programmer 3V3/5V disconnected.

## Software downloads

| Tool | Official download | Version / files used |
| --- | --- | --- |
| XHSC MCU Programmer | [XHSC ISP V2.23 ZIP](https://oss-nc-beijing-2.cecloudcs.com/doc-xh/XHSC%20ISP%20V2.23.zip) · [vendor listing](https://www.xhsc.com.cn/product/1248.html) | V2.23; run XHSC.exe |
| PowerShell 7 | [Microsoft installation instructions](https://learn.microsoft.com/en-us/powershell/scripting/install/install-powershell-on-windows) | Open PowerShell 7, not Windows PowerShell 5.1 |
| C-SKY / XuanTie CDK | [Official CDK page](https://www.xrvm.cn/community/download?id=4478329920585535488) | CKV2ElfMinilib 3.10.29; csky-elfabiv2-gdb.exe |
| C-SKY / XuanTie DebugServer | [Official DebugServer page](https://www.xrvm.cn/community/download?id=4453644425750450176) | User layer 5.18.10 and CKLink Windows driver |
| TXW flash algorithm | [Taixin FPV 2.5.4.7 V45354](https://taixin-semi.com/zh/downloads/TXW81x_FPV-v2.5.4.7) | TXW81X_FLASH_ALGORITHM.elf and matching .init |

Vendor pages may require sign-in. Taixin lists **TXW81x_FPV-v2.5.4.7-45354.zip**. Extract it and find the two algorithm files under **sdk/chip/txw81x**. No SDK build is needed.

Install the CKLink driver supplied with DebugServer. Use **C-SKY** GDB, not RISC-V GDB. CDK may bundle a different DebugServer version: do not assume it matches our tested separate installation. Installer layouts/download availability can change; stop and ask for help if the listed tools cannot be obtained.

Keil, compilers, Python and a logic analyser are **not required for uploading**.

## Project files

1. [Release downloads](https://github.com/maddenste/Chouchin-899-latest-revision/releases/tag/v2.0-rc1): HC32 V15 HEX, Wi-Fi R16 FULL BIN, SHA256SUMS.txt and user manual.
2. On [the repository main page](https://github.com/maddenste/Chouchin-899-latest-revision), select **Code → Download ZIP**. Extract the current **tools** folder.
   Do not use the release's automatic “Source code” ZIP: its tag predates the clean-image script update.
3. Use paths without spaces, following [the Wi-Fi guide](FLASHING.md).

Check downloaded firmware with PowerShell 7 if desired:

~~~powershell
Get-FileHash 'C:\ClockFlash\Chouchin-899-HC32-V15-20261001.hex' -Algorithm SHA256
Get-FileHash 'C:\ClockFlash\WiFi-Clock-v2.0-R16-20261002_FULL.bin' -Algorithm SHA256
~~~

Compare against SHA256SUMS.txt. The Wi-Fi script also checks the image and algorithms automatically.

[Next: upload HC32 →](HC32.md)
