# Troubleshooting

| Symptom | First checks |
| --- | --- |
| No ICE / CKLink connection failed | Probe USB cable, driver/device recognition, no other DebugServer/FlashProgrammer process |
| Probe identified, HID all FF / target check fails | Board power, common ground, actual PA9/PA10 wiring, catch timing; probe recognition alone is not target access |
| Optional ICE firmware update appears | Choose No for the documented working workflow; an upgrade failure is not a chip firmware update |
| Writer blocked by busy process | Close the named programmer/debugger windows; inspect process identity before stopping it |
| No flash algorithm file | Supply matching algorithm ELF and init file from the required SDK |
| Dump failed / Rcmd error | Read the attempt logs, confirm init and connection stability; do not jump straight to erase |
| All-FF output | Check region and initialisation; an erased slot is not a recovered whole-firmware backup |
| GDB prompt cannot type | Focus its console and leave text-selection mode with Esc; do not reset the board merely to restore typing |
| Write running for minutes | Keep power steady and inspect progress logs; timeout alone does not prove a clean failure |
| Write result ambiguous | Stop. Do not repeat the write automatically. Preserve logs and diagnose connection/result |
| No AP and no UART after flash | Exact image/package, board configuration and flash result first; do not assume low voltage or erase again |
| AP/page works but no +TIME | Wi-Fi association, DHCP, DNS and NTP response. Test a literal NTP IP to distinguish DNS from time service |
| Local hostname works, external fails | Check router DNS and redirection; capture UDP without filtering out useful DNS packets |
| Wi-Fi powers off with page open | Confirm WIFIAPPING on TXW PA14 at 9600 and browser page remains active |
| Parking selection ignored | Confirm compatible HC32 V15 and the actual final selector digit on UART |
| UI says saved but clock has no time | Saving settings is not an NTP-success assertion; check connection/time status |

## Report a problem

[Open a GitHub issue](https://github.com/maddenste/Chouchin-899-latest-revision/issues)
and include:

- Both chip markings and the firmware filenames used.
- Programmer model, software version and pin connections.
- What you expected, what happened and the exact error message.
- The relevant attempt's server/GDB logs, if flashing failed or was ambiguous.

The documented scripts keep per-attempt logs. Redact credentials, private
addresses as appropriate and factory data before posting.

## Optional UART diagnosis

Experienced users can inspect clock messages with [PulseView](https://www.sigrok.org/wiki/Downloads);
see its [user manual](https://www.sigrok.org/doc/pulseview/0.4.2/manual.html)
for capture and decoder controls. Use the [confirmed wiring](HARDWARE.md#inter-chip-uart--passive-analyser)
and decode **9600 baud, 8N1**. A logic analyser is not required for uploading.
