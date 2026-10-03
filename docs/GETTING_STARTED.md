# Start here — install WiFi Clock v2.0

This guide installs the prepared firmware. No reading firmware from the chip, editing or compiling is required.

## 1. Identify your board

Check both markings: **HC32L130J8TA** and **TXW813-320**.
[Chip photographs](HARDWARE.md#board-photographs).

**MM32/ESP boards require [the older project](https://github.com/maddenste/Chouchin-CH899-Firmware), not these files.**

## 2. Prepare

Follow [the equipment and software checklist](TOOLS.md).

We used a **3.3 V bench supply connected to the battery terminals**: positive to battery positive, negative to battery negative. Remove the batteries and check polarity. Do not connect programmer 3V3/5V outputs.

Programming replaces existing firmware. Keep any original backups you already have. Optional backup procedures are in [the background documents](BACKGROUND.md), outside this upload guide.

## 3. Upload HC32 V15

[Movement-controller upload steps →](HC32.md)

Use **Chouchin-899-HC32-V15-20261001.hex**, DAPLink UART pins and XHSC MCU Programmer.

## 4. Upload Wi-Fi R16

[Wi-Fi-controller upload steps →](FLASHING.md)

Use **WiFi-Clock-v2.0-R16-20261002_FULL.bin**, CKLink Lite and the power-on catch script. If a chip already has the correct version, leave it alone.

## 5. Set up

After successful flashing, power off, remove programming connections and restart normally.

1. Join **WiFi-Clock-Setup**.
2. Open **http://192.168.4.1/**.
3. Enter your Wi-Fi password and clock preferences.
4. Select **Save settings**. Once Wi-Fi and NTP synchronise, the clock receives the time.

The clean Wi-Fi image starts with no saved settings. See [the user manual](WiFi-Clock-User-Manual-v2.0.pdf) for calibration and everyday use.

### Validation note

HC32 V15 and Wi-Fi R16 functionality were tested on Steve's clock. The public Wi-Fi FULL removes old configuration sectors for a clean installation; that specific blank-configuration packaging has been checked offline but has not yet been hardware-tested. [Test record](TESTING.md).
