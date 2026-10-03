# Copyright (C) 2026 Steve Madden
# SPDX-License-Identifier: GPL-3.0-or-later
[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Start-WiFiFlash.ps1') -DefineOnly
function Assert-ClockTest {
    param([bool]$Condition, [string]$Message)
    if (-not $Condition) { throw $Message }
}
# Unique fixtures only. This test never launches DebugServer, GDB or a write.
$testRoot = Join-Path $env:TEMP ('ClockLauncherTest-' + [Guid]::NewGuid().ToString('N'))
[void](New-Item -ItemType Directory -Path $testRoot)
$source = Join-Path $testRoot 'download folder with spaces'
[void](New-Item -ItemType Directory -Path $source)
foreach ($name in 'sample.bin','TXW81X_FLASH_ALGORITHM.elf','TXW81X_FLASH_ALGORITHM.init') {
    Set-Content -LiteralPath (Join-Path $source $name) -Value "fixture $name"
}
$simpleRoot = Join-Path $env:SystemDrive ('ClockLauncherTest-' + [Guid]::NewGuid().ToString('N'))
try {
    $stage = New-ClockFlashStage $simpleRoot (Join-Path $source 'sample.bin') (Join-Path $source 'TXW81X_FLASH_ALGORITHM.elf') (Join-Path $source 'TXW81X_FLASH_ALGORITHM.init')
    Assert-ClockTest ($stage.ImagePath -notmatch '\s') 'Staged paths contain spaces.'
    Assert-ClockTest ((Get-FileHash $stage.ImagePath).Hash -eq (Get-FileHash (Join-Path $source 'sample.bin')).Hash) 'Staging altered image.'
    Assert-ClockTest (Test-Path -LiteralPath $stage.InitScriptPath) 'Missing staged init.'
    $stage2 = New-ClockFlashStage $simpleRoot (Join-Path $source 'sample.bin') (Join-Path $source 'TXW81X_FLASH_ALGORITHM.elf') (Join-Path $source 'TXW81X_FLASH_ALGORITHM.init')
    Assert-ClockTest ($stage.ImagePath -ne $stage2.ImagePath) 'Runs must not overwrite one another.'
    foreach ($bad in 'C:\has spaces','relative','\\server\share','C:\bad;path','C:\bad&path') {
        $rejected = $false
        try { New-ClockFlashStage $bad '' '' '' | Out-Null } catch { $rejected = $true }
        Assert-ClockTest $rejected "Unsafe workspace accepted: $bad"
    }
    function Select-ClockFile { throw 'Picker should not be called for an existing candidate.' }
    $resolved = Resolve-ClockFile 'init' '*.init|*.init' @('C:\missing.init', (Join-Path $source 'TXW81X_FLASH_ALGORITHM.init')) 'TXW81X_FLASH_ALGORITHM.init'
    Assert-ClockTest ($resolved -eq (Join-Path $source 'TXW81X_FLASH_ALGORITHM.init')) 'Candidate resolution failed.'
    function Select-ClockFile { return (Join-Path $source 'sample.bin') }
    $rejected = $false
    try { Resolve-ClockFile 'GDB' '*.exe|*.exe' @() 'csky-elfabiv2-gdb.exe' | Out-Null } catch { $rejected = $true }
    Assert-ClockTest $rejected 'Wrong file name accepted.'
    function Select-ClockFile { throw 'Cancelled. No write started.' }
    $rejected = $false
    try { Resolve-ClockFile 'image' '*.bin|*.bin' @() 'missing.bin' | Out-Null } catch { $rejected = $_.Exception.Message -like 'Cancelled*' }
    Assert-ClockTest $rejected 'File picker cancellation lost.'
    $config = @(
        [pscustomobject]@{ InterfaceAlias='Wi-Fi'; NetAdapter=@{Status='Up'}; IPv4DefaultGateway=@{NextHop='192.168.1.1'}; IPv4Address=@{IPAddress='192.168.1.2'} },
        [pscustomobject]@{ InterfaceAlias='Down'; NetAdapter=@{Status='Down'}; IPv4DefaultGateway=@{NextHop='192.168.2.1'}; IPv4Address=@{IPAddress='192.168.2.2'} },
        [pscustomobject]@{ InterfaceAlias='Link-local'; NetAdapter=@{Status='Up'}; IPv4DefaultGateway=@{NextHop='169.254.1.1'}; IPv4Address=@{IPAddress='169.254.1.2'} },
        [pscustomobject]@{ InterfaceAlias='No gateway'; NetAdapter=@{Status='Up'}; IPv4DefaultGateway=$null; IPv4Address=@{IPAddress='10.0.0.2'} }
    )
    Assert-ClockTest (@(Get-ClockNetworkChoices $config).Count -eq 1) 'Adapter filtering failed.'
    function Get-NetIPConfiguration { $config }
    Assert-ClockTest ((Select-ClockEndpoint) -eq '192.168.1.2:1025') 'Single adapter selection failed.'
    $config += [pscustomobject]@{ InterfaceAlias='Ethernet'; NetAdapter=@{Status='Up'}; IPv4DefaultGateway=@{NextHop='10.0.0.1'}; IPv4Address=@{IPAddress='10.0.0.3'} }
    function Read-Host { return '2' }
    Assert-ClockTest ((Select-ClockEndpoint) -eq '10.0.0.3:1025') 'Multiple adapter selection failed.'
    function Read-Host { return '999' }
    $rejected = $false
    try { Select-ClockEndpoint | Out-Null } catch { $rejected = $true }
    Assert-ClockTest $rejected 'Invalid adapter selection accepted.'
    $config = @()
    $rejected = $false
    try { Select-ClockEndpoint | Out-Null } catch { $rejected = $true }
    Assert-ClockTest $rejected 'Missing network accepted.'
    # Exercise the complete wizard with a fake writer, never the real programmer.
    function Resolve-ClockFile {
        param($Title, $Filter, $Candidates, $RequiredName)
        if ($RequiredName -eq 'TXW81X_FLASH_ALGORITHM.init') { return (Join-Path $source 'TXW81X_FLASH_ALGORITHM.init') }
        if ($RequiredName -eq 'TXW81X_FLASH_ALGORITHM.elf') { return (Join-Path $source 'TXW81X_FLASH_ALGORITHM.elf') }
        return (Join-Path $source 'sample.bin')
    }
    function Select-ClockEndpoint { return '192.168.1.2:1025' }
    $script:writerCalls = [Collections.Generic.List[bool]]::new()
    $script:failPreflight = $false
    $script:failWrite = $false
    function Invoke-ClockWriter {
        param([hashtable]$Arguments, [switch]$Program)
        $script:writerCalls.Add([bool]$Program)
        if (-not $Program -and $script:failPreflight) { throw 'Test: invalid image.' }
        if ($Program -and $script:failWrite) { throw 'Test: programmer did not report success.' }
    }
    $preferences = Join-Path $testRoot 'paths.json'
    @{ Workspace = $simpleRoot } | ConvertTo-Json | Set-Content -LiteralPath $preferences
    function Read-Host { return 'Q' }
    Invoke-ClockFlashWizard -PreferencesPath $preferences | Out-Null
    Assert-ClockTest ($script:writerCalls.Count -eq 1 -and -not $script:writerCalls[0]) 'Cancel must perform offline preflight only.'
    $remembered = Read-ClockPaths $preferences
    Assert-ClockTest ($remembered.Count -eq 6 -and -not $remembered.ContainsKey('DebuggerEndpoint')) 'Preferences must contain only file/workspace paths.'
    $script:writerCalls.Clear()
    $script:failPreflight = $true
    $rejected = $false
    try { Invoke-ClockFlashWizard -PreferencesPath $preferences | Out-Null } catch { $rejected = $true }
    Assert-ClockTest ($rejected -and $script:writerCalls.Count -eq 1 -and -not $script:writerCalls[0]) 'Preflight failure must block programming.'
    $script:failPreflight = $false
    function Read-Host { return '' }
    $script:writerCalls.Clear()
    $script:failWrite = $true
    $rejected = $false
    $messages = & {
        try { Invoke-ClockFlashWizard -PreferencesPath $preferences } catch { $script:writeRejected = $true }
    } 6>&1 | Out-String
    Assert-ClockTest ($script:writeRejected -and $script:writerCalls.Count -eq 2 -and $script:writerCalls[1]) 'Write failure must stop without retry.'
    Assert-ClockTest ($messages -notmatch 'Finished: programmer reported') 'False success shown after write failure.'
    $script:failWrite = $false
    $script:writerCalls.Clear()
    $messages = Invoke-ClockFlashWizard -PreferencesPath $preferences 6>&1 | Out-String
    Assert-ClockTest ($script:writerCalls.Count -eq 2 -and -not $script:writerCalls[0] -and $script:writerCalls[1]) 'Expected one preflight and one write.'
    Assert-ClockTest ($messages -match 'Finished: programmer reported') 'Missing successful finish guidance.'
    # A detached BAT must stop instead of starting a different or missing writer.
    $testBat = Join-Path $testRoot 'Flash-WiFi.bat'
    Copy-Item -LiteralPath (Join-Path (Split-Path -Parent $PSScriptRoot) 'Flash-WiFi.bat') -Destination $testBat
    $batchOutput = & cmd.exe /d /c ('""{0}" <nul"' -f $testBat) 2>&1 | Out-String
    Assert-ClockTest ($LASTEXITCODE -eq 1 -and $batchOutput -match 'Extract the complete project ZIP first') 'Detached BAT did not stop safely.'
    Write-Host 'PASS: file/staging/network checks and mocked complete wizard: cancellation, failed preflight, failed write, one-write success and remembered paths. No hardware accessed.'
} finally {
    # Only delete the exact UUID fixture directories this test created.
    foreach ($fixture in @($testRoot, $simpleRoot)) {
        if ((Split-Path -Leaf $fixture) -match '^ClockLauncherTest-[a-f0-9]{32}$' -and (Test-Path -LiteralPath $fixture)) {
            $resolvedFixture = (Resolve-Path -LiteralPath $fixture).Path
            if ($resolvedFixture -ne [IO.Path]::GetFullPath($fixture)) { throw 'Unexpected fixture cleanup path.' }
            Remove-Item -LiteralPath $resolvedFixture -Recurse -Force
        }
    }
}
