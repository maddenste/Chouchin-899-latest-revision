# Firmware reference

Current release: **HC32 V21 R3 — physically tested** (status updated 8 October
2026), with unchanged Wi-Fi **v2.0 R17**. V15 is retired and its release and
download assets have been removed. Historical test records remain in
[TESTING.md](TESTING.md) and [HISTORY.md](HISTORY.md).

The [V21 release](https://github.com/maddenste/Chouchin-899-latest-revision/releases/tag/nightly-hc32-v21-r3-20261007)
retains its original tag and NIGHTLY filenames so existing downloads can be
identified byte-for-byte. It is now the latest, non-prerelease download; this
status change does not rebuild or modify any firmware.
V1.0 refers to the older MM32/ESP board project, not this hardware revision.

| Released image | Size | SHA-256 |
| --- | --- | --- |
| Chouchin-899-HC32-V21-R3-NIGHTLY-20261007.hex | 180236 bytes | 12C82774B8C4E7205B484A2EB6930FC18F6E705172DEE53E2B03C10C9690ED18 |
| Chouchin-899-HC32-V21-R3-NIGHTLY-20261007.bin | 65536 bytes | 52788FB1D3DCC4B4671CE7011F67F61BBE0BAF53010CFDA9358149978CAAFB8A |
| WiFi-Clock-v2.0-R17-20261003_FULL.bin | 2097152 bytes | 1F4908F9ACC9160CDF53CA3F5C5BEF96CBF4C9197700C87640C01F13592CE9A9 |

Use the HC32 HEX for the documented XHSC UART procedure. The HC32 BIN is the
same 64 KiB image as raw binary, **not a Wi-Fi image**. SHA256SUMS.txt covers
all three downloads. Leave already-correct Wi-Fi firmware alone.

V21's observed scheduled 10:00 wake, red-then-blue LEDs, continued correct
time and absence of noticeable hand jumps/calibration turns passed the owner's
physical test on 8 October 2026. See [the test record](TESTING.md) for limits.
This report does not newly identify or validate the installed Wi-Fi image.

The Wi-Fi FULL contains the R17 APP at offset zero and FF throughout the
remainder, including the final 8 KiB. It contains no copied original data.
It is written directly at offset zero for a clean installation; enter Wi-Fi
settings after restarting. R17 passed the offline test/build pipeline but
has not been separately confirmed by this physical test.
Its existing status API string remains `TXW813 HC32-V15 R17`; that legacy
string is **not** proof of the movement-controller version. The footer
remains WiFi Clock v2.0. No Wi-Fi rebuild accompanies this release update.

Use the main-branch ZIP for current documentation and the guided
Flash-WiFi.bat launcher with its supporting scripts. The release's source
archive is the older publication snapshot. Download packaged firmware
separately from the release assets. Neither archive includes the omitted
vendor integration or makes the public tree independently rebuildable.
Do not substitute a raw linker binary for the packaged Wi-Fi image.

The legacy PDF user manual remains a historical v2.0 document; its V15
filename is obsolete. Use [GETTING_STARTED.md](GETTING_STARTED.md),
[HC32.md](HC32.md) and [USER_GUIDE.md](USER_GUIDE.md) for current instructions.
