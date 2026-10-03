# Wi-Fi flashing — detailed help

[Simple flashing guide](FLASHING.md) · [Equipment and software downloads](TOOLS.md)

Use the **guided Windows launcher**: select the files, check the wiring and
follow its prompts. No commands to copy or paths to edit.
This installs Wi-Fi R17 on the **TXW813-320** only. Flash the
[HC32 movement controller separately](HC32.md) if needed.

## 1. Download and install

- Install **C-SKY DebugServer with the CKLink driver** and
  **C-SKY CDK/toolchain**, using [these software links](TOOLS.md#software-downloads).
- Download **WiFi-Clock-v2.0-R17-20261003_FULL.bin** from
  [the v2.0 release](https://github.com/maddenste/Chouchin-899-latest-revision/releases/tag/v2.0).
- Extract the Taixin package linked in the software list. You need
  **TXW81X_FLASH_ALGORITHM.elf** and its matching **.init** file under
  **sdk/chip/txw81x**. No SDK build is required.
- Download the [current project ZIP](https://github.com/maddenste/Chouchin-899-latest-revision/archive/refs/heads/main.zip)
  and **extract it**. Keep **Flash-WiFi.bat** beside its **tools** folder.
  The v2.0 release's **“Source code (zip)”** download also includes the launcher
  and can be used instead. The firmware BIN is a separate release download.

The downloads can remain in their usual folders, including folders with spaces.
The launcher copies the three flash inputs into a separate working folder with
simple paths; you do not need to reorganise them yourself.

## 2. Connect with board power off

Check the board markings: **HC32L130J8TA + TXW813-320**, not MM32/ESP.
Remove batteries. Use external regulated **3.3 V at the battery terminals**:
positive to battery **+**, negative to battery **−**. Check polarity.
See [power-supply safety](TOOLS.md#equipment).

![TXW813 signal connections](diagrams/txw-upload.svg)

| CKLink Lite pin | Board pad |
| --- | --- |
| TMS/IO | PA9 |
| TCK/CK | PA10 |
| GND | GND |

Leave all other probe pins disconnected, including **3V3, 5V, TDI, TDO and
nRST**. No BOOT/PA8 strap is used. Match pad names, not guessed header order.

<img src="photos/txw813-wifi-chip-and-debug-pads.jpg" alt="TXW813 and J1 GND, PA10, PA9, PA8, VCC pads" width="650">

Plug CKLink into USB. **Keep board power off for now.**
Close FlashProgrammer and all DebugServer windows.

## 3. Double-click Flash-WiFi.bat

The launcher finds tools in our usual installation locations and remembers
your selections for next time. If anything is missing, a file-selection window
asks you to locate it. Choose **csky-elfabiv2-gdb.exe**, not RISC-V GDB.

It selects your PC's active IPv4 network automatically. If there is more than
one, choose your normal Ethernet or Wi-Fi adapter, not a VPN. This is the
**PC's address**, not the clock's address.

The launcher checks the firmware and both algorithm checksums before offering
to start. If a check fails, **nothing is written**: correct the files first.
When everything is ready, press **Enter** to start, or type **Q** to quit.

If Windows blocks a downloaded file, inspect it, then use **Properties → Unblock**
on that file if offered. The launcher uses a process-only PowerShell execution
policy override; it does not change your system's execution policy. Organisation
security restrictions may still prevent it running—do not disable them.

## 4. Catch and write

1. When connection attempts begin, switch the board supply **on**.
2. If it has not connected, cycle board power while it retries.
3. The script automatically chooses **No** on the exact CKLink firmware-update
   prompt, polling every 25 ms. If it remains open, choose **No** manually.
4. **Once connected, stop power cycling and keep power steady.**

With the original firmware, PA10 quickly changes from debug clock to UART
debug output. The script must catch that short boot window. On our setup it
took **about five power cycles on average**; the connection-attempt counter
is not a power-cycle count. Our firmware leaves PA10 available for debug,
so this particular boot-window problem goes away after installation.

Wait for **WRITTEN: programmer reported Program success**. “Connected” alone
is not success. Our writes took roughly **90–100 seconds**; allow longer on
other setups. The script issues **one erase/program command**, with no automatic
write retry or flash readback.

If writing may have started and then fails, **do not immediately rerun it**.
Keep power steady, retain the logs and see [troubleshooting](TROUBLESHOOTING.md).
If it never connected, no write command was issued; check wiring, power and
drivers before retrying. Connection attempts time out after five minutes.

The window stays open after completion or failure. Each run has its own files
and logs under **C:\ClockFlash\run-…** by default; the launcher prints the exact
location. File selections are remembered under **%LOCALAPPDATA%\WiFiClockFlasher\paths.json**;
no passwords are stored. If the working folder cannot be created, select a
writable local folder with **no spaces in its path**. Run folders are retained
for diagnosis; no existing files are deleted.

## 5. Restart and set up

After **WRITTEN**, power off, unplug CKLink and remove its board connections.
Restart normally. Join **WiFi-Clock-Setup**, open **http://192.168.4.1/** and
enter your Wi-Fi and clock settings. The clean installation clears old settings.

[User manual](WiFi-Clock-User-Manual-v2.0.pdf) ·
[Manual PowerShell alternative](FLASHING_CLI.md) ·
[Validation status](GETTING_STARTED.md#validation-note)

The launcher has automated offline tests; its interactive file dialogs and
end-to-end hardware flashing have not yet been tested by another owner.
It uses the existing writer, not a new flash protocol.
Automatic dismissal applies only to the update prompt belonging to the
DebugServer process started by this script; other dialogs are left alone.
Manual users can opt out with **-ManualIcePrompt**. Failed connections can
restart promptly when DebugServer exits; the five-second per-attempt timeout
is only a limit for a stalled attempt, not a delay between every retry.
Launcher tests and the real R17 firmware/algorithm preflight also passed under
built-in Windows PowerShell 5.1. No PowerShell 7 installation is required.
