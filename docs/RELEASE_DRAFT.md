# WiFi Clock v2.0 — Public release 1

The first public firmware download for this board revision. V15 and R16 are
internal development build identifiers, not public release numbers.

For **Chouchin-899 with HC32L130J8TA + TXW813-320** only.
Not compatible with the older MM32/ESP board.

## Install

**[Start here: equipment, software downloads and step-by-step upload guide](https://github.com/maddenste/Chouchin-899-latest-revision/blob/main/docs/GETTING_STARTED.md)**

We used Windows and a **3.3 V external supply connected to the battery terminals**.
Install the prepared files using the documented UART and CKLink procedures; no compiling is required.

After Wi-Fi flashing, enter your settings: join **WiFi-Clock-Setup**, open **http://192.168.4.1/** and select **Save settings**.

## Downloads

- **Chouchin-899-HC32-V15-20261001.hex** — complete 64 KiB movement firmware.
- **WiFi-Clock-v2.0-R16-20261002_FULL.bin** — complete 2 MiB clean Wi-Fi image.
- **WiFi-Clock-User-Manual-v2.0.pdf** — setup and operation.
- **SHA256SUMS.txt** — separate checksums.

No original firmware or saved Wi-Fi credentials are included.

## Changelog — improvements over the original firmware

### Time synchronisation and waking

- Two configurable NTP servers instead of relying on one. After five seconds,
  the second server is also tried while replies from the first remain eligible;
  the first valid reply wins.
- Improved DNS handling for redirected replies, plus support for correlated
  NTP replies when a local router redirects the response source address.
- Daily update time selectable in ten-minute steps, rather than only on the hour.
- Temporary DST update wakes, including non-hourly changes, without overwriting
  the saved daily update time. Forward and backward changes are supported.
- Automatic, Disabled and custom POSIX DST settings, with a live next-change
  preview based on the current form selections. Disabled DST adds no special wake.

### Hand movement and battery saving

- Configurable Sweep, Burst and Hold&Start modes, carried in the extended +TIME
  message and interpreted by the movement controller.
- Hold&Start provides Swiss railway station-clock-style motion: seconds wait
  at 12, then restart as the minute hand advances.
- Second hand battery saver: Off, Night parking (00:00–06:00), or On.
  On lets the second hand reach 12 and remain parked indefinitely, useful when
  no second hand is fitted; the minute hand continues normally.
- Fixed the overnight restart behaviour while retaining the original battery
  protection checks.
- Extended the HC32 parser to accept the movement/battery-saver selectors and
  minute-precision wake times.

### Wi-Fi setup and web page

- Rebuilt local settings page with a restrained glass-style theme, grouped
  controls and clearer status/save messages.
- Available-network selection displays the saved SSID; manual SSID entry appears
  only when Manual is selected.
- Both NTP fields use the same input validation.
- Settings stored together in the application's flash settings area.
- Browser keep-alive sends WIFIAPPING in both setup/AP and station modes, so
  opening the settings page can keep the Wi-Fi chip powered.
- Setup AP named WiFi-Clock-Setup; DHCP hostname WiFi-Clock- followed by the final
  six MAC hex digits in capitals.
- +TIME starts after valid NTP synchronisation and repeats while the time-output
  state permits and the Wi-Fi chip remains powered.

### Uploading and documentation

- PA10 remains available for debug instead of being repurposed for UART debug
  output, removing the original firmware's short power-on debug window after
  this firmware is installed.
- Complete prepared firmware images for both chips; the clean Wi-Fi image has
  no saved credentials and requires settings to be entered after flashing.
- Windows upload walkthrough, software/hardware links, board photographs and
  wiring diagrams, plus a user manual and separate checksums.

This renaming does not change the firmware bytes. Download filenames retain
their internal build IDs so existing instructions and checksums remain valid.

The clean FULL's layout and installer preflight are checked offline; its blank old-configuration sectors have not yet been hardware-tested. [Test record](https://github.com/maddenste/Chouchin-899-latest-revision/blob/main/docs/TESTING.md).
