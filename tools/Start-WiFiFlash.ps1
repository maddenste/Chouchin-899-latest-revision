# Copyright (C) 2026 Steve Madden
# SPDX-License-Identifier: GPL-3.0-or-later
[CmdletBinding()]
param([switch]$DefineOnly)
$ErrorActionPreference = 'Stop'

function Select-ClockFile {
    param([string]$Title, [string]$Filter)
    $dialog = [Windows.Forms.OpenFileDialog]::new()
    try {
        $dialog.Title = $Title
        $dialog.Filter = $Filter
        $dialog.CheckFileExists = $true
        if ($dialog.ShowDialog() -ne [Windows.Forms.DialogResult]::OK) { throw 'Cancelled. No write started.' }
        return $dialog.FileName
    } finally { $dialog.Dispose() }
}

function Resolve-ClockFile {
    param([string]$Title, [string]$Filter, [string[]]$Candidates, [string]$RequiredName)
    foreach ($candidate in $Candidates) {
        if ($candidate -and (Test-Path -LiteralPath $candidate -PathType Leaf)) {
            if ([IO.Path]::GetFileName($candidate) -ieq $RequiredName) { return (Resolve-Path -LiteralPath $candidate).Path }
        }
    }
    Write-Host "Please locate $Title in the file-selection window."
    $selected = Select-ClockFile $Title $Filter
    if ([IO.Path]::GetFileName($selected) -ine $RequiredName) { throw "Select $RequiredName, not a different tool or firmware file." }
    return $selected
}

function Get-ClockNetworkChoices {
    param([object[]]$Configurations)
    @($Configurations | Where-Object {
        $_.NetAdapter.Status -eq 'Up' -and $_.IPv4DefaultGateway -and $_.IPv4Address
    } | ForEach-Object {
        foreach ($address in $_.IPv4Address) {
            if ($address.IPAddress -notlike '169.254.*') {
                [pscustomobject]@{ Label = $_.InterfaceAlias; Address = $address.IPAddress }
            }
        }
    })
}

function Select-ClockEndpoint {
    $choices = @(Get-ClockNetworkChoices @(Get-NetIPConfiguration))
    if ($choices.Count -eq 1) {
        Write-Host "PC network: $($choices[0].Label) ($($choices[0].Address))."
        return "$($choices[0].Address):1025"
    }
    if ($choices.Count -gt 1) {
        Write-Host 'More than one PC network is active. Select your normal Ethernet or Wi-Fi connection (not a VPN).'
        for ($i = 0; $i -lt $choices.Count; $i++) { Write-Host "$($i + 1). $($choices[$i].Label) - $($choices[$i].Address)" }
        $number = 0
        $answer = Read-Host 'Network number'
        if (-not [int]::TryParse($answer, [ref]$number) -or $number -lt 1 -or $number -gt $choices.Count) { throw 'Invalid network selection. No write started.' }
        return "$($choices[$number - 1].Address):1025"
    }
    throw 'No active IPv4 network with a gateway found. Connect your PC to Ethernet or Wi-Fi, then restart the launcher.'
}

function New-ClockFlashStage {
    param([string]$Workspace, [string]$Image, [string]$Algorithm, [string]$Init)
    if ($Workspace -notmatch '^[A-Za-z]:\\' -or $Workspace -match '[\s";&]') { throw 'The working folder must be an absolute local-drive path without spaces, quotes, semicolons or ampersands.' }
    $stage = Join-Path $Workspace ('run-' + [Guid]::NewGuid().ToString('N'))
    [void](New-Item -ItemType Directory -Path $stage -Force)
    $result = @{}
    foreach ($item in @(
        @{ Key = 'ImagePath'; Source = $Image; Name = 'firmware_FULL.bin' },
        @{ Key = 'AlgorithmPath'; Source = $Algorithm; Name = 'TXW81X_FLASH_ALGORITHM.elf' },
        @{ Key = 'InitScriptPath'; Source = $Init; Name = 'TXW81X_FLASH_ALGORITHM.init' }
    )) {
        $destination = Join-Path $stage $item.Name
        Copy-Item -LiteralPath $item.Source -Destination $destination
        $result[$item.Key] = $destination
    }
    $result['OutputRoot'] = Join-Path $stage 'logs'
    return $result
}

function Invoke-ClockWriter {
    param([hashtable]$Arguments, [switch]$Program)
    & (Join-Path $PSScriptRoot 'Program-TXW813-DCDC0-WriteOnly-OnPower.ps1') @Arguments -Program:$Program
}

function Invoke-ClockFlashWizard {
    param([string]$PreferencesPath = (Join-Path $env:LOCALAPPDATA 'WiFiClockFlasher\paths.json'))
    if (-not $IsWindows -or $PSVersionTable.PSVersion.Major -lt 7) { throw 'Use Windows and PowerShell 7 through Flash-WiFi.bat.' }
    Add-Type -AssemblyName System.Windows.Forms
    Write-Host "`nWiFi Clock v2.0 - guided Wi-Fi R17 installation" -ForegroundColor Cyan
    Write-Host 'TXW813-320 only. This does not flash the HC32 movement controller.'
    Write-Host 'Old Wi-Fi settings will be erased. No flash readback is performed.'
    Write-Host 'Keep board power OFF while preparing. Close FlashProgrammer and all DebugServer windows.'

    $projectRoot = Split-Path -Parent $PSScriptRoot
    $preferencesPath = $PreferencesPath
    $saved = @{}
    if (Test-Path -LiteralPath $preferencesPath) {
        try { $saved = Get-Content -LiteralPath $preferencesPath -Raw | ConvertFrom-Json -AsHashtable } catch { Write-Host 'Saved paths could not be loaded; please select files again.' }
        if ($null -eq $saved -or $saved -isnot [Collections.IDictionary]) { $saved = @{} }
    }
    $imageName = 'WiFi-Clock-v2.0-R17-20261003_FULL.bin'
    $image = Resolve-ClockFile 'the Wi-Fi R17 FULL firmware' 'Firmware BIN (*.bin)|*.bin' @($saved.Image, (Join-Path $projectRoot $imageName), (Join-Path $env:USERPROFILE "Downloads\$imageName")) $imageName
    $algorithm = Resolve-ClockFile 'TXW81X_FLASH_ALGORITHM.elf from the Taixin package' 'Flash algorithm (*.elf)|*.elf' @($saved.Algorithm, 'C:\ClockTools\TXW81X_FLASH_ALGORITHM.elf') 'TXW81X_FLASH_ALGORITHM.elf'
    $init = Resolve-ClockFile 'the matching TXW81X_FLASH_ALGORITHM.init' 'Initialisation file (*.init)|*.init' @($saved.Init, (Join-Path (Split-Path -Parent $algorithm) 'TXW81X_FLASH_ALGORITHM.init')) 'TXW81X_FLASH_ALGORITHM.init'
    $server = Resolve-ClockFile 'DebugServerConsole.exe' 'DebugServerConsole.exe|DebugServerConsole.exe' @($saved.Server, 'C:\C-Sky\DebugServer\bin\DebugServerConsole.exe') 'DebugServerConsole.exe'
    $gdbCandidates = @($saved.Gdb, 'C:\C-Sky\CDKRepo\Toolchain\CKV2ElfMinilib\V3.10.29\R\bin\csky-elfabiv2-gdb.exe')
    $gdb = Resolve-ClockFile 'C-SKY csky-elfabiv2-gdb.exe (not RISC-V GDB)' 'C-SKY GDB|csky-elfabiv2-gdb.exe' $gdbCandidates 'csky-elfabiv2-gdb.exe'
    $endpoint = Select-ClockEndpoint
    $workspace = if ($saved.Workspace) { [string]$saved.Workspace } else { Join-Path $env:SystemDrive 'ClockFlash' }
    try { [void](New-Item -ItemType Directory -Path $workspace -Force) } catch {
        Write-Host 'Cannot create the working folder. Choose a writable folder with no spaces in its path.'
        $folder = [Windows.Forms.FolderBrowserDialog]::new()
        try {
            if ($folder.ShowDialog() -ne [Windows.Forms.DialogResult]::OK) { throw 'Cancelled. No write started.' }
            $workspace = $folder.SelectedPath
        } finally { $folder.Dispose() }
    }
    $flashArgs = New-ClockFlashStage $workspace $image $algorithm $init
    $flashArgs.CleanFullImage = $true
    $flashArgs.ServerPath = $server
    $flashArgs.GdbPath = $gdb
    $flashArgs.DebuggerEndpoint = $endpoint
    Write-Host "`nChecking firmware and algorithm checksums..."
    Invoke-ClockWriter $flashArgs
    # Save only file locations. No passwords, firmware contents or network address.
    try {
        [void](New-Item -ItemType Directory -Path (Split-Path -Parent $preferencesPath) -Force)
        @{ Image = $image; Algorithm = $algorithm; Init = $init; Server = $server; Gdb = $gdb; Workspace = $workspace } |
            ConvertTo-Json | Set-Content -LiteralPath $preferencesPath -Encoding utf8
    } catch { Write-Host 'Could not remember these paths; you can still continue.' }

    Write-Host "`nWIRING - with batteries removed and board power OFF" -ForegroundColor Cyan
    Write-Host 'CKLink TMS/IO -> board PA9; TCK/CK -> PA10; GND -> GND.'
    Write-Host 'External regulated 3.3 V -> battery +; supply negative -> battery -.'
    Write-Host 'Leave CKLink 3V3, 5V, TDI, TDO and nRST disconnected. No BOOT strap.'
    Write-Host 'Plug CKLink into USB. Check chip markings and supply polarity.'
    Write-Host "`nPress Enter when wired and ready, or type Q to quit without writing."
    if ((Read-Host 'Ready') -ne '') { Write-Host 'Cancelled. No write started.'; return }
    Write-Host "`nWhen connection attempts begin, switch board power ON."
    Write-Host 'If it does not catch, cycle board power. About five cycles was typical on our setup.'
    Write-Host 'Choose NO if a probe firmware-update prompt appears.'
    Write-Host 'ONCE CONNECTED: STOP CYCLING POWER. KEEP POWER STEADY UNTIL WRITTEN.' -ForegroundColor Yellow
    Write-Host "Working files and logs: $($flashArgs.OutputRoot)"
    Invoke-ClockWriter $flashArgs -Program
    Write-Host "`nFinished: programmer reported the firmware written." -ForegroundColor Green
    Write-Host 'Power off, unplug CKLink and remove its board connections. Restart normally.'
    Write-Host 'Join WiFi-Clock-Setup, open http://192.168.4.1/ and save your Wi-Fi and clock settings.'
}

if (-not $DefineOnly) {
    try { Invoke-ClockFlashWizard } catch {
        Write-Host "`nSTOPPED: $($_.Exception.Message)" -ForegroundColor Red
        Write-Host 'If writing may have started, do not immediately retry. Keep power steady and keep the logs.'
        exit 1
    }
}
