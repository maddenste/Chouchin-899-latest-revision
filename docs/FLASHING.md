# Flash Wi-Fi R16 onto TXW813

For the **TXW813-320** on the newer Chouchin-899. HC32 is flashed separately.

## Downloads and wiring

Download **WiFi-Clock-v2.0-R16-20261002_FULL.bin** and **SHA256SUMS.txt** from
[the release](https://github.com/maddenste/Chouchin-899-latest-revision/releases/tag/v2.0-rc1).
The BIN contains the complete 2 MiB clean installation; no compiling, image
assembly or settings-copy helper is needed. Original firmware is not included.

| CKLink Lite pin | TXW board pad |
| --- | --- |
| TMS/IO | PA9 |
| TCK/CK | PA10 |
| GND | GND |

Use stable board-compatible power and common ground. Do not connect 5 V.
See [hardware](HARDWARE.md) for voltage-reference cautions and [tools](TOOLS.md)
for the separately obtained C-SKY DebugServer/GDB and matching TXW flash algorithm.

**Save a verified original backup before writing. Keep it private.**
The full installation erases all old settings and configuration sectors.
It does not erase the chip's eFuse. R16 uses eFuse MAC identification and its
own settings store, not the old configuration-loading code. The clean image
has been checked offline but not yet flashed and tested with its final sectors blank.

## Flash at address zero

Use the FULL BIN as a binary image at **0x000000**, covering all **2,097,152 bytes**.
Do not flash an APP/raw linker image in its place.

If you need to catch the chip at power-on, use the existing one-shot writer.
Download the latest repository ZIP using GitHub's Code → Download ZIP, extract it,
and open **PowerShell 7** in its top-level folder. Close FlashProgrammer and
DebugServer windows first.

Substitute your file paths and the IP of the **PC running DebugServer**, not the
clock's Wi-Fi IP:

```powershell
.\tools\Program-TXW813-DCDC0-WriteOnly-OnPower.ps1 -CleanFullImage -ImagePath 'C:\ClockFiles\WiFi-Clock-v2.0-R16-20261002_FULL.bin' -AlgorithmPath 'C:\ClockTools\TXW81X_FLASH_ALGORITHM.elf' -InitScriptPath 'C:\ClockTools\TXW81X_FLASH_ALGORITHM.init' -DebuggerEndpoint 'YOUR_PC_IP:1025' -Program
```

The script checks the download and image layout, then retries debug connections.
Turn board power on while it is trying. If it misses the window, cycle power
while still trying. Choose **No** if an ICE update box appears.

**Once connected, stop cycling power.** Wait for **WRITTEN**.
The script sends one complete erase/program operation, with no confirmation,
automatic write retry or flash readback. The bench write took about 90–100 seconds;
other setups may take longer. If it fails after writing starts, stop and inspect
the logs rather than immediately repeating it.

Omit -Program for an offline preflight without accessing the board.
Paths used in the vendor command must not contain spaces. Custom installations
can use -ServerPath and -GdbPath.

## Set up after flashing

Power-cycle after success. Join **WiFi-Clock-Setup**, open **http://192.168.4.1/**
and enter your Wi-Fi and clock settings. Old settings are not retained.

If HC32 V15 is already installed, leave it alone. Otherwise follow
[the HC32 instructions](HC32.md).

## Original backup, if needed

The read script uses the same debug wiring and separately obtained algorithm.
Start it before applying board power, then power on while it is trying:

```powershell
.\tools\Read-TXW813-OnPower.ps1 -AlgorithmPath 'C:\ClockTools\TXW81X_FLASH_ALGORITHM.elf' -InitScriptPath 'C:\ClockTools\TXW81X_FLASH_ALGORITHM.init' -DebuggerEndpoint 'YOUR_PC_IP:1025' -OutputRoot 'C:\ClockBackups' -GdbInitAndDump
```

Repeat independently. Compare the actual output files: both must be exactly
2 MiB, plausible and have the same SHA-256. An all-FF dump is not a valid backup.
The read initialises hardware through the SDK script; it does not erase/program.

## Matching algorithm

Checks are automatic; do not bypass them for another SDK algorithm:

- ELF: 7BF137DB393ECF74F361554691044D8D65266753340718DA63373510EEEEDC8A
- init: 16DE6E4FC6A9D4124B98D45DC934AAE6C6E2AF79E2681DF537F63C790E0DB88B

See [troubleshooting](TROUBLESHOOTING.md) if a connection or write fails.
