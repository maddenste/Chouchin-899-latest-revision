# Source-only publication package

The prepared package contains documentation, our own application code, tests,
web assets and helper scripts. Firmware downloads are separate release assets,
not stored in Git history. This is not a complete cross-build environment.

## Included

- README, GPL licence, attribution and third-party/publication notices.
- Hardware, tools, UART, startup/build/flash, operation and troubleshooting guide.
- Historical project journey, actual test ledger and pending-release checklist.
- Desktop/mobile screenshots using synthetic network settings.
- Three-page PDF user manual for WiFi Clock v2.0 on this board revision.
- Portable calendar, NTP validation, grouped-settings, WPA-key and UART logic.
- Clock application, UART, storage, NTP and HTTP modules, plus the editable UI.
- C/Python/browser tests with mocked hardware boundaries.
- SDK preparation, image assembly, power-on catch/read and one-shot writer scripts.
- Build script as a record of the local workflow; omitted integration is required.
- Local package checker for exclusions, common secret patterns, script syntax
  and relative Markdown links. It does not certify legal clearance or detect
  every possible secret.
- Offline HC32 HEX backup verifier; prepared release description and handoff.

## Not included

- SDK, vendor libraries, startup/integration overlays and vendor build metadata.
- Vendor tools, flash algorithm ELF/init, compiler or developer runtimes.
- APP/FULL/HEX/ELF images, original dumps, radio/factory data or saved credentials.
- Personal UART/network captures or private investigation logs.

The firmware modules require the SDK and board-specific integration. Excluding
the SDK is not enough to make an otherwise incomplete tree buildable; see
[publication status](PUBLICATION_STATUS.md) and [build scope](BUILD.md).

The existing development and review trees are retained privately, unchanged
by package staging. Do not upload those trees in place of this allowlisted one.

## Verify before upload

From the source-only package root, run in PowerShell 7:

```powershell
.\tools\Test-PublicPackage.ps1
```

Inspect the complete file list and screenshots before committing.

## Release assets

The release has two firmware downloads: clean Wi-Fi R16 FULL and HC32 V15 HEX,
plus the user manual and a separate SHA256SUMS.txt. Notices remain in this
repository. Flash the full images and enter Wi-Fi settings afterwards.
No original firmware or private backup is supplied.
