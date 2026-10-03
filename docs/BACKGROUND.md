# Technical background

These documents are not needed to upload the prepared firmware.
[Return to the installation walkthrough](GETTING_STARTED.md).

## Research and hardware

- [Board photographs and additional interfaces](HARDWARE.md)
- [The development journey](HISTORY.md)
- [HC32 backup using Keil](HC32_BACKUP.md)
- [UART protocol](PROTOCOL.md)
- [Architecture](ARCHITECTURE.md)

The TXW read tool remains in tools/Read-TXW813-OnPower.ps1 for experienced users. Reading, building and editing are separate from the upload guide.

## Tools at a glance

| Tool in tools/ | Role |
| --- | --- |
| Program-TXW813-DCDC0-WriteOnly-OnPower.ps1 | Wi-Fi upload; the only script needed by the installation guide |
| Read-TXW813-OnPower.ps1 | Advanced original-firmware backup and diagnosis |
| verify_hc32_backup.py | Offline validation of exported HC32 backups |
| Prepare-Sdk.ps1 | Development SDK preparation; does not restore omitted integration |
| Build-TXW813-DCDC0FullImage.ps1 | Development image assembly retaining a private board's final sectors |
| New-TXW813-PublicFullImage.ps1 | Offline clean-image packaging for the released R16 APP |
| Test-PublicPackage.ps1 | Repository hygiene, syntax and local-link checks |

## Development and validation

- [Build notes and source scope](BUILD.md)
- [Tests and outstanding checks](TESTING.md)
- [Release identities](RELEASE_IDENTITY.md)
- [Project notices](../NOTICE.md)

Investigation used Keil, pyOCD, PulseView UART captures and OPNsense DNS/NTP captures. These are not upload requirements. Vendor SDKs/tools, original firmware and private captures are not bundled.

## TXW SDKs used — building versus uploading

The Wi-Fi application was built using **TXW81x_IOT-v2.5.2.6-31320**.
See [the build notes](BUILD.md) for its role in development.

Uploading used **TXW81X_FLASH_ALGORITHM.elf** and its matching **.init** from
**TXW81x_FPV-v2.5.4.7-45354**, not the IOT package's algorithm files.
The original writer names the FPV paths; the current private dependency copies
match those FPV files byte-for-byte. The successful R16 upload log records
programming with that copied algorithm.

The IOT and FPV algorithm pairs have different SHA-256 hashes. The public
upload script checks the FPV pair, so the IOT files are not interchangeable
in the documented procedure. Neither SDK is redistributed here. Users uploading
the prepared image do not need the IOT build SDK.
