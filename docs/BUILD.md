# Development and build notes (Windows)

**Scope:** this records the working local build, not a complete buildable public
SDK project. Vendor-derived startup/integration overlays and project metadata
are not included. Prepare-Sdk alone does not restore those modified files.
A reproducible public cross-build is not yet offered. For installation, use
the prepared images and [the upload walkthrough](GETTING_STARTED.md).
See [the project notices](../NOTICE.md) for source and binary licensing scope.

## Dependencies

- TXW81x IOT SDK **2.5.2.6-31320**, separately obtained; root must contain
  `sdk`, `csky`, `libs`, `project`. Use PowerShell 7 for the image builder.
- C-Sky CDK and CKV2ElfMinilib **3.10.29**, `csky-elfabiv2-*` tools.
- Python 3 with `tzdata` available for `zoneinfo` tests on Windows.
- TinyCC native Windows compiler for real-C host tests.
- Node.js, Playwright and Microsoft Edge for browser tests.
- Your own verified 2 MiB original flash backup, kept outside the repository.

Vendor executables/libraries are not supplied. Preserve existing upstream
notices; overlay permissions are still under review separately from compiling.
CDK GUI is optional: the build invokes `cdk-make.exe`.

From the repository root:

```powershell
.\tools\Prepare-Sdk.ps1 -SdkRoot 'D:\SDK\TXW81x_IOT-v2.5.2.6-31320'
python -m pip install tzdata
npm install --prefix 'D:\Tools\node-project' playwright
```

Prepare-Sdk refuses existing dependency destinations. It copies the vendor
trees, vendor project support directories, packaging dependencies and unchanged
vendor parameter files, leaving the tracked application
overlays intact. Do not overwrite the application with the vendor demo.

Build example; substitute your actual tool/module/backup paths:

```powershell
$backup = 'D:\PrivateBackups\my-txw-original.bin'
$backupHash = (Get-FileHash -LiteralPath $backup -Algorithm SHA256).Hash
.\firmware\Build-Firmware.ps1 -Name WiFi_Clock_v2_0_R16_TEST `
  -TccPath 'D:\Tools\tcc\tcc.exe' `
  -NodeModules 'D:\Tools\node-project\node_modules' `
  -FactoryBackup $backup -ExpectedFactoryHash $backupHash
```

The hash should already have been recorded when two independent original
reads matched; computing it above identifies the input but does not prove it
is a good backup. Do not use an erased/all-FF or another board's dump.

Optional parameters select `-Python`, `-Node`, `-CdkMake`, `-Toolchain`.
`-SkipBrowserTests` is for development only; it explicitly reduces verification.
The portable staging build does not include private HC32 machine-code emulation.

## Pipeline

1. Generate `clock_web_ui.h` from tracked `web/index.html`.
2. Compile/run native protocol, backend, application and HTTP tests.
3. Run browser/UI tests unless explicitly skipped.
4. Compile `TXW813_Minimal.cdkws`, active project
   `iot_sdk_work/clock_project/txw81x.cdkproj`, configuration `FLASH`.
5. Require a fresh ELF and clock application symbols.
6. Package using vendor BinScript/makecode. Use the **packaged APP**, not the
   unwrapped linker binary.
7. Construct a full image and independently check every byte of its layout.
8. Keep APP, FULL, ELF and map together under `firmware/release/`.

The full image has application code at zero, FF elsewhere, and your original
final two factory sectors at `0x1FE000` and `0x1FF000`. Size is 2,097,152 bytes.
Settings sectors `0x1FC000/0x1FD000` start erased. Packaging refuses overwrite.
Building does not access the hardware and does not flash.

Configuration fixes which must be retained: DCDC disabled for this board, no
external PSRAM assumptions, UART0 PA13/PA14 at 9600, no debug output on PA14,
cached WPA PSK at boot, enabled lwIP receive/send socket timeouts with timeval
format throughout the locally compiled stack. Do not copy prebuilt lwIP or
restore incompatible vendor defaults without rerunning tests.

## Portable package validation status

On 1 October 2026 the relocated R14 development tree completed C/backend,
HTTP/browser tests, cross-compilation and full-image packaging at its new
E: location. Its full image matched the installed R14 candidate byte-for-byte.
The renamed-image writer's offline preflight passed with no hardware access.
The portable write-only helper was subsequently used for R15/R16; both
reported programming success without flash readback.
A clean public-draft copy was prepared with the separately obtained original
SDK and completed the same full test/build pipeline. Its raw code differs
only in five build-timestamp characters; packaged checksum differences follow.
No firmware from that check was flashed. Another user's machine/board and an
independent-user hardware validation remain outstanding.
The full R16 local build passed again before programming on 1 October 2026.
