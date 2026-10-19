# Copyright (C) 2026 Steve Madden
# SPDX-License-Identifier: GPL-3.0-or-later
# Offline regression only. Deliberately missing executables stop both real
# helpers before any debugger, connection, programming or output-file creation.
[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
$missingServer = Join-Path ([IO.Path]::GetTempPath()) ('ClockMissingServer-' + [Guid]::NewGuid().ToString('N') + '.exe')
$customOutput = Join-Path ([IO.Path]::GetTempPath()) 'clock log test with spaces'
$scripts = @('Program-TXW813-DCDC0-WriteOnly-OnPower.ps1', 'Read-TXW813-OnPower.ps1')
$shells = @((Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe'))
$currentShell = (Get-Process -Id $PID).Path
if ($currentShell -notin $shells) { $shells += $currentShell }
$cases = 0
foreach ($name in $scripts) {
    $scriptPath = Join-Path $PSScriptRoot $name
    $tokens = $null; $parseErrors = $null
    $ast = [Management.Automation.Language.Parser]::ParseFile($scriptPath, [ref]$tokens, [ref]$parseErrors)
    $parameter = $ast.ParamBlock.Parameters | Where-Object { $_.Name.VariablePath.UserPath -eq 'OutputRoot' }
    if ($parseErrors.Count -or $null -ne $parameter.DefaultValue) {
        throw "OutputRoot must be resolved after parameter binding: $name"
    }
    $baseArgs = @('-ServerPath', $missingServer, '-AlgorithmPath', ($missingServer + '.elf'),
                  '-InitScriptPath', ($missingServer + '.init'), '-DebuggerEndpoint', '127.0.0.1:1025')
    $expected = 'Missing debugger server:'
    if ($name -like 'Program-*') {
        $baseArgs += @('-CleanFullImage', '-ImagePath', ($missingServer + '.bin'))
        $expected = 'Missing file:'
    }
    foreach ($shell in $shells) {
        foreach ($mode in @('default', 'custom', 'relative')) {
            $arguments = $baseArgs
            if ($mode -eq 'custom') { $arguments += @('-OutputRoot', $customOutput) }
            if ($mode -eq 'relative') { $arguments += @('-OutputRoot', '.\clock-relative-logs') }
            $previousPreference = $ErrorActionPreference
            try {
                # A nonzero child exit is expected. Capture diagnostics in both
                # Windows PowerShell 5.1 and PowerShell 7 without changing policy.
                $ErrorActionPreference = 'Continue'
                $output = & $shell -NoLogo -NoProfile -NonInteractive -File $scriptPath @arguments 2>&1 | Out-String
                $exitCode = $LASTEXITCODE
            } finally { $ErrorActionPreference = $previousPreference }
            if ($exitCode -eq 0 -or $output -notmatch [regex]::Escape($expected) -or
                $output -notmatch [regex]::Escape($missingServer) -or
                $output -match 'Cannot bind argument|Writer armed|Preflight OK') {
                throw "Unexpected $mode invocation result for $name in ${shell}: $output"
            }
            ++$cases
        }
    }
    # Empty and whitespace values must fail explicitly, never silently choose
    # another folder. Use direct invocation to preserve empty string arguments.
    foreach ($empty in @('', '   ')) {
        $arguments = @{ ServerPath = $missingServer; AlgorithmPath = $missingServer;
                        InitScriptPath = $missingServer; DebuggerEndpoint = '127.0.0.1:1025'; OutputRoot = $empty }
        if ($name -like 'Program-*') { $arguments.CleanFullImage = $true; $arguments.ImagePath = $missingServer }
        $rejected = $false
        try { & $scriptPath @arguments | Out-Null }
        catch { $rejected = $_.Exception.Message -eq 'OutputRoot must be a non-empty filesystem folder.' }
        if (-not $rejected) { throw "Empty OutputRoot was not rejected clearly: $name" }
        ++$cases
    }
}
Write-Output "PASS: $cases log-folder cases; default/custom/relative direct launches and empty-value rejection. No hardware accessed or test files created."
# The expected failing child launches must not leave a failure status in callers.
$global:LASTEXITCODE = 0
