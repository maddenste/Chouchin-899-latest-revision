# Flash the Wi-Fi chip

For **TXW813-320 + HC32L130J8TA boards**, using Windows 11 and CKLink Lite.
The [HC32 chip has separate upload instructions](HC32.md).

Prefer commands? Use the [manual CLI flashing guide](FLASHING_CLI.md).

## Download the launcher

**[Download current flashing launcher and scripts (ZIP)](https://github.com/maddenste/Chouchin-899-latest-revision/archive/refs/heads/main.zip)**

Includes **[Flash-WiFi.bat](../Flash-WiFi.bat)** and its supporting scripts for guided
TXW813 Wi-Fi flashing, with file selection, automatic checks and power-on instructions.
**Extract the ZIP and keep its folder structure intact**—do not download the BAT on its own.
Download the flashing software and firmware separately in step 1 below.

## 1. Get the software and firmware

1. Install **C-SKY DebugServer with the CKLink driver**, and **CDK**, which supplies the debugger used by the launcher.
2. Download and extract the **Taixin package**. You only need `TXW81X_FLASH_ALGORITHM.elf` and `TXW81X_FLASH_ALGORITHM.init` from its `sdk/chip/txw81x` folder. Nothing needs compiling.
3. Download the **Wi-Fi firmware BIN** linked below.

Get the tools from the [software downloads guide](TOOLS.md#software-downloads).
PowerShell is already included with Windows 11.

**[Download unchanged Wi-Fi R17 firmware (BIN)](https://github.com/maddenste/Chouchin-899-latest-revision/releases/download/nightly-hc32-v21-r3-20261007/WiFi-Clock-v2.0-R17-20261003_FULL.bin)**

## 2. Connect with power off

Remove batteries. Connect a regulated **3.3 V supply to the battery terminals**,
positive to **+**, negative to **−**. Check polarity and leave the supply off.

Leave **all other probe pins disconnected**. No BOOT strap is needed.
Plug CKLink into USB, but keep board power off.

![TXW813 wiring](diagrams/txw-upload.svg)

<img src="photos/txw813-wifi-chip-and-debug-pads.jpg" alt="TXW813 board pads: GND, PA10, PA9, PA8 and VCC" width="650">

## 3. Run Flash-WiFi.bat

Close FlashProgrammer and all DebugServer windows.
Double-click **Flash-WiFi.bat** in the extracted folder.

Select files when prompted. The launcher checks them; press **Enter** when
wired and ready. No commands or paths to edit.

- When connection attempts begin, turn board power **on**.
- If it has not connected, cycle board power. About **five power cycles** was typical.
- The script automatically declines the CKLink update prompt. If it stays open, choose **No**.
- **Once connected, stop cycling power and keep power steady.**

Wait for **WRITTEN: programmer reported Program success**. Writing took around
**90–100 seconds** on our setup; other setups may take longer.

**Red LED warning:** the HC32 controls this LED, so it can keep flashing during
writing or after a missed boot window. **Follow the script's messages, not the
LED. Do not cycle power or unplug the probe during writing.**

To stop a repeating update-window loop **before writing starts**, unplug the
probe from USB, then close the flashing window.

If writing fails, **do not immediately retry**. Keep the logs and follow
[troubleshooting](TROUBLESHOOTING.md). There is no automatic write retry or
flash readback.

## 4. Restart and set up

After **WRITTEN**, power off and remove the probe connections. Restart normally.
Join **WiFi-Clock-Setup**, open **http://192.168.4.1/** and save your settings.
Old settings are cleared by this installation.

[Detailed help](FLASHING_HELP.md) · [Manual command-line alternative](FLASHING_CLI.md) ·
[Current user guide](USER_GUIDE.md)
