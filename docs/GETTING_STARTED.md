# Start here — install WiFi Clock v2.0

This guide installs the prepared firmware. No reading firmware from the chip, editing or compiling is required.

Downloads: **[HC32 V21 + Wi-Fi R17](https://github.com/maddenste/Chouchin-899-latest-revision/releases/tag/v21)**.

## 1. Identify your board

This guide is for the **HC32 board**. Look for **HC32L130J8TA** on the movement-control chip and **TXW813-320** on the Wi-Fi chip. [Compare with the photographs](HARDWARE.md#board-photographs).

If your clock has the **MM32 board**, use [WiFi Clock v1.5 for movement styles and battery saving](https://github.com/maddenste/Chouchin-CH899-MM32-ESP8285), or [the Wi-Fi-only upgrade to keep the factory hand movement](https://github.com/maddenste/Chouchin-CH899-Firmware). Each has its own installation guide; these HC32 files will not work on that board.

## 2. Prepare

Follow [the equipment and software checklist](TOOLS.md).

We used a **TENSTAR ROBOT TSP-03 fixed 3.3 V supply connected to the battery terminals**: +Vo to battery positive, −Vo to battery negative. Remove the batteries and check polarity. Do not connect programmer 3V3/5V outputs. See [the supply safety note](TOOLS.md#equipment); this is not a mains-wiring guide.

Programming replaces existing firmware. Keep any original backups you already have. Optional backup procedures are in [the background documents](BACKGROUND.md), outside this upload guide.

## 3. Upload HC32 V21

[Movement-controller upload steps →](HC32.md)

Use **Chouchin-899-HC32-V21.hex**, DAPLink UART pins and XHSC MCU Programmer.

## 4. Upload Wi-Fi R17

[Wi-Fi-controller upload steps →](FLASHING.md)

Use **WiFi-Clock-v2.0-R17-20261003_FULL.bin**, CKLink Lite and the double-click
**Flash-WiFi.bat** guided launcher. It checks the files and guides the power-on
catch procedure without commands to edit.

## 5. Set up

After successful flashing, power off, remove programming connections and restart normally.

1. Join **WiFi-Clock-Setup**.
2. Open **http://192.168.4.1/**.
3. Enter your Wi-Fi password and clock preferences.
4. Select **Save settings**. Once Wi-Fi and NTP synchronise, the clock receives the time.

The clean Wi-Fi image starts with no saved settings. See [the current user guide](USER_GUIDE.md) for calibration and everyday use.
