# Copyright (C) 2026 Steve Madden
# SPDX-License-Identifier: GPL-3.0-or-later
# One-shot full-chip clock program. The DebugServer flash command erases
# the chip. CleanFullImage uses the public R17 image with all settings erased.
# The legacy per-board route preserves configuration from a private backup.
# There is deliberately no confirmation prompt, -v verification, or readback.
# Without -Program this script only checks the local input files.
[CmdletBinding()]
param(
    [switch]$Program,
    [switch]$CleanFullImage,
    [switch]$ManualIcePrompt,
    [string]$CodePath,
    [Parameter(Mandatory)][string]$ImagePath,
    [string]$ExpectedCodeHash,
    [string]$ExpectedImageHash,
    [string]$FactoryBackup,
    [string]$ExpectedFactoryHash,
    [Parameter(Mandatory)][string]$AlgorithmPath,
    [Parameter(Mandatory)][string]$InitScriptPath,
    [string]$ServerPath = 'C:\C-Sky\DebugServer\bin\DebugServerConsole.exe',
    [string]$GdbPath = 'C:\C-Sky\CDKRepo\Toolchain\CKV2ElfMinilib\V3.10.29\R\bin\csky-elfabiv2-gdb.exe',
    [Parameter(Mandatory)][string]$DebuggerEndpoint,
    [string]$OutputRoot = (Join-Path $PSScriptRoot '..\logs'),
    [string]$IceClock = '1200k',
    [int]$CatchTimeoutSeconds = 300,
    [int]$OperationTimeoutSeconds = 900
)

$ErrorActionPreference = 'Stop'
$serverExe = $ServerPath
$gdbExe = $GdbPath
$algorithm = $AlgorithmPath
$initScript = $InitScriptPath
$factory = $FactoryBackup
if ($DebuggerEndpoint -notmatch '^[A-Za-z0-9.-]+:1025$') { throw 'Use a debugger host with port 1025.' }
$code = $CodePath
$image = $ImagePath
if ($CleanFullImage) {
    if ($CodePath -or $FactoryBackup -or $ExpectedFactoryHash -or $ExpectedCodeHash) {
        throw 'CleanFullImage does not use APP or original-backup parameters.'
    }
    $releaseHash = '1F4908F9ACC9160CDF53CA3F5C5BEF96CBF4C9197700C87640C01F13592CE9A9'
    if ($ExpectedImageHash -and $ExpectedImageHash -ne $releaseHash) { throw 'Unexpected clean R17 image hash.' }
    $ExpectedImageHash = $releaseHash
} elseif (-not $CodePath -or -not $ExpectedCodeHash -or -not $FactoryBackup -or -not $ExpectedFactoryHash -or -not $ExpectedImageHash) {
    throw 'Legacy image mode requires APP, backup and recorded hashes. Use CleanFullImage for the public FULL.'
}
$expectedHashes = @{
    $algorithm = '7BF137DB393ECF74F361554691044D8D65266753340718DA63373510EEEEDC8A'
    $initScript = '16DE6E4FC6A9D4124B98D45DC934AAE6C6E2AF79E2681DF537F63C790E0DB88B'
    $image = $ExpectedImageHash
}
if (-not $CleanFullImage) {
    $expectedHashes[$factory] = $ExpectedFactoryHash
    $expectedHashes[$code] = $ExpectedCodeHash
}
$flashLength = 0x200000
$settingsStart = 0x1FC000
$configStart = 0x1FE000

foreach ($path in @($serverExe, $gdbExe) + @($expectedHashes.Keys)) {
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing file: $path" }
}
foreach ($path in $expectedHashes.Keys) {
    $hash = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
    if ($hash -ne $expectedHashes[$path]) { throw "Hash mismatch: $path; $hash" }
}
if ($IceClock -notmatch '^(100|[1-9][0-9]{2,3})k$') { throw 'Invalid ICE clock.' }
if ($CatchTimeoutSeconds -lt 5 -or $OperationTimeoutSeconds -lt 600) { throw 'Timeout too short.' }
if (-not (Test-Path -LiteralPath (Split-Path -Qualifier $OutputRoot))) { throw "Output drive missing: $OutputRoot" }
$imageBytes = [IO.File]::ReadAllBytes($image)
if ($CleanFullImage) {
    $factoryBytes = [byte[]]::new($flashLength)
    $codeBytes = [byte[]]::new(327184)
    if ($imageBytes.Length -ne $flashLength) { throw 'Clean FULL must be exactly 2 MiB.' }
    [Array]::Copy($imageBytes, $codeBytes, $codeBytes.Length)
} else {
    $factoryBytes = [IO.File]::ReadAllBytes($factory)
    $codeBytes = [IO.File]::ReadAllBytes($code)
}
if ($factoryBytes.Length -ne $flashLength -or $imageBytes.Length -ne $flashLength -or
    $codeBytes.Length -lt 0x1000 -or $codeBytes.Length -gt $settingsStart -or $imageBytes[0] -ne 0x69 -or
    $imageBytes[1] -ne 0x5a -or $imageBytes[2] -ne 0) {
    throw 'Image size or first-slot boot header is unexpected.'
}
for ($i = 0; $i -lt $flashLength; $i++) {
    $expected = if ($i -lt $codeBytes.Length) { $codeBytes[$i] }
        elseif ($i -ge $configStart -and -not $CleanFullImage) { $factoryBytes[$i] }
        else { 0xff }
    if ($imageBytes[$i] -ne $expected) { throw ('Image layout mismatch at 0x{0:X}' -f $i) }
}
Write-Host "Preflight OK: 2 MiB DCDC-off image $($expectedHashes[$image])."
if ($CleanFullImage) { Write-Host 'Clean installation: old settings are erased. Enter Wi-Fi settings after flashing.' }
else { Write-Host 'Private per-board image: original configuration sectors preserved.' }
Write-Host 'No flash verification or flash readback is configured.'
if (-not $Program) { Write-Host 'Offline preflight only; no hardware accessed.'; return }

$busy = Get-Process -Name 'CSKYFlashProgrammer', 'CSKYFlashProgramerConsole',
    'DebugServer', 'T-HeadDebugServer', 'DebugServerConsole' -ErrorAction SilentlyContinue
if ($busy) { throw 'Close FlashProgrammer and all DebugServer windows first.' }
if (Get-NetTCPConnection -LocalPort 1025 -State Listen -ErrorAction SilentlyContinue) {
    throw 'Debugger port 1025 is already in use.'
}

if (-not $ManualIcePrompt) {
    Add-Type -AssemblyName UIAutomationClient
    Add-Type -AssemblyName UIAutomationTypes
}
function Dismiss-IceFirmwareUpdatePrompt {
    param([int]$ProcessId)
    if ($ManualIcePrompt) { return $false }
    try {
        $windows = [System.Windows.Automation.AutomationElement]::RootElement.FindAll(
            [System.Windows.Automation.TreeScope]::Children,
            [System.Windows.Automation.Condition]::TrueCondition)
        foreach ($window in $windows) {
            if ($window.Current.ProcessId -ne $ProcessId) { continue }
            $children = $window.FindAll(
                [System.Windows.Automation.TreeScope]::Descendants,
                [System.Windows.Automation.Condition]::TrueCondition)
            $prompt = $false
            $noButton = $null
            foreach ($child in $children) {
                if ($child.Current.Name -like '*New firmware of ICE is detected, Update or not?*') { $prompt = $true }
                if ($child.Current.Name -eq 'No' -and
                    $child.Current.ControlType -eq [System.Windows.Automation.ControlType]::Button) { $noButton = $child }
            }
            if ($prompt -and $noButton) {
                $noButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern).Invoke()
                return $true
            }
        }
    } catch { }
    return $false
}

$runDirectory = Join-Path $OutputRoot ('dcdc0-write-only-' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $runDirectory -ErrorAction Stop | Out-Null
$summary = Join-Path $runDirectory 'summary.txt'
@("Prepared image: $image", "Image SHA-256: $($expectedHashes[$image])",
  "Factory backup: $factory", 'No verification or readback requested.') | Set-Content -LiteralPath $summary
Write-Host "Writer armed; power-cycle the TXW once. Logs: $runDirectory"

$deadline = (Get-Date).AddSeconds($CatchTimeoutSeconds)
$attempt = 0
$caught = $false
while ((Get-Date) -lt $deadline -and -not $caught) {
    $attempt++
    $prefix = 'attempt-{0:D3}' -f $attempt
    $serverLog = Join-Path $runDirectory "$prefix-server.log"
    $serverErr = Join-Path $runDirectory "$prefix-server.err"
    $serverArgs = @('-setcdi', '2', '-setclk', $IceClock, '-arch', 'csky', '-noddc',
        '-skip-enter', '-port', '1025', '-flash-timeout', '300', '--debug', 'connect')
    $server = Start-Process -FilePath $serverExe -ArgumentList $serverArgs `
        -WorkingDirectory (Split-Path -Parent $serverExe) -WindowStyle Hidden `
        -PassThru -RedirectStandardOutput $serverLog -RedirectStandardError $serverErr
    try {
        $attemptDeadline = (Get-Date).AddSeconds(5)
        while ((Get-Date) -lt $attemptDeadline -and -not $server.HasExited) {
            if (Dismiss-IceFirmwareUpdatePrompt -ProcessId $server.Id) {
                "Attempt $attempt declined CKLink firmware update prompt (No)." | Add-Content -LiteralPath $summary
            }
            if ((Test-Path -LiteralPath $serverLog -PathType Leaf) -and
                (Select-String -LiteralPath $serverLog -SimpleMatch 'Connect target end(Leave target_open).' -Quiet -ErrorAction SilentlyContinue)) {
                $caught = $true
                break
            }
            Start-Sleep -Milliseconds 25
        }
        if ($caught) {
            Write-Host "Attempt $attempt connected. Sending the single full-image program command..."
            "Program command issued: $(Get-Date -Format o)" | Add-Content -LiteralPath $summary
            $gdbLog = Join-Path $runDirectory "$prefix-gdb.log"
            $gdbErr = Join-Path $runDirectory "$prefix-gdb.err"
            $imageForward = $image.Replace('\', '/')
            $algorithmForward = $algorithm.Replace('\', '/')
            $initForward = $initScript.Replace('\', '/')
            $programCommand = "monitor flash program -f $imageForward -b -a 0x0 -al $algorithmForward"
            $gdbArgs = @('-batch', '-q', '-ex', '"set remotetimeout 300"',
                '-ex', ('"target remote ' + $DebuggerEndpoint + '"'),
                '-ex', ('"source ' + $initForward + '"'),
                '-ex', '"monitor p $hsr"',
                '-ex', ('"' + $programCommand + '"'))
            $gdb = Start-Process -FilePath $gdbExe -ArgumentList $gdbArgs `
                -WindowStyle Hidden -PassThru -RedirectStandardOutput $gdbLog `
                -RedirectStandardError $gdbErr
            try {
                $elapsed = 0
                while (-not $gdb.WaitForExit(30000)) {
                    $elapsed += 30
                    Write-Host "Programming still running ($elapsed seconds); keep power steady..."
                    if ($elapsed -ge $OperationTimeoutSeconds) {
                        Stop-Process -Id $gdb.Id -Force -ErrorAction SilentlyContinue
                        throw "Programming timed out after write may have begun. DO NOT retry. Inspect $runDirectory"
                    }
                }
            } finally { $gdb.Dispose() }
        }
    } finally {
        if (-not $server.HasExited) { Stop-Process -Id $server.Id -Force -ErrorAction SilentlyContinue }
        $server.Dispose()
    }
    if (-not $caught -and $attempt % 20 -eq 0) { Write-Host "Attempt ${attempt}: waiting for power-on debug window..." }
}
if (-not $caught) { throw "No connection; no write was issued. Logs: $runDirectory" }
$gdbText = if (Test-Path -LiteralPath $gdbErr -PathType Leaf) { Get-Content -LiteralPath $gdbErr -Raw } else { '' }
if ($gdbText -notmatch '(?m)^Program success\.\r?$') {
    throw "Programmer did not report success. DO NOT retry. Inspect $runDirectory"
}
"Programmer reported success: $(Get-Date -Format o)" | Add-Content -LiteralPath $summary
Write-Host "WRITTEN: programmer reported Program success. No flash readback was performed. Logs: $runDirectory"
