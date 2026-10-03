# Copyright (C) 2026 Steve Madden
# SPDX-License-Identifier: GPL-3.0-or-later
# Pure offline result classification; no files or hardware are accessed.
function Test-TXW813DumpResult {
    param(
        [long]$Size,
        [long]$ExpectedSize,
        [string]$ServerText,
        [string]$GdbText,
        [bool]$UsesGdb,
        [Nullable[int]]$GdbExitCode,
        [bool]$GdbTimedOut
    )
    if ($Size -ne $ExpectedSize -or $ExpectedSize -le 0) { return $false }
    $failurePattern = '(?im)ERROR:|Dump failed|Failed to load algorithm|Protocol error|Ignoring packet error|Remote communication error|Connection timed out|Cannot access memory|Remote connection closed'
    if ($ServerText -match $failurePattern) { return $false }
    if ($UsesGdb -and ($GdbTimedOut -or $null -eq $GdbExitCode -or
            $GdbExitCode -ne 0 -or $GdbText -match $failurePattern)) { return $false }
    return $true
}
