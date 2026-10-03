# Upload Wi-Fi R16 — TXW813

[Start here](GETTING_STARTED.md) · [Equipment/software](TOOLS.md)

Our **CKLink Lite V2 power-on catch procedure** writes the complete 2 MiB FULL at **0x000000**. No building or settings-copy helper is needed.

## 1. Prepare files

Create these folders and copy the files:

~~~text
C:\ClockFlash\
  tools\                         ← from current Code → Download ZIP
  WiFi-Clock-v2.0-R16-20261002_FULL.bin
C:\ClockTools\
  TXW81X_FLASH_ALGORITHM.elf
  TXW81X_FLASH_ALGORITHM.init
~~~

Get the BIN from [the release](https://github.com/maddenste/Chouchin-899-latest-revision/releases/tag/v2.0-rc1), algorithms from [the matching Taixin package](TOOLS.md).
**Paths must not contain spaces.**

Locate your installed DebugServer and C-SKY GDB. Our paths were:

~~~text
C:\C-Sky\DebugServer\bin\DebugServerConsole.exe
C:\C-Sky\CDKRepo\Toolchain\CKV2ElfMinilib\V3.10.29\R\bin\csky-elfabiv2-gdb.exe
~~~

Use your actual paths below if different. Do not select RISC-V GDB.

## 2. Wire with board power off

Remove batteries. Connect bench 3.3 V at the **battery terminals**.

![TXW813 signal connections](diagrams/txw-upload.svg)

| CKLink Lite pin | Board pad |
| --- | --- |
| TMS/IO | PA9 |
| TCK/CK | PA10 |
| GND | GND |

Leave other probe pins disconnected, including 3V3, 5V, TDI, TDO and nRST. No BOOT/PA8 strap is used.

<img src="photos/txw813-wifi-chip-and-debug-pads.jpg" alt="TXW813 and J1 GND, PA10, PA9, PA8, VCC pads" width="650">

Match pad names, not guessed header order. Plug CKLink into USB; keep bench power off for now.

## 3. Check the command before writing

Close **FlashProgrammer and all DebugServer windows**; the script starts its own server.

Open **PowerShell 7**. Run **ipconfig** and find the IPv4 address of your PC's active network adapter. Use the **PC running DebugServer**, not the clock's address.

Paste this block, changing **192.168.0.5** to your PC address and changing installed-program paths if needed:

~~~powershell
Set-Location C:\ClockFlash
$flashArgs = @{
    CleanFullImage = $true
    ImagePath = 'C:\ClockFlash\WiFi-Clock-v2.0-R16-20261002_FULL.bin'
    AlgorithmPath = 'C:\ClockTools\TXW81X_FLASH_ALGORITHM.elf'
    InitScriptPath = 'C:\ClockTools\TXW81X_FLASH_ALGORITHM.init'
    ServerPath = 'C:\C-Sky\DebugServer\bin\DebugServerConsole.exe'
    GdbPath = 'C:\C-Sky\CDKRepo\Toolchain\CKV2ElfMinilib\V3.10.29\R\bin\csky-elfabiv2-gdb.exe'
    DebuggerEndpoint = '192.168.0.5:1025'
}
.\tools\Program-TXW813-DCDC0-WriteOnly-OnPower.ps1 @flashArgs
~~~

This run is **offline checks only**. Continue when it prints **Preflight OK** and **Offline preflight only; no hardware accessed**. Fix missing-file/hash errors first.

If Windows blocks the downloaded script, inspect it and unblock that specific file:

~~~powershell
Unblock-File C:\ClockFlash\tools\Program-TXW813-DCDC0-WriteOnly-OnPower.ps1
~~~

## 4. Catch and write

In the **same PowerShell window**, run:

~~~powershell
.\tools\Program-TXW813-DCDC0-WriteOnly-OnPower.ps1 @flashArgs -Program
~~~

1. When debug connection attempts begin, turn on the bench supply.
2. If not caught, cycle board power while the script is still retrying. Many attempts can be needed.
3. Select **No** if an ICE/probe firmware update prompt appears.
4. **Once connected and writing starts, stop power cycling. Keep power steady.**

The script sends one erase/program command. It does not automatically retry writing or read flash back.

Wait for **WRITTEN: programmer reported Program success**. Our write took roughly **90–100 seconds**; others may take longer. “Connected” alone is not success.

If a write may have started and then fails, **do not immediately rerun it**. Keep logs and consult [troubleshooting](TROUBLESHOOTING.md).

If the script stops without ever connecting, no write was started. Check wiring,
power and drivers, then start the catch command again. Connection attempts time
out after five minutes. Logs are saved under **C:\ClockFlash\logs**.

## 5. Restart and set up

After WRITTEN, power off, unplug CKLink and remove its board connections. Restart normally.

Join **WiFi-Clock-Setup**, open **http://192.168.4.1/** and enter your Wi-Fi and clock settings. Old settings are cleared.

[User manual](WiFi-Clock-User-Manual-v2.0.pdf) · [Validation status](GETTING_STARTED.md#validation-note)
