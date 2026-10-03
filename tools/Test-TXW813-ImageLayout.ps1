# Copyright (C) 2026 Steve Madden
# SPDX-License-Identifier: GPL-3.0-or-later
# Synthetic offline tests only; no programmer or real firmware is accessed.
[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$testDirectory = Join-Path ([IO.Path]::GetTempPath()) ('clock-image-test-' + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $testDirectory | Out-Null
try {
    $factoryPath = Join-Path $testDirectory 'synthetic-factory.bin'
    $factory = [byte[]]::new(0x200000)
    [Array]::Fill($factory, [byte]0xA5)
    [IO.File]::WriteAllBytes($factoryPath, $factory)
    $factoryHash = (Get-FileHash -LiteralPath $factoryPath).Hash
    foreach ($length in @(0x1000, 0x1FC000, 0x1FC001, 0x1FD000)) {
        $codePath = Join-Path $testDirectory ('synthetic-code-' + $length + '.bin')
        $outputPath = Join-Path $testDirectory ('synthetic-full-' + $length + '.bin')
        $code = [byte[]]::new($length)
        $code[0] = 0x69; $code[1] = 0x5A
        [IO.File]::WriteAllBytes($codePath, $code)
        $codeHash = (Get-FileHash -LiteralPath $codePath).Hash
        $rejected = $false
        try {
            & (Join-Path $PSScriptRoot 'Build-TXW813-DCDC0FullImage.ps1') -CodePath $codePath -OutputPath $outputPath -ExpectedCodeHash $codeHash -FactoryBackup $factoryPath -ExpectedFactoryHash $factoryHash
        } catch {
            if ($_.Exception.Message -ne 'Unexpected flash length, firmware length, or first-slot boot header.') { throw }
            $rejected = $true
        }
        if ($length -gt 0x1FC000) {
            if (-not $rejected -or (Test-Path -LiteralPath $outputPath)) { throw "Overlapping image accepted: $length" }
        } else {
            if ($rejected) { throw "Valid boundary rejected: $length" }
            $full = [IO.File]::ReadAllBytes($outputPath)
            if ($full.Length -ne 0x200000) { throw 'Incorrect FULL size.' }
            for ($i = 0; $i -lt $full.Length; ++$i) {
                $expected = if ($i -lt $length) { $code[$i] } elseif ($i -ge 0x1FE000) { 0xA5 } else { 0xFF }
                if ($full[$i] -ne $expected) { throw "Unexpected byte at $i" }
            }
        }
    }
    Write-Output 'PASS: valid images preserve settings sectors; overlapping images are rejected before output creation.'
} finally {
    # Only remove files made by this test in its uniquely named directory.
    Get-ChildItem -LiteralPath $testDirectory -File | Remove-Item
    Remove-Item -LiteralPath $testDirectory
}
