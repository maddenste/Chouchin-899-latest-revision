# HC32 / TXW UART protocol

Target: HC32 V15 + Wi-Fi v2.0 R16. TXW UART0 PA13 RX / PA14 TX, **9600 8N1**, CRLF.
The real C formatter/parser and tests are authoritative.

| HC32 request | TXW response / action |
| --- | --- |
| WIFIID | WIFIIDOK with saved credentials; otherwise WIFIIDNC |
| WIFIAP | WIFIAP OK; request AP mode |
| WIFISTA | WIFISTA OK; request station mode |
| WIFIRESET twice | WIFIRESET OK; clear settings when requests are 750–4000 ms apart |

Spaces in “ OK” responses matter. Successful page save/switch also sends WIFIAPPOK.
Acknowledgment does not prove DHCP, DNS or NTP success. Early startup commands
are deferred; repeated requests do not repeatedly restart the interface.

## Keep-alive

WIFIAPPING is sent while a browser lease is active in AP or station mode.
The lease expires after 15 seconds. Inactive AP sessions send WIFIEXIT;
station expiry stops keep-alive without adding WIFIEXIT. HC32 decides final power-off.

## +TIME

After valid NTP, +TIME starts immediately and repeats once per second while the
station/time-output state permits it and TXW remains powered:

```text
+TIME:Thu Oct  1 17:09:56 2026 +0100 17:10 30\r\n
```

Backslashes here represent actual CR/LF bytes. Single-digit days are space-padded.
The signed UTC offset is followed by HC32 wake HH:MM and **two ASCII selectors**.

| First digit | Minute | Seconds |
| --- | --- | --- |
| 0 | Gradual | Continuous |
| 1 | Jump | Continuous |
| 2 | Gradual | Pause at 12 |
| 3 | Jump | Pause at 12 |

| Second digit | Second-hand battery saving |
| --- | --- |
| 0 | Off |
| 1 | Night parking, 00:00–06:00 |
| 2 | Reach 12 then stay parked continuously |

Default **00**: Sweep / Off. Hold&Start / Off is **30**.
V11 introduced the movement/night selectors, V14 added continuous parking and V15
uses the wake minute. Stock/older controller firmware is not assumed compatible.

For DST, only outgoing HH:MM is temporarily overridden. Use the HC32's pre-change
clock for the wake; after corrected time, restore the saved schedule. Exact-minute
transitions remain exact; second-bearing transitions round up to the next minute.
Disabled/fixed-offset zones produce no extra DST wake. The stored record is unchanged.

Record analyser rate and direction; a trace ending is not proof of power-off.
Never add release debug logging on PA14's clock protocol stream.
