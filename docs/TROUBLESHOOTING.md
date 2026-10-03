# Troubleshooting

## Flashing problems

**If writing has started, keep power steady.** The clock's red LED is controlled
by the HC32, not the Wi-Fi programmer; follow the script's messages, not the LED.
If the write fails or its result is unclear, keep the logs and do not immediately
retry.

| Symptom | First checks |
| --- | --- |
| Launcher says to extract the complete project | Extract the whole ZIP. Keep **Flash-WiFi.bat** beside the **tools** folder; do not run the BAT alone or from inside the ZIP. |
| Programmer/debugger is already running | Close FlashProgrammer and DebugServer windows before starting the launcher. |
| CKLink is not detected / "No ICE" | Check the probe's USB cable and installed driver. Close other programmer/debugger windows. |
| It keeps trying to connect | Before writing starts, check board power, common GND and the [PA9/PA10 wiring](FLASHING.md#2-connect-with-power-off). Cycle board power when the script is trying to connect; about five cycles was typical on our setup. |
| CKLink firmware-update window appears | The launcher normally selects **No** automatically. If the window stays open, select **No** yourself. This is a probe update, not the clock firmware upload. To stop a repeating window loop **before writing starts**, unplug the probe from USB, then close the flashing window. |
| Cannot find the flash algorithm | Select the matching **TXW81X_FLASH_ALGORITHM.elf** and **.init** files from the [documented FPV package](TOOLS.md#software-downloads). |
| HC32 serial COM port is missing | Check the probe has UART pins, its USB cable and driver. Follow the [HC32 power-on order](HC32.md#2-power-on-in-this-order); DAPLink was plugged into USB after the clock was powered. |
| Writing is taking longer than expected | Keep power steady. Wi-Fi writing took about **90–100 seconds** on our setup, but may take longer. Inspect progress logs; elapsed time alone does not prove failure. |
| Writing failed or success is unclear | Do not immediately retry or erase the chip. Keep the attempt logs and [report the problem](#report-a-problem). |

## Clock and webpage problems

| Symptom | First checks |
| --- | --- |
| No setup network after flashing | Confirm the programmer reported success, the correct image was used and the probe wires were removed before restarting. A clean Wi-Fi installation should provide **WiFi-Clock-Setup**. |
| Page says settings are saved, but the clock has no time | Check the page's Wi-Fi and time status. Saving settings does not mean NTP has synchronised. |
| Page works, but time does not synchronise | Check Wi-Fi, DNS and NTP access. An IPv4 NTP address can help distinguish a DNS problem from a time-server problem. |
| Local NTP hostname works, but an external hostname fails | Check router DNS, redirection and Internet NTP access. For packet-capture checks, see the advanced section below. |
| Wi-Fi powers off while the page is open | Keep the page active. If it still happens, use the optional UART checks below to confirm the keep-alive messages. |
| Seconds hand stays at 12 | Check **Second hand battery saver** and **Hold&Start** first. If changing the setting has no effect, confirm HC32 V15 is installed; advanced UART checks are below. |

## Report a problem

[Open a GitHub issue](https://github.com/maddenste/Chouchin-899-latest-revision/issues)
and include:

- Both chip markings and the firmware filenames used.
- Programmer model, software version and pin connections.
- What you expected, what happened and the exact error message.
- The relevant attempt's server/GDB logs, if flashing failed or was ambiguous.

The documented scripts keep per-attempt logs. Redact credentials, private
addresses as appropriate and factory data before posting.

## Advanced diagnostics

These checks are optional and are not part of the normal flashing procedure.

| Symptom | Further checks |
| --- | --- |
| Probe identified, but HID is all FF / target check fails | Check board power, common ground, PA9/PA10 wiring and catch timing. Probe recognition alone is not target access. |
| Dump failed / Rcmd error | Read the attempt logs and confirm initialisation and connection stability; do not jump straight to erase. |
| All-FF read output | Check the region and initialisation. An erased slot is not a recovered whole-firmware backup. |
| GDB console will not accept typing | Focus its console and leave text-selection mode with **Esc**. Do not reset the board merely to restore typing. |
| No AP and no UART after flashing | Check the exact image/package, board configuration and flash result first; do not assume low voltage or erase again. |
| DNS or NTP behaviour is unclear | Capture router UDP traffic without filtering out DNS replies. Check Wi-Fi association, DHCP, DNS and the NTP response. |
| Wi-Fi powers off with the page open | Confirm **WIFIAPPING** on TXW PA14 at **9600 baud**, with the browser page active. |
| Parking selection is ignored | Confirm compatible HC32 V15 and the actual final selector digit in **+TIME** on UART. |

## Optional UART diagnosis

Experienced users can inspect clock messages with [PulseView](https://www.sigrok.org/wiki/Downloads);
we used a [DAOKAI 24 MHz, 8-channel USB logic analyser](https://www.amazon.co.uk/dp/B0CP952357).
See its [user manual](https://www.sigrok.org/doc/pulseview/0.4.2/manual.html)
for capture and decoder controls. Use the [confirmed wiring](HARDWARE.md#inter-chip-uart--passive-analyser)
and decode **9600 baud, 8N1**. A logic analyser is not required for uploading.
