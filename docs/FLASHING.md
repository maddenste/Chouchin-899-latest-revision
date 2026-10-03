# Flash the Wi-Fi chip

For **TXW813-320 + HC32L130J8TA boards**, using Windows and CKLink Lite.
The [HC32 chip has separate upload instructions](HC32.md).

## Download the launcher

**[Download flashing launcher and scripts (ZIP)](https://github.com/maddenste/Chouchin-899-latest-revision/archive/refs/tags/v2.0.zip)**

Includes **[Flash-WiFi.bat](../Flash-WiFi.bat)** and its required scripts.
**Extract the ZIP**—do not download the BAT on its own.

## 1. Get the software and firmware

Install **C-SKY DebugServer/driver and CDK** from the
[software download list](TOOLS.md#software-downloads).
Keep the Taixin **TXW81X_FLASH_ALGORITHM.elf** and matching **.init** files.
The launcher uses Windows' built-in PowerShell; no PowerShell installation is needed.

**[Download Wi-Fi R17 firmware (BIN)](https://github.com/maddenste/Chouchin-899-latest-revision/releases/download/v2.0/WiFi-Clock-v2.0-R17-20261003_FULL.bin)**

## 2. Connect with power off

Remove batteries. Connect a regulated **3.3 V supply to the battery terminals**,
positive to **+**, negative to **−**. Check polarity.

| CKLink Lite | Board |
| --- | --- |
| TMS/IO | PA9 |
| TCK/CK | PA10 |
| GND | GND |

Leave **all other probe pins disconnected**. No BOOT strap is needed.
Plug CKLink into USB, but keep board power off.

<details>
<summary>Show wiring diagram and board photograph</summary>

![TXW813 wiring](diagrams/txw-upload.svg)

<img src="photos/txw813-wifi-chip-and-debug-pads.jpg" alt="TXW813 board pads: GND, PA10, PA9, PA8 and VCC" width="650">

</details>

## 3. Run Flash-WiFi.bat

Close FlashProgrammer and all DebugServer windows.
Double-click **Flash-WiFi.bat** in the extracted folder.

Select any files it asks for. The launcher checks them and guides you through
starting. No commands or paths to edit.

- When connection attempts begin, turn board power **on**.
- If it has not connected, cycle board power. Around **five cycles** was typical.
- The script automatically declines the CKLink update prompt. If it stays open, choose **No**.
- **Once connected, stop cycling power and keep power steady.**

Wait for **WRITTEN: programmer reported Program success**—usually around
90–100 seconds on our setup. There is no automatic write retry or flash readback.

If writing fails, **do not immediately retry**. Keep the logs and follow
[troubleshooting](TROUBLESHOOTING.md).

## 4. Restart and set up

After **WRITTEN**, power off and remove the probe connections. Restart normally.
Join **WiFi-Clock-Setup**, open **http://192.168.4.1/** and save your settings.
Old settings are cleared by this installation.

[Detailed help](FLASHING_HELP.md) · [Manual command-line alternative](FLASHING_CLI.md) ·
[User manual](WiFi-Clock-User-Manual-v2.0.pdf)
