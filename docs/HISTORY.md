# The journey

## Why a second project?

The older Chouchin-899 project targets MM32 + ESP hardware. This newer unit
uses HC32L130J8TA + TXW813-320, so an ESP binary could not be reused.
The useful part to port was the behaviour: setup web page, NTP/timezone backend,
and the UART contract with the movement controller.

## Catching the original TXW

DebugServer could identify CKLink, but repeated target checks returned invalid
HID values and failed debug-register reads. Recognising the USB probe did not
prove the CPU was connected. Firmware was apparently disabling/reassigning
debug access after boot, so a retrying power-on catch workflow was developed.

The script retries connections while the owner applies board power. It is not a
hardware-triggered logic-analyser script: no direct spike detection is involved.
Debug IO is on PA9 and clock on PA10, confirmed by the owner.

Initial GDB connections did not immediately yield flash: the correct SDK init
and matching flash algorithm were needed. Two original 2 MiB reads eventually
matched the same SHA-256, providing the recovery baseline.

## Building replacement firmware

Early slot programming, flash-driver failures, missing AP and missing UART led
to simplifying the application rather than continuing to imitate every stock
internal detail. Board-specific startup, DCDC configuration, RAM assumptions
and packaged flash format mattered. The resulting application uses the vendor
SDK for hardware support and project code for the clock backend and interface.

DNS/NTP was diagnosed separately from basic Wi-Fi: literal server IPs worked
while external names failed. OPNsense redirection captures helped reproduce
the problem. The revised backend allocates adequate DNS/UDP resources and
validates correlated NTP replies without insisting on the unredirected source IP.

## Working clock behaviour

The replacement sends +TIME after NTP and continues while powered. WIFIAPPING
was essential to prevent the HC32 switching the Wi-Fi chip off while the browser
was open. Hand movement selectors, selectable parking, indefinite saving and
minute-resolution wake support required compatible HC32 patches.

The web page was refined with a restrained glass-like theme, saved SSID display,
two racing NTP servers, concise movement/saving controls and custom POSIX rules.
DST preview now recalculates from unsaved settings without writing them.

## First public release

**WiFi Clock v2.0 — Public release 1**, containing internal builds
**HC32 V15 + Wi-Fi R16**, branded
**WiFi Clock · v2.0 — Modified by Steve Madden · 2026**.
V1.0 is the older board revision. R15/R16 refined labels and attribution without
changing clock behavior; the footer links to the new project.
The owner confirmed a 17:10 wake and forward/backward DST tests.
This is a successful test programme on one board, not proof of compatibility
with every “899” clock. Outstanding tests and rights questions are listed in
[TESTING.md](TESTING.md) and [NOTICE](../NOTICE.md).

## Wi-Fi R17 maintenance — 3 October 2026

The public version remains **v2.0**. Wi-Fi R17 replaces the Wi-Fi download;
the HC32 V15 firmware is unchanged.

- Reset-command timing now uses UART arrival timestamps, including when
  commands wait in the receive queue. Saving settings cancels an armed reset
  and prevents older queued reset commands from erasing the new settings.
- AP/station acknowledgements reflect successful interface startup. Failed
  startup can retry, and a newer mode command cancels older pending mode work.
- Reset renews the setup AP grace period and clears stale browser keep-alive
  state. Time output pauses during reset and after saving until the new NTP
  session synchronises, preventing stale time messages.
- Includes the custom-DST fixes for transitions crossing a year boundary and
  offsets outside the UART time format, plus intact NTP hostname responses
  up to the supported 127-character limit.

Automated UART, reset/mode-state, settings, DST, NTP and web tests passed, as
did the target build and image checks. R17 has not yet been bench-tested on
the clock; the earlier owner confirmations apply to the preceding build.

Historical captures, failed builds and recovery logs are preserved privately.
They should not be mixed into a public source repository or described as
working release images.
