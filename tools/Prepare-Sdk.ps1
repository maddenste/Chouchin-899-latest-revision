# Copyright (C) 2026 Steve Madden
# SPDX-License-Identifier: GPL-3.0-or-later
# Copies your separately obtained SDK into ignored working directories.
# Never overwrites application overlays or bundles the vendor SDK for release.
[CmdletBinding()]
param([Parameter(Mandatory)][string]$SdkRoot)
$ErrorActionPreference = 'Stop'
$source = (Resolve-Path -LiteralPath $SdkRoot).Path
$destination = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\firmware\iot_sdk_work'))
foreach ($name in @('sdk','csky','libs','project')) {
    if (-not (Test-Path -LiteralPath (Join-Path $source $name) -PathType Container)) { throw "Missing SDK directory: $name" }
}
foreach ($name in @('sdk','csky','libs')) {
    $target = Join-Path $destination $name
    if (Test-Path -LiteralPath $target) { throw "SDK destination already exists: $target" }
}
foreach ($name in @('BinScript.exe','makecode.exe','crc.exe','loader.bin','parameter.cfg','parameter.bincfg','parameter_ui.cfg')) {
    $file = Join-Path $source ('project\' + $name)
    if (-not (Test-Path -LiteralPath $file -PathType Leaf)) { throw "Missing SDK packaging tool: $file" }
    if (Test-Path -LiteralPath (Join-Path $destination ('clock_project\' + $name))) { throw "Packaging destination already exists: $name" }
}
foreach ($name in @('app','params','psramCFG','utilities')) {
    if (-not (Test-Path -LiteralPath (Join-Path $source ('project\' + $name)) -PathType Container)) { throw "Missing SDK project directory: $name" }
    if (Test-Path -LiteralPath (Join-Path $destination ('clock_project\' + $name))) { throw "SDK project destination already exists: $name" }
}
foreach ($name in @('sdk','csky','libs')) { Copy-Item -LiteralPath (Join-Path $source $name) -Destination $destination -Recurse }
foreach ($name in @('app','params','psramCFG','utilities')) {
    Copy-Item -LiteralPath (Join-Path $source ('project\' + $name)) -Destination (Join-Path $destination 'clock_project') -Recurse
}
foreach ($name in @('BinScript.exe','makecode.exe','crc.exe','loader.bin','parameter.cfg','parameter.bincfg','parameter_ui.cfg')) {
    Copy-Item -LiteralPath (Join-Path $source ('project\' + $name)) -Destination (Join-Path $destination 'clock_project')
}
Write-Host 'SDK dependencies copied. Application overlays unchanged. No hardware accessed.'
