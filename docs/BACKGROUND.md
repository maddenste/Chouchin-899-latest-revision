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
- [Package contents](PACKAGE_CONTENTS.md)
- [Publication status](PUBLICATION_STATUS.md)
- [Release checklist](RELEASE_CHECKLIST.md)
- [Project notices](../NOTICE.md)

Investigation used Keil, pyOCD, PulseView UART captures and OPNsense DNS/NTP captures. These are not upload requirements. Vendor SDKs/tools, original firmware and private captures are not bundled.
