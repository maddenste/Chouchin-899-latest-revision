# Copyright (C) 2026 Steve Madden
# SPDX-License-Identifier: GPL-3.0-or-later
# Synthetic offline tests of backup acceptance; never starts a debugger.
[CmdletBinding()]
param()
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'Test-TXW813-DumpResult.ps1')
$testCases = @(
    @{ Name = 'clean GDB'; Accept = $true },
    @{ Name = 'short output'; Size = 4095; Accept = $false },
    @{ Name = 'nonzero exit'; GdbExitCode = 1; Accept = $false },
    @{ Name = 'missing exit'; GdbExitCode = $null; Accept = $false },
    @{ Name = 'timeout with full file'; GdbTimedOut = $true; Accept = $false },
    @{ Name = 'GDB dump error'; GdbText = 'Dump failed.'; Accept = $false },
    @{ Name = 'GDB protocol error'; GdbText = 'Protocol error with Rcmd'; Accept = $false },
    @{ Name = 'GDB packet error'; GdbText = 'Ignoring packet error, continuing...'; Accept = $false },
    @{ Name = 'server error'; ServerText = 'ERROR: target lost'; Accept = $false },
    @{ Name = 'clean server-only'; UsesGdb = $false; GdbExitCode = $null; Accept = $true },
    @{ Name = 'server-only dump error'; UsesGdb = $false; ServerText = 'Dump failed.'; Accept = $false },
    @{ Name = 'harmless warning'; GdbText = 'warning: No executable has been specified'; Accept = $true }
)
foreach ($case in $testCases) {
    $argsForTest = @{Size = 4096; ExpectedSize = 4096; ServerText = ''; GdbText = ''; UsesGdb = $true; GdbExitCode = 0; GdbTimedOut = $false}
    foreach ($key in $case.Keys) {
        if ($key -notin @('Name', 'Accept')) { $argsForTest[$key] = $case[$key] }
    }
    if ((Test-TXW813DumpResult @argsForTest) -ne $case.Accept) { throw "Result check failed: $($case.Name)" }
}
Write-Output "PASS: $($testCases.Count) backup-result cases; GDB failures, timeouts and incomplete files rejected."
