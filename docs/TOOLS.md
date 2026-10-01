# Hardware and software

## Hardware actually used or required

- Newer Chouchin-899: HDSC HC32L130J8TA movement MCU and Taixin TXW813-320 Wi-Fi SoC.
- CKLink Lite V2 detected by XuanTie DebugServer, probe firmware App_ver 2.38.
- CMSIS-DAP / DAPLink ARM JTAG/SWD debug probe for HC32.
  Wiring is documented from the owner's confirmed pin labels/connections;
  no photograph is available. Do not infer a header pin order from the probe name.
- Bench supply (owner reported 3.38 V during diagnosis), multimeter and short wires.
  Follow the board's electrical requirements; this measurement is not a universal
  voltage specification.
- Logic analyser, PulseView UART decoder and saved .sr captures.
- OPNsense packet capture for DNS/NTP diagnosis; not required for normal operation.

## Software

| Purpose | Tool / version used |
| --- | --- |
| TXW connection | XuanTie / C-SKY DebugServer; logged user layer 5.18.10 |
| TXW debugger/compiler | C-SKY CDK, CKV2ElfMinilib 3.10.29, csky-elfabiv2 tools |
| SDK build | TXW81x IOT SDK 2.5.2.6-31320 |
| Flash algorithm | TXW81X_FLASH_ALGORITHM.elf and matching .init from separately obtained FPV SDK |
| HC32 firmware installation | XHSC MCU Programmer V2.23; UART U_TX to DIO, U_RX to CLK, GND to GND, BOOT held at 3.3 V |
| HC32 original backup | Keil µVision 5 Command-pane SAVE commands over DAPLink SWD; offline HEX verification helper included |
| HC32 device project | Keil and HDSC.HC32L130.1.0.1 device pack; distinct from actual UART programming route |
| HC32 read-only diagnosis | pyOCD / CMSIS-DAP |
| Host tests | Python 3 + tzdata, TinyCC |
| Browser tests | Node.js + Playwright + Microsoft Edge |
| Scripts | PowerShell; PowerShell 7 recommended |

Vendor SDKs, flash algorithms, executable tools and original dumps are not
bundled in the public draft. Installing a tool does not establish permission
to redistribute it or a firmware linked against its libraries.

The owner supplied a screenshot confirming application version **V2.23**.
Windows executable metadata reports 1.0.0.0; use the displayed V2.23 version
when identifying the programming tool, not that generic file metadata.
Its local directory includes the vendor Cortex-M online-programmer manual
named Rev2.13. Neither the executable nor that manual is bundled here.

## Useful primary references

- [Original older-board project](https://github.com/maddenste/Chouchin-CH899-Firmware).
- [wuxx CKLink-lite hardware reference](https://github.com/wuxx/CKLink-lite).
- [CKLink firmware converter](https://github.com/cjacker/cklink-lite-fw-convertor):
  investigated, but probe conversion is **not** a required step in our workflow.
- [PulseView](https://sigrok.org/wiki/PulseView).
- [pyOCD commands](https://pyocd.io/docs/command_reference.html) and
  [target support](https://pyocd.io/docs/target_support.html).
- [Keil device catalogue](https://www.keil.arm.com/devices/).
- [PowerShell installation](https://learn.microsoft.com/en-us/powershell/scripting/install/install-powershell-on-windows).

Do not upgrade CKLink firmware merely because a prompt appears. The investigation
included a failed upgrade; the working catch workflow ignores the optional
upgrade. Select **No** in the prompt when following that workflow.
