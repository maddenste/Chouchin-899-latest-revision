# Copyright (C) 2026 Steve Madden
# SPDX-License-Identifier: GPL-3.0-or-later
# TXW813-320 flash-read catch. Normally backs up the full 2 MiB; with
# -GdbInitAndDump -ProbeSlot2 it reads 4 KiB at 0x100000 through the same
# DebugServer/GDB route as the successful backup. It never erases or programs.
# A saved PulseView .sr file is not a live trigger; start this before power-up.
[CmdletBinding()]
param(
    [string]$OutputRoot = (Join-Path $PSScriptRoot '..\logs'),
    [string]$ServerPath = 'C:\C-Sky\DebugServer\bin\DebugServerConsole.exe',
    [string]$GdbPath = 'C:\C-Sky\CDKRepo\Toolchain\CKV2ElfMinilib\V3.10.29\R\bin\csky-elfabiv2-gdb.exe',
    [Parameter(Mandatory)][string]$AlgorithmPath,
    [Parameter(Mandatory)][string]$InitScriptPath,
    [Parameter(Mandatory)][string]$DebuggerEndpoint,
    [switch]$ManualIcePrompt = $true,
    [string]$IceClock = '1200k',
    [int]$DumpTimeoutSeconds = 1200,
    [switch]$GdbOnConnect,
    [switch]$GdbAutoDump,
    [switch]$GdbProbeMemory,
    [switch]$GdbInitAndDump,
    [switch]$ProbeSlot2,
    [switch]$ProbeSlot2Region,
    [switch]$PreflightOnly
)

$ErrorActionPreference = 'Stop'
$serverExe = $ServerPath
$gdbExe = $GdbPath
$flashAlgorithm = $AlgorithmPath
$flashInitScript = $InitScriptPath
if ($DebuggerEndpoint -notmatch '^[A-Za-z0-9.-]+:1025$') { throw 'Use a debugger host with port 1025.' }
$flashStart = if ($ProbeSlot2 -or $ProbeSlot2Region) { '0x100000' } else { '0x0' }
$readLength = if ($ProbeSlot2Region) { 0x50000 } elseif ($ProbeSlot2) { 0x1000 } else { 0x200000 }

# The debugger may ask to update CKLink firmware. Decline only that exact prompt.
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
            $isFirmwarePrompt = $false
            $noButton = $null
            foreach ($child in $children) {
                if ($child.Current.Name -like '*New firmware of ICE is detected, Update or not?*') {
                    $isFirmwarePrompt = $true
                }
                if ($child.Current.Name -eq 'No' -and
                    $child.Current.ControlType -eq [System.Windows.Automation.ControlType]::Button) {
                    $noButton = $child
                }
            }
            if ($isFirmwarePrompt -and $noButton) {
                $invoke = $noButton.GetCurrentPattern([System.Windows.Automation.InvokePattern]::Pattern)
                $invoke.Invoke()
                return $true
            }
        }
    }
    catch {
        # A window can close during enumeration; retry on the next poll.
    }
    return $false
}

function Test-DumpFileClosed {
    param([string]$Path)
    try {
        $handle = [System.IO.File]::Open(
            $Path, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read,
            [System.IO.FileShare]::None)
        $handle.Dispose()
        return $true
    }
    catch [System.IO.IOException] { return $false }
}

if (-not (Test-Path -LiteralPath $serverExe -PathType Leaf)) { throw "Missing debugger server: $serverExe" }
if ((@($GdbOnConnect, $GdbAutoDump, $GdbProbeMemory, $GdbInitAndDump) | Where-Object { $_ }).Count -gt 1) {
    throw 'Choose only one GDB mode.'
}
if ($ProbeSlot2 -and $ProbeSlot2Region) {
    throw 'Choose either -ProbeSlot2 or -ProbeSlot2Region.'
}
if (($ProbeSlot2 -or $ProbeSlot2Region) -and -not $GdbInitAndDump) {
    throw 'The slot-2 probe must use the same -GdbInitAndDump route as the successful factory backup.'
}
if (($GdbOnConnect -or $GdbAutoDump -or $GdbProbeMemory -or $GdbInitAndDump) -and -not (Test-Path -LiteralPath $gdbExe -PathType Leaf)) { throw "Missing C-SKY GDB: $gdbExe" }
if (-not (Test-Path -LiteralPath $flashAlgorithm -PathType Leaf)) { throw "Missing TXW81x flash algorithm: $flashAlgorithm" }
if ($GdbInitAndDump -and -not (Test-Path -LiteralPath $flashInitScript -PathType Leaf)) { throw "Missing TXW81x initialization script: $flashInitScript" }
if ($GdbInitAndDump) {
    $expectedInitHash = '16DE6E4FC6A9D4124B98D45DC934AAE6C6E2AF79E2681DF537F63C790E0DB88B'
    $actualInitHash = (Get-FileHash -LiteralPath $flashInitScript -Algorithm SHA256).Hash
    if ($actualInitHash -ne $expectedInitHash) {
        throw "Vendor initialization script has changed since review; refusing to run it. SHA256: $actualInitHash"
    }
}
if (-not (Test-Path -LiteralPath (Split-Path -Qualifier $OutputRoot))) { throw "Output drive is unavailable: $OutputRoot" }
if ($DumpTimeoutSeconds -lt 60) { throw 'DumpTimeoutSeconds must be at least 60.' }
if ($PreflightOnly) {
    $testName = if ($ProbeSlot2Region) { 'attempt-001-slot2-region.bin' } elseif ($ProbeSlot2) { 'attempt-001-slot2-probe.bin' } else { 'attempt-001-full-flash.bin' }
    Write-Host "Preflight OK: range $flashStart + 0x$($readLength.ToString('X')); output name $testName."
    Write-Host 'No output folder or debugger process was created.'
    return
}

$busy = Get-Process -Name 'CSKYFlashProgrammer', 'DebugServer', 'T-HeadDebugServer', 'DebugServerConsole' -ErrorAction SilentlyContinue
if ($busy) { throw 'Close FlashProgrammer and all DebugServer windows before running this script.' }
if (Get-NetTCPConnection -LocalPort 1025 -State Listen -ErrorAction SilentlyContinue) {
    throw 'Port 1025 is already in use by another debugger. Close it first.'
}

$runDirectory = Join-Path $OutputRoot (Get-Date -Format 'yyyyMMdd-HHmmss')
New-Item -ItemType Directory -Path $runDirectory -ErrorAction Stop | Out-Null
$summary = Join-Path $runDirectory 'summary.txt'

Write-Host ("TXW813-320 flash read: offset {0}, length 0x{1:X}, ICE clock {2}." -f $flashStart, $readLength, $IceClock)
Write-Host "Results will be kept in: $runDirectory"
if ($GdbOnConnect) {
    Write-Host 'GDB mode: the first successful connection stays open; no flash dump runs automatically.'
    Write-Warning 'In GDB, do not use load, flash program, or flash erase. Type quit to close GDB and the server.'
}
if ($GdbAutoDump) {
    Write-Host 'GDB auto-dump mode: attach and request the read immediately after a successful connection.'
    Write-Warning 'This executes a flash-read algorithm in target RAM; it does not erase or program flash.'
}
if ($GdbProbeMemory) {
    Write-Host 'GDB memory-probe mode: read the stopped PC, SRAM, and both candidate flash mappings immediately after connection.'
    Write-Warning 'This reads target memory only; it does not load the flash algorithm or write chip registers.'
}
if ($GdbInitAndDump) {
    Write-Host 'GDB SDK-init mode: run the vendor initialization script and request the flash read immediately after connection.'
    Write-Warning 'The vendor script writes CPU, watchdog, and peripheral control registers, but does not erase or program flash.'
}
Write-Warning 'The Wi-Fi chip must stay powered for the entire dump, potentially many minutes.'
Write-Warning 'A complete-size file is a candidate backup, not a verified second read.'
Write-Warning 'Retries have no time limit or pause. Press Ctrl+C to stop; attempt logs will accumulate in OutputRoot.'
Read-Host 'Press Enter to start retrying, then trigger the Wi-Fi chip whenever ready' | Out-Null

$attempt = 0
$complete = $false
while (-not $complete) {
    $attempt++
    $prefix = 'attempt-{0:D3}' -f $attempt
    $serverLog = Join-Path $runDirectory "$prefix-server.log"
    $serverErr = Join-Path $runDirectory "$prefix-server.err"
    $commandScript = Join-Path $runDirectory "$prefix-commands.txt"
    $dumpName = if ($ProbeSlot2Region) { "$prefix-slot2-region.bin" } elseif ($ProbeSlot2) { "$prefix-slot2-probe.bin" } else { "$prefix-full-flash.bin" }
    $dumpFile = Join-Path $runDirectory $dumpName
    $attemptStarted = Get-Date

    if (-not ($GdbOnConnect -or $GdbAutoDump -or $GdbProbeMemory -or $GdbInitAndDump)) {
        # Check the CPU state after connection, request debug mode again, then dump.
        # HCR is a debug-control register; this does not erase or program flash.
        # The flash command uses the TXW81x algorithm to read the selected range.
        @(
            'p $hsr'
            'set $hcr=0x8000'
            'p $hsr'
            ("flash dump -o {0} -b -a {1} -s 0x{2:X} -al {3}" -f $dumpFile, $flashStart, $readLength, $flashAlgorithm)
        ) |
            Set-Content -LiteralPath $commandScript -Encoding Ascii
    }

    Write-Host ("Attempt {0} at {1:HH:mm:ss.fff}: trying target debug connection..." -f $attempt, $attemptStarted)
    $serverArgs = @(
        '-setcdi', '2', '-setclk', $IceClock, '-arch', 'csky', '-noddc',
        '-skip-enter', '-port', '1025',
        '--debug', 'connect'
    )
    if (-not ($GdbOnConnect -or $GdbAutoDump -or $GdbProbeMemory -or $GdbInitAndDump)) { $serverArgs += @('-cmd-script', $commandScript) }
    $server = Start-Process -FilePath $serverExe -ArgumentList $serverArgs -WorkingDirectory (Split-Path -Parent $serverExe) -WindowStyle Hidden -PassThru -RedirectStandardOutput $serverLog -RedirectStandardError $serverErr

    $caught = $false
    try {
        $deadline = (Get-Date).AddSeconds($DumpTimeoutSeconds)
        $connectDeadline = (Get-Date).AddSeconds(15)
        while ((Get-Date) -lt $deadline -and -not $server.HasExited) {
            if (Dismiss-IceFirmwareUpdatePrompt -ProcessId $server.Id) {
                "Attempt $attempt declined CKLink firmware update prompt (No)." | Add-Content -LiteralPath $summary
            }
            if (($GdbOnConnect -or $GdbAutoDump -or $GdbProbeMemory -or $GdbInitAndDump) -and (Test-Path -LiteralPath $serverLog -PathType Leaf) -and
                (Select-String -LiteralPath $serverLog -SimpleMatch 'Connect target end(Leave target_open).' -Quiet -ErrorAction SilentlyContinue)) {
                $caught = $true
                if ($GdbAutoDump -or $GdbProbeMemory -or $GdbInitAndDump) {
                    $gdbLog = Join-Path $runDirectory "$prefix-gdb.log"
                    $gdbErr = Join-Path $runDirectory "$prefix-gdb.err"
                    if ($GdbAutoDump -or $GdbInitAndDump) {
                        if ($GdbInitAndDump) {
                            "Attempt $attempt connected; running TXW81x SDK initialization before GDB flash dump." | Add-Content -LiteralPath $summary
                            Write-Host "Attempt $attempt connected. GDB is running the SDK init and requesting the flash read now..."
                        }
                        else {
                            "Attempt $attempt connected; running immediate GDB flash dump." | Add-Content -LiteralPath $summary
                            Write-Host "Attempt $attempt connected. GDB is requesting the flash read now..."
                        }
                        $dumpForward = $dumpFile.Replace('\', '/')
                        $algorithmForward = $flashAlgorithm.Replace('\', '/')
                        $dumpCommand = "monitor flash dump -o $dumpForward -b -a $flashStart -s 0x$($readLength.ToString('X')) -al $algorithmForward"
                        $gdbArgs = @('-batch', '-q', '-ex', '"set remotetimeout 120"', '-ex', ('"target remote ' + $DebuggerEndpoint + '"'))
                        if ($GdbInitAndDump) {
                            $initForward = $flashInitScript.Replace('\', '/')
                            $gdbArgs += @('-ex', ('"source ' + $initForward + '"'))
                            $gdbArgs += @('-ex', '"monitor p $hsr"')
                        }
                        $gdbArgs += @('-ex', ('"' + $dumpCommand + '"'))
                    }
                    else {
                        "Attempt $attempt connected; probing memory mappings through GDB." | Add-Content -LiteralPath $summary
                        Write-Host "Attempt $attempt connected. GDB is reading small memory samples now..."
                        $gdbArgs = @(
                            '-batch', '-q',
                            '-ex', '"set remotetimeout 30"',
                            '-ex', ('"target remote ' + $DebuggerEndpoint + '"'),
                            '-ex', '"x/16wx $pc"',
                            '-ex', '"x/16wx 0x20000000"',
                            '-ex', '"x/16wx 0x08000000"',
                            '-ex', '"x/16wx 0x18000000"'
                        )
                    }
                    $gdb = Start-Process -FilePath $gdbExe -ArgumentList $gdbArgs -WindowStyle Hidden -PassThru -RedirectStandardOutput $gdbLog -RedirectStandardError $gdbErr
                    try {
                        if (-not $gdb.WaitForExit($DumpTimeoutSeconds * 1000)) {
                            Stop-Process -Id $gdb.Id -Force -ErrorAction SilentlyContinue
                            "Attempt $attempt GDB dump timed out after $DumpTimeoutSeconds seconds." | Add-Content -LiteralPath $summary
                        }
                    }
                    finally { $gdb.Dispose() }
                }
                else {
                    "Attempt $attempt connected; opening GDB while DebugServer stays running." | Add-Content -LiteralPath $summary
                    Write-Host "Attempt $attempt connected. Opening C-SKY GDB in its own window now..."
                    $gdb = Start-Process -FilePath $gdbExe -ArgumentList @('-q', '-ex', ('"target remote ' + $DebuggerEndpoint + '"')) -WindowStyle Normal -PassThru
                    try {
                        Write-Host 'Use the new GDB window. Closing it will release this debug connection.'
                        $gdb.WaitForExit()
                    }
                    finally { $gdb.Dispose() }
                }
                break
            }
            if ((Get-Date) -ge $connectDeadline -and
                -not (Test-Path -LiteralPath $dumpFile -PathType Leaf)) {
                # No dump began. Restart the debugger instead of waiting 20 minutes.
                break
            }
            if (Test-Path -LiteralPath $dumpFile -PathType Leaf) {
                $size = (Get-Item -LiteralPath $dumpFile).Length
                if ($size -eq $readLength -and (Test-DumpFileClosed -Path $dumpFile)) {
                    # The expected bytes are present and the writer has closed the file.
                    break
                }
            }
            Start-Sleep -Milliseconds 100
        }
    }
    finally {
        if (-not $server.HasExited) { Stop-Process -Id $server.Id -Force -ErrorAction SilentlyContinue }
        $server.Dispose()
    }

    if ($caught -and $GdbOnConnect) { break }
    if ($caught -and $GdbProbeMemory) {
        "Attempt $attempt memory probe finished; GDB log: $gdbLog; GDB errors: $gdbErr" |
            Add-Content -LiteralPath $summary
        Write-Host "Memory probe finished. See $gdbLog"
        break
    }

    $size = if (Test-Path -LiteralPath $dumpFile -PathType Leaf) {
        (Get-Item -LiteralPath $dumpFile).Length
    } else { 0 }
    $serverText = if (Test-Path -LiteralPath $serverLog -PathType Leaf) {
        Get-Content -LiteralPath $serverLog -Raw
    } else { '' }
    $serverErrorText = if (Test-Path -LiteralPath $serverErr -PathType Leaf) {
        Get-Content -LiteralPath $serverErr -Raw
    } else { '' }
    $serverFailed = ($serverText + "`n" + $serverErrorText) -match '(?im)ERROR:|Dump failed|Failed to load algorithm'
    if ($size -eq $readLength -and -not $serverFailed) {
        $bytes = [System.IO.File]::ReadAllBytes($dumpFile)
        if ($ProbeSlot2 -or $ProbeSlot2Region) {
            $nonBlank = -1
            for ($i = 0; $i -lt $bytes.Length; $i++) {
                if ($bytes[$i] -ne 0xff) { $nonBlank = $i; break }
            }
            if ($nonBlank -ge 0) {
                throw ('Slot 2 is not blank at flash address 0x{0:X}; no programming was attempted.' -f (0x100000 + $nonBlank))
            }
            "SLOT-2 READ SUCCESS: $dumpFile; $readLength bytes of FF at 0x100000" | Add-Content -LiteralPath $summary
            Write-Host "SLOT-2 READ SUCCESS: $readLength bytes of FF at 0x100000; log: $gdbErr"
            $complete = $true
            break
        }
        $nonUniform = $false
        for ($i = 1; $i -lt $bytes.Length; $i++) {
            if ($bytes[$i] -ne $bytes[0]) { $nonUniform = $true; break }
        }
        if ($nonUniform) {
            $hash = (Get-FileHash -LiteralPath $dumpFile -Algorithm SHA256).Hash
            $complete = $true
            @("CANDIDATE FULL BACKUP: $dumpFile", "Size: $size bytes", "SHA256: $hash") |
                Add-Content -LiteralPath $summary
            Write-Host "Candidate 2 MiB backup: $dumpFile"
            Write-Host "SHA-256: $hash"
            break
        }
    }

    "Attempt $attempt ($($attemptStarted.ToString('HH:mm:ss.fff'))): $size of $readLength bytes; serverFailed=$serverFailed; logs: $serverLog" |
        Add-Content -LiteralPath $summary
    if (($GdbAutoDump -or $GdbInitAndDump) -and $caught) {
        Write-Host "The immediate GDB read produced $size bytes. Stopping to inspect this attempt's logs."
        break
    }
    Write-Host "Attempt $attempt did not produce a complete, plausible image ($size bytes). Retrying..."
}

Write-Host "Results: $runDirectory"
