# Publication scope — 1 October 2026

Project: https://github.com/maddenste/Chouchin-899-latest-revision

The v2.0-rc1 candidate pairs HC32 V15 with WiFi Clock v2.0 R16 on the
HC32L130J8TA / TXW813-320 board. Firmware downloads are separate release assets.

## Repository scope

Authored application logic, UI, tests, helper scripts, synthetic screenshots and
guides are included. Vendor SDK trees, startup/integration overlays, project
metadata, executables and flash algorithms are not bundled. Published source is
not a complete reproducible cross-build environment. The build guide records the
tested private workflow, not a promise that this tree can build unaided.

Original backups, credentials, raw captures and factory-containing FULL images
remain private. Each owner must preserve their own final 8 KiB of TXW flash.

## Unresolved rights

The owner elected to publish compiled candidates while vendor redistribution
rights remain unconfirmed. Taixin permission was requested; reply pending.
HC32 underlying vendor-firmware rights are separate. No permission or legal
clearance is claimed. See [NOTICE](../NOTICE.md) and
[third-party inventory](../THIRD_PARTY_NOTICES.md).

## Evidence

Pin-label wiring and HC32 XHSC V2.23 UART programming settings were confirmed by
the owner. BOOT was held at 3.3 V throughout programming; clock board power came
from the bench supply. For SWD backup, DAPLink USB was plugged in after board
power-on. Original Keil SAVE commands are documented in the HC32 guide.
No physical header orientation is inferred from unavailable wiring photographs.

Additional hardware tests remain marked in [TESTING](TESTING.md).
