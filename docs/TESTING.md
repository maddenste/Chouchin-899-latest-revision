# Validation

## HC32 V21

Physical daily-wake check, 8 October 2026:

- Woke at the scheduled 10:00.
- Red LED flashes followed by blue.
- No noticeable hand jump or calibration turn.
- Continued displaying the correct time.

This records the observed daily-wake behaviour on the owner's clock.

On 18 October the owner reported a week of stable running on the wall clock,
believed to be V21. Its installed image has not been read back, so that report
does not independently verify the release file's identity or every code path.

## HC32 V21 branch repair

The original V21 image contained `B.W 0x618` at flash address `0x4F50` (bytes
`FB F7 62 BB`). That Thumb-2 branch is unsupported by the HC32's Cortex-M0+.
Reaching it can fault; normal operation does not prove the path is safe.
A previous private fault capture identified this address as the saved PC.

The current V21 release includes the owner-tested **V21-R3-FIX**, retaining
the public filename `Chouchin-899-HC32-V21.hex`. It replaces that branch with
`LDR R3, [PC, #4]` followed by `BX R3`, plus the destination literal at
`0x4F58`. Exactly eight bytes change; thresholds, movement, homing, UART,
Wi-Fi scheduling and reference checks remain unchanged. No smoothing changes
are included. The [release identities](RELEASE_IDENTITY.md) distinguish old
and repaired downloads without renaming the existing release tag.

The owner installed `Chouchin-899-HC32-V21-R3-FIX.hex` and has now confirmed
it is working. This is owner hardware confirmation, not independent-user
validation or a newly measured battery-voltage threshold.

Offline checks replayed the captured fault through the repair, verified all
256 qualification-counter values and continuation register/flag equivalence,
and confirmed the exact eight-byte change. The Cortex-M0+ assembler rejects
the old instruction and accepts the repair. Unguarded Unicorn emulation
accepts the invalid branch even in its M0 model, so architecture verification
also uses an explicit instruction guard rather than relying on that model alone.

## Wi-Fi R17

Passed the target build and offline UART, NTP, settings, DST, reset and web
regression checks. The owner also relayed a user's email confirming successful
flashing and operation of the GitHub Wi-Fi release. That report covers the
Wi-Fi firmware only; it is not independent validation of HC32 V21 or every
Wi-Fi feature.

Tests are supplied under **firmware/tests/** and **tools/**.
Private HC32 binary emulation is not redistributed.

## Repository review — 18 October 2026

Reviewed the published `main` tree and V21 release downloads. Offline checks
passed for native protocol/UART, NTP, settings, application sequences, HTTP,
browser UI, launcher, image layout, backup-result classification and package
hygiene. Backend timezone tests covered 1,081,280 IANA comparisons through 2035.
Check that the compiled webpage matches its editable HTML without writing files:

```powershell
python .\firmware\tools\embed_web.py --check
```

The original HC32 HEX decoded to the exact original BIN; release checksums and the clean
Wi-Fi image layout match. This does not establish hardware correctness,
redistribution rights or a complete reproducible public cross-build.

## Final repository checks — 19 October 2026

Rebuilt and reran the portable C, app, UART, HTTP, NTP, settings and calendar
tests, including 40,000 event sequences, 10,002 timing/rollover cases and
1,081,280 timezone comparisons. Desktop/mobile browser tests and the embedded
webpage consistency check passed. Guided-launcher, direct-launch log defaults,
backup-result and image-layout tests passed without accessing hardware.

Checked the current V21 release's three assets and checksums against the
downloaded files. The controller and Wi-Fi images were not changed during
this review. These are offline regression checks, not a guarantee against
hardware, mechanical, power-supply or network faults.

All checked local documentation/image references and heading anchors resolved.
The project, release and vendor download URLs responded; the optional Time.is
reference blocked automated access. The three-page PDF manual was rendered
and visually checked.
