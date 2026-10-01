# Release-candidate test ledger

Current pair: **HC32 V15 + Wi-Fi v2.0 R16**. The human-facing v2.0 footer is not a
complete binary identity: retain build ID and SHA-256. Programmer-reported
success is not independent flash readback; R13–R16 writes were write-only.
V15 installation was reconfirmed by the owner. R16 completed the full test/build
pipeline and the programmer reported success on 1 October 2026. Changes from
R14 are presentation/build identity only. Hardware observations below retain
the actual revision on which they were reported.

## Owner-reported hardware tests, 1 October 2026

| Check | Result / context |
| --- | --- |
| AP/page, station, NTP and +TIME | Working candidate baseline |
| Sweep/Burst/Hold&Start and live setting changes | Working |
| Night parking and continuously On parking | Working on V15 candidate |
| 17:10 daily wake | Woke/synchronised on V15/R13 |
| Cold-start settings persistence | Confirmed in owner's grouped tests 1–5 on V15/R13 |
| Primary unavailable, secondary fallback | Confirmed in grouped tests 1–5 |
| Wi-Fi unavailable and later recovery | Confirmed in grouped tests 1–5 |
| Page keep-alive / closing-page power-down | Confirmed in grouped tests 1–5 |
| Parking entry/restart/release | Confirmed in grouped tests 1–5 |
| Forward DST 17:00 → 18:00; normal daily time still 23:00 | Explicitly confirmed on V15/R13 |
| R14 live preview / branded page | Owner reports “works lovely” after upload |
| Backward DST 18:30 → 17:30 | Explicitly confirmed after R14 installation |
| Non-hourly DST transition | Pending |
| Disabled DST boundary test | Pending |
| 48-hour soak | Pending |
| Genuine low-battery threshold/recovery | Not established by 3.3 V bench-supply tests |

Grouped reports are recorded as such; no unprovided waveform, timing measurement
or battery-current result is invented. Repeat essential checks on R16 before final
release. One board's results do not establish compatibility with every production revision.

## Offline checks completed on R14

Real C tests cover protocol, all 12 selectors, WPA key vectors, grouped settings/
corruption, application state, DNS/NTP concurrency/redirection/deadlines, HTTP
fragmentation/validation/privacy, and read-only DST preview. Browser tests exercise
desktop/mobile, unsaved edits, invalid rules, Disabled, saved/manual SSID and
save/reset behavior. Calendar testing includes 40 presets/80 rules and 1,081,280
IANA comparisons through 2035.

Private HC32 machine-code tests include all 144 ten-minute daily choices through
the C formatter, V15 receiver and wake scheduler. Original/vendor-containing
controller dumps are not distributed to make those tests run publicly.
C-Sky compilation, packaging and full-image layout checks pass.

On 1 October 2026 the reorganised E: development copy passed the full pipeline
and reproduced the installed R14 image hash. A clean public-draft copy prepared
with the external SDK also passed. Its raw code differed only in five characters
of the SDK build timestamp; neither validation build was flashed.

These tests mock hardware boundaries: they are not RF, motor, battery-life or
post-program readback verification.

## DST test method

Use normal NTP and a temporary Custom POSIX rule. Pick today's month/week/weekday
and a future transition, verify the preview, set daily update after it, save and
allow sync. **Close the webpage** so WIFIAPPING cannot mask the scheduled wake.
Record real UTC, local pre/post time, requested wake and restored daily setting.
Forward transitions are expressed on standard time; backward on daylight time.

For non-hourly testing, choose a minute such as :15. HC32 accepts this temporary
DST minute even though normal webpage daily choices are multiples of ten.
For Disabled, verify no DST override is sent and ordinary daily waking continues.
Restore normal rules/NTP/daily time after each test; old dated examples cannot
be reused unchanged on another date.

## Soak

Leave the unchanged candidates running for 48 hours. Check daily wakes, hand
alignment, unexpected resets, Wi-Fi power-down/session expiry and recovery.
A bench supply does not establish battery lifetime or low-battery behavior.
Retain original battery checks and document a separate appropriate battery test.
Redact credentials and factory/MAC data before sharing evidence.
