# Start from scratch

This guide applies only to the newer Chouchin-899 with **HC32L130J8TA +
TXW813-320**. Read [hardware identification](HARDWARE.md) first.

This describes the tested local workflow. Download the prebuilt candidate pair
from the project's v2.0-rc1 release. The published source omits vendor startup/build
integration and is not a complete reproducible cross-build environment.

## 1. Identify and document your board

Remove batteries and disconnect power before soldering. Photograph chip markings,
both PCB faces and your connections. Keep the mechanism safe from accidental
movement. Do not infer compatibility from “899” on the case.

You need a CKLink Lite for TXW two-wire C-SKY debug, a separate ARM SWD probe
for HC32, stable board-compatible power, fine soldering tools and a multimeter.
A logic analyser makes UART validation much easier. See [tools](TOOLS.md).

## 2. Connect TXW debug

Owner-confirmed connections are CKLink **TMS/IO → TXW PA9** and
**TCK/CK → TXW PA10**, with common ground. Do not connect a 5 V output.
Determine the probe's voltage-reference arrangement from its actual schematic;
a pin marked 3V3 is not automatically a VREF input. Do not parallel two supplies.

Leave PA9/PA10 available for debug in replacement firmware. TXW UART uses
PA13 RX / PA14 TX at 9600 8N1. HC32 PA13/PA14 are different physical pins.
See the [full wiring cautions](HARDWARE.md).

## 3. Preserve recovery material

Before any writes, obtain two independent full TXW reads, compare SHA-256 hashes,
and preserve a 64 KiB original HC32 dump using the appropriate SWD tools.
Record board identity, wiring and tools with the hashes. A correctly sized
all-FF file is not a useful original backup.

[FLASHING.md](FLASHING.md) gives the TXW catch-at-power-on commands.
[HC32.md](HC32.md) explains the movement-controller limitations and installation.

## 4. Build your own Wi-Fi image

Use the release's packaged APP and the image assembly workflow in
[FLASHING.md](FLASHING.md). Keep your own factory backup private. The full TXW
image must preserve **your board's** final 8 KiB, not Steve's or another owner's.
Obtain external dependencies separately. [BUILD.md](BUILD.md) records the local
build process and explains the missing public build integration.

## 5. Install the compatible pair

The current candidate pairing is **HC32 V15 + Wi-Fi v2.0 R16**.
See the test ledger for the revision associated with each hardware observation.
Follow the chip-specific guides; never use ESP or MM32 tools/images on these
chips. Run writer preflight without `-Program` first. The explicit `-Program`
switch performs one full-image write with no flash readback.

Keep power steady after the write starts. An ambiguous result is a diagnostic
stop, not permission to retry or erase again.

## 6. Configure and verify

Power-cycle after programming has completed. Join **WiFi-Clock-Setup**, open
`http://192.168.4.1/`, and save network, NTP and movement settings.
In station mode use the DHCP-assigned IP. See [USER_GUIDE.md](USER_GUIDE.md).

Passively capture TXW PA14: expect time messages once NTP synchronises and
WIFIAPPING while the page is active. Check motor movement, saving mode, daily
wake and DST against [TESTING.md](TESTING.md). Do not declare battery performance
proven from a bench supply test.
