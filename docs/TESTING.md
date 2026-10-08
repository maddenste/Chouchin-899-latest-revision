# Validation

## HC32 V21

Physical daily-wake check, 8 October 2026:

- Woke at the scheduled 10:00.
- Red LED flashes followed by blue.
- No noticeable hand jump or calibration turn.
- Continued displaying the correct time.

This records the observed daily-wake behaviour on the owner's clock.

## Wi-Fi R17

Passed the target build and offline UART, NTP, settings, DST, reset and web
regression checks. Separate hardware validation of R17 remains outstanding.

Tests are supplied under **firmware/tests/** and **tools/**.
Private HC32 binary emulation is not redistributed.
