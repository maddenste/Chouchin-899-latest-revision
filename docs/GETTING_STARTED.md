# Start here — install WiFi Clock v2.0

This guide installs the prepared firmware. No reading firmware from the chip, editing or compiling is required.

**Current release: HC32 V21 R3**, physically tested for scheduled daily wake on 8 October 2026. Read the [release notes and test scope](https://github.com/maddenste/Chouchin-899-latest-revision/releases/tag/nightly-hc32-v21-r3-20261007). V15 is retired and its release/downloads have been removed. Wi-Fi R17 is unchanged.

## 1. Identify your board

Check both markings: **HC32L130J8TA** and **TXW813-320**.
[Chip photographs](HARDWARE.md#board-photographs).

**MM32/ESP boards require [the older project](https://github.com/maddenste/Chouchin-CH899-Firmware), not these files.**

## 2. Prepare

Follow [the equipment and software checklist](TOOLS.md).

We used a **TENSTAR ROBOT TSP-03 fixed 3.3 V supply connected to the battery terminals**: +Vo to battery positive, −Vo to battery negative. Remove the batteries and check polarity. Do not connect programmer 3V3/5V outputs. See [the supply safety note](TOOLS.md#equipment); this is not a mains-wiring guide.

Programming replaces existing firmware. Keep any original backups you already have. Optional backup procedures are in [the background documents](BACKGROUND.md), outside this upload guide.

## 3. Upload HC32 V21 R3

[Movement-controller upload steps →](HC32.md)

Use **Chouchin-899-HC32-V21-R3-NIGHTLY-20261007.hex**, DAPLink UART pins and XHSC MCU Programmer.

## 4. Upload Wi-Fi R17

[Wi-Fi-controller upload steps →](FLASHING.md)

Use **WiFi-Clock-v2.0-R17-20261003_FULL.bin**, CKLink Lite and the double-click
**Flash-WiFi.bat** guided launcher. It checks the files and guides the power-on
catch procedure without commands to edit. If a chip already has the correct
version, leave it alone.

## 5. Set up

After successful flashing, power off, remove programming connections and restart normally.

1. Join **WiFi-Clock-Setup**.
2. Open **http://192.168.4.1/**.
3. Enter your Wi-Fi password and clock preferences.
4. Select **Save settings**. Once Wi-Fi and NTP synchronise, the clock receives the time.

The clean Wi-Fi image starts with no saved settings. See [the current user guide](USER_GUIDE.md) for calibration and everyday use.

### Validation note

The current download pair is **HC32 V21 R3 + Wi-Fi R17**. V21 R3 passed the owner's scheduled 10:00 daily-wake test: red then blue LEDs, no noticeable hand jump or full-turn calibration, and correct time continued. This is physical validation of that observed behaviour, not every setting or long-term reliability. The unchanged NIGHTLY filename identifies the tested binary. Earlier Wi-Fi builds passed
owner tests including a 48-hour run; R17 passed the full offline test/build
pipeline but has not yet been tested on the clock. The clean Wi-Fi image starts
with no saved settings. [Test record](TESTING.md).
