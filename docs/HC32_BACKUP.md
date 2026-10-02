# HC32 original backup — advanced reference

Keep your original backup private. This procedure reads the chip; it does not
supply original firmware for download.

## Original firmware backup — Keil µVision 5 over SWD

The owner confirmed the original HC32 firmware backup was read using
**Keil µVision 5**, with DAPLink **TMS/IO to DIO, TCK/CK to CLK and GND to
common board ground**. This was not an XHSC UART read.

### Read-only Keil setup

The archived read-only project confirms these settings:

| Setting | Value |
| --- | --- |
| Device | HDSC HC32L130J8TA |
| Device pack | HDSC.HC32L130.1.0.1 |
| Debug adapter | CMSIS-DAP (CMSIS_AGDI.dll) |
| Interface / initial clock | SWD / 100 kHz |
| Flash address / size | 0x00000000 / 0x10000 (64 KiB) |
| RAM address / size | 0x20000000 / 0x2000 (8 KiB) |
| Load Application at Startup | Disabled in the target debugger |
| Update Target before Debugging | Disabled |
| Debug initialization file | None |

Install Keil µVision 5 and the device pack separately. Create a project for
this exact device, without adding application/startup code. Under Options for
Target → Debug, select the CMSIS-DAP adapter and disable Load Application at
Startup. In its Settings, select SWD at 100 kHz and your connected probe.
Under Utilities, disable Update Target before Debugging. Also disable
Run to main() for a backup-only session; the archived project retained that
checkbox, but it is unnecessary when no application is loaded.

Connect the confirmed SWD wires, power the board and start a debug session
without building or downloading firmware. Open View → Command Window.
Do not use Flash Download, erase, unlock or a flash algorithm to obtain a backup.
Attaching may halt/reset the running controller: this is flash-read-only,
not a guarantee of zero runtime disturbance.

The saved session records identify **SAVE commands entered in Keil's Command
pane**, rather than a standalone PowerShell read script. On 24 September 2026,
the read was repeated in five chunks spanning the complete 64 KiB flash.
The historical commands below use a relative output filename; choose a new
output location explicitly (replace each filename with a new absolute path
without spaces if needed). Do not overwrite an existing backup.

```text
SAVE HC32L130_recheck_00000000-000043FF.hex 0x00000000,0x000043FF
SAVE HC32L130_recheck_00004400-0000B3FF.hex 0x00004400,0x0000B3FF
SAVE HC32L130_recheck_0000B400-0000B5FF.hex 0x0000B400,0x0000B5FF
SAVE HC32L130_recheck_0000B600-0000B7FF.hex 0x0000B600,0x0000B7FF
SAVE HC32L130_recheck_0000B800-0000FFFF.hex 0x0000B800,0x0000FFFF
```

Local notes record valid Intel HEX checksums, 65,536 distinct bytes with no
gaps or overlaps, and an assembled image matching the earlier backup.
The assembled original SHA-256 is
`7F464A7EF98DBFC5FE41A3B2EA14E16EF964885490D7C8276CE34DC4D25C433F`.
This is a historical procedure, not a newly repeated hardware test.
Do not substitute an erase, Download or protection-unlock operation for
a read-only backup. Keil's Download action writes firmware to the target.

### Validate the exported backup

The included Python helper checks Intel HEX checksums, rejects gaps/overlaps
and assembles exactly 65,536 bytes. It never connects to a programmer.
Supply your five actual export filenames:

```powershell
python .\tools\verify_hc32_backup.py chunk1.hex chunk2.hex chunk3.hex chunk4.hex chunk5.hex --output original-hc32.bin
```

The output must not already exist. Keep the original HEX files and hash
privately. Different factory firmware can legitimately have a different hash;
do not require another owner's original hash to match yours.
