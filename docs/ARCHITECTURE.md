# Source map and design boundaries

Keep portable logic independent of SDK drivers. Preserve the known-working vendor
startup and board configuration; do not replace it with unrelated ESP startup code.

| Files | Responsibility |
| --- | --- |
| Vendor `main.c`, `syscfg.c`, `events.c` (not bundled) | Board/network startup and task/event integration in the private SDK build |
| `clock_app.c` in that project | Command state machine, settings ownership, mode switching, time-output scheduling |
| `clock_uart.c` | UART0 driver boundary, bounded receive/command handling |
| `clock_ntp.c` | Network-facing DNS/UDP worker and connectivity handling |
| root `clock_ntp_packet.c` | Portable NTP validation/time extraction |
| root `clock_calendar.c` | Local date/offset and POSIX/DST calculation |
| root `clock_psk.c` | WPA derivation, performed on credential changes rather than boot |
| root `clock_settings.c`, project `clock_storage.c` | Validation/CRC/version record and alternating flash-sector I/O |
| root `txw_hc32_protocol.c` | Exact line parser, responses, selector encoding and +TIME formatter |
| project `clock_web.c` | Bounded HTTP handling and JSON API |
| `web/index.html`, `tools/embed_web.py` | Editable page and generated read-only C asset |

The `clock_*` project entries in the table are relative to
`firmware/iot_sdk_work/clock_project`; portable root entries are relative to
`firmware/`. HAL test builds execute the actual app/web C with mocked SDK boundaries.

UART task loops every 20 ms and must remain responsive while the separate NTP
worker performs network requests and the web task derives changed WPA keys.
The settings lock protects state changes; cached PSK reuse avoids expensive boot
derivation. Credentials remain private in API responses. No logging should be
inserted into PA14's clock protocol stream in a release build.

## Network synchronisation

NTP replies may arrive from a different IPv4 address when a router transparently
redirects UDP port 123 to a local server. R4 accepts that source-address change,
but still requires source port 123, a matching echoed 64-bit request token,
NTPv3/v4 server mode, synchronized leap status, valid stratum and a plausible
timestamp. R5 starts the primary DNS lookup/request first and adds the secondary
after five seconds without valid time, retaining the primary request. One UDP
socket accepts the first valid correlated reply from either server. DNS uses
asynchronous, persistent callback storage, so pending lookups cannot block reply
reception or outlive stack storage. A round ends after ten seconds; failures
retry after one second. Invalid packets do not restart that deadline. These
checks correlate replies but do not
cryptographically authenticate the server. Redirected packets must still reach
the clock; firmware cannot repair a missing network return path.

All application settings are grouped in a 476-byte version-3 CRC-protected flash
record: Wi-Fi credentials/cached key, primary and secondary NTP hostnames,
timezone, daily update time and hand settings. The complete record alternates
between adjacent sectors 0x1FC000 and 0x1FD000; the second is a redundant copy,
not a separate settings category. The final sectors at 0x1FE000 and 0x1FF000
were retained in the private tested image but are blank in the clean public FULL.
No version-2 migration is implemented; incompatible records reset to
defaults. Keep future application settings in this grouped record.

## Web routes

GET `/` serves the embedded page. GET `/api/v1/config`, `/api/v1/status` and
`/api/v1/scan` return configuration/status/scan data. Authenticated scan start
uses `/api/v1/scan?start=1`. POST config saves settings; POST
`/api/v1/factory-reset`, `/api/v1/keepalive`, `/api/v1/session/close` handle reset
and portal-session lifetime. Reuse the page's per-boot request token and handler
validation rather than adding an unauthenticated write endpoint.

## DST scheduling

The current V21/R17 combination retains the DST wake planner, which changes only outgoing +TIME schedule fields.
On the last normal sync before a transition, it requests the needed
pre-change-clock minute (rounded up if seconds are present), only if it
precedes the next normal daily wake.
Following the transition, the original saved wake time is sent again. Fixed
offset rules (including Disabled) never create extra DST wakes. No flash write
or DST-specific WIFIAPPING is used. Forward/backward transitions have owner
confirmation, including non-hourly transitions in both directions. The backward
transition at 18:30 → 17:30 and a non-hourly forward transition are owner-confirmed.
Status exposes the next saved-rule DST transition once UTC is synchronized.
POST `/api/v1/dst-preview` calculates from unsaved timezone selections without
saving configuration, resetting the chip or extending the browser lease.
The UI debounces edits and rejects stale preview responses.

## Flash layout and controller responsibilities

Code begins at `0x0`; the boot wrapper's code offset is `0xC00` and load/run
address `0x18000000`. Application settings use `0x1FC000/0x1FD000`. Factory radio
configuration sectors are at `0x1FE000/0x1FF000`; the clean public image blanks
them instead of copying private board data. This division is specific to
the tested 2 MiB image/board; changing it requires a reviewed layout and recovery
plan. CRC is corruption detection, not confidentiality or authenticity.

HC32 owns physical hand movement, scheduled wake and power-off. TXW communicates
settings/time; do not duplicate motor timing in the Wi-Fi firmware. First diagnose
NTP/network reachability when AP works but time is absent, and diagnose HC32/state
when valid +TIME is present but the physical action is wrong.
