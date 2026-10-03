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
