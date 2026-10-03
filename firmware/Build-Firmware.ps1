# Copyright (C) 2026 Steve Madden
# SPDX-License-Identifier: GPL-3.0-or-later
[CmdletBinding()]
param(
    [string]$Name = ('WiFi_Clock_v2_0_R16_' + (Get-Date -Format 'yyyyMMdd-HHmmss')),
    [string]$Python = 'python',
    [Parameter(Mandatory)][string]$TccPath,
    [string]$CdkMake = 'C:\C-Sky\CDK\cdk-make.exe',
    [string]$Toolchain = 'C:\C-Sky\CDKRepo\Toolchain\CKV2ElfMinilib\V3.10.29\R\bin',
    [Parameter(Mandatory)][string]$FactoryBackup,
    [Parameter(Mandatory)][string]$ExpectedFactoryHash,
    [string]$Node = 'node',
    [string]$NodeModules,
    [switch]$SkipBrowserTests
)
$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$project = Join-Path $root 'iot_sdk_work\clock_project'
$release = Join-Path $root 'release'
$tests = Join-Path $root 'tests'
$tcc = $TccPath
if ($Name -notmatch '^[A-Za-z0-9_-]+$') { throw 'Invalid image name' }
New-Item -ItemType Directory -Force $release, $tests | Out-Null
Push-Location $root
try {
    & $python tools\embed_web.py
    if ($LASTEXITCODE -ne 0) { throw 'Web asset generation failed' }
    & $tcc -I . tests\test_hc32_protocol.c txw_hc32_protocol.c -o tests\protocol.exe
    if ($LASTEXITCODE -ne 0) { throw 'UART test compilation failed' }
    & .\tests\protocol.exe
    if ($LASTEXITCODE -ne 0) { throw 'UART execution tests failed' }
    & $tcc -shared '-Wl,-export-all-symbols' clock_psk.c clock_settings.c clock_calendar.c clock_ntp_packet.c txw_hc32_protocol.c -o tests\backend.dll
    if ($LASTEXITCODE -ne 0) { throw 'Backend test compilation failed' }
    & $python tests\test_backend.py
    if ($LASTEXITCODE -ne 0) { throw 'Backend execution tests failed' }
    & $tcc -I tests -I . tests\test_ntp.c clock_ntp_packet.c -o tests\ntp.exe
    if ($LASTEXITCODE -ne 0) { throw 'NTP integration test compilation failed' }
    & .\tests\ntp.exe
    if ($LASTEXITCODE -ne 0) { throw 'NTP integration tests failed' }
    & $tcc -I tests -I . tests\test_app.c clock_psk.c clock_settings.c clock_calendar.c txw_hc32_protocol.c -o tests\app.exe
    if ($LASTEXITCODE -ne 0) { throw 'App test compilation failed' }
    & .\tests\app.exe
    if ($LASTEXITCODE -ne 0) { throw 'App integration tests failed' }
    & $tcc -I tests -I . tests\test_web.c clock_settings.c clock_calendar.c -o tests\web.exe
    if ($LASTEXITCODE -ne 0) { throw 'HTTP test compilation failed' }
    & .\tests\web.exe
    if ($LASTEXITCODE -ne 0) { throw 'HTTP execution tests failed' }
    if ($SkipBrowserTests) { Write-Warning 'Browser tests skipped; this build is not a fully tested release.' }
    else {
        if (-not $NodeModules) { throw 'Specify NodeModules containing playwright, or explicitly SkipBrowserTests.' }
        & $Node tests\test_web_ui.cjs $NodeModules
        if ($LASTEXITCODE -ne 0) { throw 'Browser UI tests failed' }
    }
    Write-Host 'Private HC32 binary emulation is not redistributed; see docs/TESTING.md.'

    $before = Get-Date
    $build = & $CdkMake -w TXW813_Minimal.cdkws -c FLASH -p txw81x -d build 2>&1
    $build | Out-File (Join-Path $tests 'build.log')
    if ($LASTEXITCODE -ne 0 -or ($build -join "`n") -match '(?i)error:|undefined reference|build failed|make.*\*\*\*') { throw 'Firmware build failed; see tests/build.log' }
    $elf = Get-Item (Join-Path $project 'Obj\txw81x.elf')
    if ($elf.LastWriteTime -lt $before.AddSeconds(-2)) { throw 'ELF was not rebuilt' }
    $symbols = & (Join-Path $toolchain 'csky-elfabiv2-nm.exe') $elf.FullName
    foreach ($required in @('clock_app_prepare','clock_app_command','clock_uart_loop','clock_psk_derive','clock_ntp_worker')) {
        if (($symbols -join "`n") -notmatch "\b$required\b") { throw "Missing application symbol: $required" }
    }
    Copy-Item $elf.FullName (Join-Path $project 'project.elf') -Force
    Copy-Item (Join-Path $project 'Obj\txw81x.ihex') (Join-Path $project 'project.hex') -Force
    Copy-Item (Join-Path $project 'Lst\txw81x.map') (Join-Path $project 'project.map') -Force
    Push-Location $project
    try {
        & .\BinScript.exe BinScript.BinScript
        if ($LASTEXITCODE -ne 0) { throw 'BinScript failed' }
        $packOutput = & .\makecode.exe
        $packOutput | Write-Output
        if ($LASTEXITCODE -ne 0) { throw 'SDK image packaging failed' }
        $packText = $packOutput -join "`n"
        if ($packText -notmatch 'successfully maked ([^\r\n]+\.bin)') { throw 'Packaged image filename missing' }
        $packName = $Matches[1].Trim()
        if ([IO.Path]::GetFileName($packName) -ne $packName) { throw 'Unexpected packaged path' }
    } finally { Pop-Location }
    $codePath = Join-Path $release ($Name + '_APP.bin')
    $fullPath = Join-Path $release ($Name + '_FULL.bin')
    if ((Test-Path $codePath) -or (Test-Path $fullPath)) { throw 'Release already exists; choose a new name' }
    Copy-Item (Join-Path $project $packName) $codePath
    $codeHash = (Get-FileHash $codePath -Algorithm SHA256).Hash
    & (Join-Path (Split-Path $root) 'tools\Build-TXW813-DCDC0FullImage.ps1') -CodePath $codePath -OutputPath $fullPath -ExpectedCodeHash $codeHash -FactoryBackup $FactoryBackup -ExpectedFactoryHash $ExpectedFactoryHash
    Copy-Item $elf.FullName (Join-Path $release ($Name + '.elf'))
    Copy-Item (Join-Path $project 'Lst\txw81x.map') (Join-Path $release ($Name + '.map'))
    Write-Output "Ready to flash: $fullPath"
    Write-Output 'No hardware access or flashing was performed.'
} finally { Pop-Location }
