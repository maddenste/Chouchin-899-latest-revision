# Copyright (C) 2026 Steve Madden
# SPDX-License-Identifier: GPL-3.0-or-later
# Package clock firmware with the user's own preserved configuration sectors.
# Offline only: this script never opens a debugger or accesses the board.
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$CodePath,
    [Parameter(Mandatory)][string]$OutputPath,
    [Parameter(Mandatory)][string]$ExpectedCodeHash,
    [Parameter(Mandatory)][string]$FactoryBackup,
    [Parameter(Mandatory)][string]$ExpectedFactoryHash
)

$ErrorActionPreference = 'Stop'
$factoryPath = $FactoryBackup
$expectedFactoryHash = $ExpectedFactoryHash
$flashLength = 0x200000
$settingsStart = 0x1FC000
$configStart = 0x1FE000

foreach ($item in @(@($factoryPath, $expectedFactoryHash), @($codePath, $expectedCodeHash))) {
    if (-not (Test-Path -LiteralPath $item[0] -PathType Leaf)) { throw "Missing file: $($item[0])" }
    $hash = (Get-FileHash -LiteralPath $item[0] -Algorithm SHA256).Hash
    if ($hash -ne $item[1]) { throw "Hash mismatch: $($item[0]); $hash" }
}
if (Test-Path -LiteralPath $outputPath) { throw "Refusing to overwrite: $outputPath" }

$factory = [IO.File]::ReadAllBytes($factoryPath)
$code = [IO.File]::ReadAllBytes($codePath)
if ($factory.Length -ne $flashLength -or $code.Length -lt 0x1000 -or
    $code[0] -ne 0x69 -or $code[1] -ne 0x5a -or $code[2] -ne 0 -or
    $code.Length -gt $settingsStart) {
    throw 'Unexpected flash length, firmware length, or first-slot boot header.'
}

$image = [byte[]]::new($flashLength)
[Array]::Fill($image, [byte]0xff)
[Array]::Copy($code, 0, $image, 0, $code.Length)
[Array]::Copy($factory, $configStart, $image, $configStart, $flashLength - $configStart)

for ($i = 0; $i -lt $flashLength; $i++) {
    $expected = if ($i -lt $code.Length) { $code[$i] }
        elseif ($i -ge $configStart) { $factory[$i] }
        else { 0xff }
    if ($image[$i] -ne $expected) { throw ('Layout mismatch at 0x{0:X}' -f $i) }
}

[IO.File]::WriteAllBytes($outputPath, $image)
$savedHash = (Get-FileHash -LiteralPath $outputPath -Algorithm SHA256).Hash
Write-Host "Created: $outputPath"
Write-Host "Length: $flashLength bytes; SHA-256: $savedHash"
Write-Host 'This was an offline build. No flash was read or written.'
