# Board identification and wiring

Target: newer Chouchin-899 / CH-899, **HDSC HC32L130J8TA + Taixin TXW813-320**.
Older ESP8285/MM32 electronics use the [original project](https://github.com/maddenste/Chouchin-CH899-Firmware).
A model label is not enough. Photograph both sides and inspect the chip markings.

## Board photographs

Photos supplied by Steve Madden. Published copies retain orientation and full
main-image resolution, with location/camera metadata removed. These are board
identification photos, not photographs of the connected programmers.

### Newer Chouchin-899 — whole-board overview

<img src="photos/chouchin-899-newer-board-overview.jpg" alt="Whole newer Chouchin-899 PCB: HC32 movement controller above, TXW813 Wi-Fi circuitry and antenna below" width="500">

**HC32 movement controller:** upper-left chip. **TXW813 Wi-Fi controller:**
lower section beside the 40 MHz crystal and PCB antenna. Both chips must match
this hardware family; the handwritten mark does not establish a PCB revision.

### HC32L130J8TA — movement controller and programming pads

<img src="photos/hc32l130-controller-and-programming-pads.jpg" alt="HDSC HC32L130J8TA close-up beside CLK, DIO, VDD, GND, RST and BOOT pads" width="600">

The chip marking identifies **HDSC HC32L130J8TA**. The pads beside it are
labelled **CLK, DIO, VDD, GND and RST**, with **BOOT** below.
For the documented UART installation: programmer **U_TX → DIO**,
**U_RX → CLK**, common **GND**, and **BOOT held at 3.3 V**.
For SWD backup/debug, DIO and CLK instead carry SWDIO and SWCLK.
These are two different programming modes; follow [the HC32 guide](HC32.md).

### TXW813-320 — Wi-Fi controller and debug pads

<img src="photos/txw813-wifi-chip-and-debug-pads.jpg" alt="Taixin TXW813-320 beside 40 MHz crystal, PCB antenna and J1 pads labelled GND, PA10, PA9, PA8 and VCC" width="800">

The chip marking identifies **Taixin TXW813-320**, beside the **40 MHz crystal**
and PCB antenna. **J1** is labelled **GND, PA10, PA9, PA8 and VCC**.
The confirmed two-wire debug connections are **TMS/IO → PA9**,
**TCK/CK → PA10** and common **GND**. PA8 is not required for the documented
catch workflow. Refer to the printed pad names, not a guessed connector orientation.

## TXW debug connection — owner-confirmed working wiring

| CKLink Lite V2 | TXW signal | Use |
| --- | --- | --- |
| **TMS/IO** | **PA9** | Two-wire debug data |
| **TCK/CK** | **PA10** | Two-wire debug clock |
| GND | Board GND | Common reference |
| TDI / TDO | Not connected | Five-wire JTAG, not needed here |
| nRST | Not required by the documented catch workflow | Not CHIP_EN |
| U-TX / U-RX | Not connected for debug | Adapter UART, not debug data |

The owner explicitly confirmed PA9/PA10 on 1 October 2026.
PA10 must remain reserved for debug. Verify continuity to your own TXW;
the photographs above show the printed J1 pad labels. Do not infer left/right
order from a programmer's connector layout; match signal names.

The adapter shown during development has 5V, 3V3, TMS/IO, GND, TCK/CK and
TDI, TDO, nRST, U-TX, U-RX labels. No separate VREF input was established.
**Do not assume 3V3 is a voltage-sense input** or connect two power sources.
Check the actual adapter documentation and measured behavior.

The successful workflow does not require a PA8 boot strap or driving CHIP_EN.
CHIP_EN is a board enable/power-control signal, not an established reset/boot
input. Do not short it or drive against the HC32.

## Inter-chip UART / passive analyser

| TXW signal | Direction / connection |
| --- | --- |
| PA13, UART0 RX | From HC32 TX |
| PA14, UART0 TX | To HC32 RX; passive analyser channel here |
| GND | Common analyser/board reference |

Decode **9600 8N1**, idle high, CRLF. Monitor both directions when possible.
Do not connect RS-232 levels, a 5 V UART output or two TX drivers together.
The preliminary P10/115200 observation was not the final release UART mapping.
DM/DP pads are not required for this update.

## HC32 SWD — separate chip and programmer

HC32L130J8TA is an ARM Cortex-M0+ controller: 64 KiB flash, 8 KiB SRAM in the
device project. Keil uses HDSC.HC32L130.1.0.1 and FlashHC32L130_64K.FLM.

| ARM probe | HC32 signal |
| --- | --- |
| TMS/IO (SWDIO in ARM SWD mode) | Board pad DIO — owner-confirmed connection |
| TCK/CK (SWCLK in ARM SWD mode) | Board pad CLK — owner-confirmed connection |
| GND | Common board ground — owner-confirmed connection |
| 3V3 / 5V | Neither connected — owner-confirmed |
| nRST | Not connected — owner-confirmed |
| VTref if specified as an input by that probe | Verified target logic rail |

The owner confirmed that TMS/IO, TCK/CK and GND were the only connections for
**SWD debug mode and the original firmware backup in Keil µVision 5**.
Saved session records identify Keil Command-pane SAVE commands for the backup;
see [HC32 backup instructions](HC32_BACKUP.md).

For HC32 firmware programming through XHSC UART boot mode, the owner confirmed:

| Programmer UART pin | Board pad |
| --- | --- |
| U_TX | DIO |
| U_RX | CLK |
| GND | GND / common ground |

The board's **BOOT pad was held at 3.3 V from the bench supply**, not the
programmer's 3V3/5V pins. The bench supply also powered the clock board itself.
These are pad-label connections
reported by the owner; do not substitute guessed UART pin numbers.
The programming software was **XHSC.exe (HDSC MCU New Programmer)**.
The owner held BOOT high before powering on the board and kept it high
throughout UART programming.
The DAPLink USB was initially unplugged and connected **after the clock board
was powered**. The owner reported that this order was necessary.
After programming, switch power off, disconnect BOOT from the 3.3 V bench
supply, then power on normally, as confirmed by the owner.
Do not treat the SWD table as UART boot-programming wiring.

**TXW PA14 is UART TX; HC32 PA14 is SWCLK.** The GPIO names repeat on both chips.
Verify continuity to the correct IC and do not swap the two debug interfaces.
The owner used a CMSIS-DAP / DAPLink ARM JTAG/SWD probe. No connection photograph
is available; the wiring table will use owner-confirmed pin labels/connections.
Do not infer physical header pin order from the probe name.

## Safety / power

Disconnect batteries/external power before soldering or moving wires. Use short,
strain-relieved wires, stable appropriate board power and common ground.
For uploading, we used a 3.3 V bench supply connected to the battery terminals,
positive to battery positive and negative to battery negative, with batteries removed.
Keep hands/gears unobstructed; avoid back-powering through signal pins.

Start the catch script before waking/power-cycling TXW. Once a program command
begins, keep power steady until completion. Do not power-cycle during writing.
A halted CPU emits no normal UART; exit debugging and power-cycle before deciding
that an image has no AP/time output.
