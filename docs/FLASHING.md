# TXW backup and power-on programming

Read [HARDWARE.md](HARDWARE.md) first. These commands are for TXW813, not HC32.
Use PowerShell 7. External DebugServer/GDB and matching algorithm files are
required; the scripts do not install drivers or convert probe firmware.

## Configure paths

From the repository root, substitute your own paths and debugger host:

```powershell
$algorithm = 'D:\SDK\TXW81X_FLASH_ALGORITHM.elf'
$init = 'D:\SDK\TXW81X_FLASH_ALGORITHM.init'
$endpoint = '192.168.0.5:1025'
$logs = 'D:\PrivateBackups\TXW-Logs'
```

The endpoint is the **PC running DebugServer**, not the Wi-Fi clock's IP.
192.168.0.5 was the bench PC address; it is not a universal default.
Check where DebugServer listens before substituting loopback or another IP.
Vendor-command path handling with spaces has not been validated: use simple
paths without spaces for firmware, algorithm and init inputs.

The documented writer pins the exact matching algorithm hashes:

- ELF: `7BF137DB393ECF74F361554691044D8D65266753340718DA63373510EEEEDC8A`
- init: `16DE6E4FC6A9D4124B98D45DC934AAE6C6E2AF79E2681DF537F63C790E0DB88B`

Do not remove the check to make a different algorithm “work”.

## Obtain an original full backup

Close all FlashProgrammer and DebugServer windows. Start before applying board
power, with the CKLink connected. Preflight first:

```powershell
.\tools\Read-TXW813-OnPower.ps1 -AlgorithmPath $algorithm -InitScriptPath $init -DebuggerEndpoint $endpoint -PreflightOnly

.\tools\Read-TXW813-OnPower.ps1 -AlgorithmPath $algorithm -InitScriptPath $init -DebuggerEndpoint $endpoint -OutputRoot $logs -GdbInitAndDump
```

Follow the start prompt, then power the TXW normally. If it misses the window,
cycle board power between connection attempts. Choose **No** for the optional
ICE firmware upgrade prompt. The backup catch retries without an overall
30-second limit; an individual dump has its own timeout.

The read route initialises hardware registers using the SDK init script and
reads 0x200000 bytes from flash offset zero. It does not erase/program flash,
but is not a purely passive electrical observation. It does not consume a
live analyser trigger or detect the recorded power-on spike.

Repeat independently and compare file sizes and SHA-256 hashes:

```powershell
Get-Item -LiteralPath 'D:\PrivateBackups\read1.bin','D:\PrivateBackups\read2.bin' | Select-Object Name,Length
Get-FileHash -LiteralPath 'D:\PrivateBackups\read1.bin','D:\PrivateBackups\read2.bin' -Algorithm SHA256
```

Both must be exactly 2,097,152 bytes, plausible and identical. Preserve both
reads and their logs. Small slot-2 probes or all-FF data are not full backups.

## Construct a per-board full image

Use the SDK **packaged APP**, not a raw linker binary. Supply previously
recorded and checked hashes; the following names are placeholders:

```powershell
$backup = 'D:\PrivateBackups\my-original.bin'
$backupHash = 'YOUR_VERIFIED_ORIGINAL_SHA256'
$app = 'D:\Builds\clock_APP.bin'
$appHash = 'YOUR_RECORDED_APP_SHA256'
$full = 'D:\Builds\clock_FULL.bin'

.\tools\Build-TXW813-DCDC0FullImage.ps1 -CodePath $app -OutputPath $full -ExpectedCodeHash $appHash -FactoryBackup $backup -ExpectedFactoryHash $backupHash
$fullHash = (Get-FileHash -LiteralPath $full -Algorithm SHA256).Hash
```

Layout: APP at zero, erased padding/settings, and **your original** final 8 KiB
at 0x1FE000–0x1FFFFF. The builder refuses an existing output file.
Never distribute or flash another board's factory-containing FULL image.

## Preflight, then one write

```powershell
$write = @{
    CodePath = $app
    ImagePath = $full
    ExpectedCodeHash = $appHash
    ExpectedImageHash = $fullHash
    FactoryBackup = $backup
    ExpectedFactoryHash = $backupHash
    AlgorithmPath = $algorithm
    InitScriptPath = $init
    DebuggerEndpoint = $endpoint
    OutputRoot = $logs
}
.\tools\Program-TXW813-DCDC0-WriteOnly-OnPower.ps1 @write
```

Without `-Program`, this checks local files/layout only. When ready:

```powershell
.\tools\Program-TXW813-DCDC0-WriteOnly-OnPower.ps1 @write -Program
```

This is destructive: the vendor full-image program command erases/programs
the TXW. There is **no confirmation prompt and no readback/verification command**.
HC32 is untouched. It catches a connection, then sends **one** bounded program
command; no automatic retry occurs after that command may have begun.

Keep power steady throughout the write. The bench R14 write took roughly
90–100 seconds, but this is not a guaranteed duration. The default catch timeout
is 300 seconds and operation timeout 900 seconds. Progress messages alone
are not success. Success requires the logged `Program success.` response.

## If anything is uncertain

Do not erase or retry blindly. Keep the attempt logs and diagnose them.
A process error/timeout can happen after a partial write. See
[TROUBLESHOOTING.md](TROUBLESHOOTING.md). A separately requested recovery operation
can build a restore image from your own backup; it is not an automatic fallback.
