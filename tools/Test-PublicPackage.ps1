[CmdletBinding()]
param([string]$Root = (Split-Path -Parent $PSScriptRoot))
$ErrorActionPreference = 'Stop'
$rootPath = (Resolve-Path -LiteralPath $Root).Path
$files = @(Get-ChildItem -LiteralPath $rootPath -Recurse -File)
$problems = [Collections.Generic.List[string]]::new()
$forbidden = '.bin','.elf','.hex','.ihex','.a','.exe','.dll','.map','.log','.err','.sr','.pcap','.zip','.7z'
$vendorNames = 'atcmd.c','atcmd.h','main.c','events.c','device.c','device.h','syscfg.c','syscfg.h','sys_config.h','project_config.h','psram_cfg.h','txw81x.cdkproj','makecode.ini','BinScript.BinScript','BinScript2.BinScript'
foreach ($file in $files) {
    if ($file.Extension -in $forbidden -or $file.Name -in $vendorNames) { $problems.Add("Excluded file present: $($file.Name)") }
    if ($file.Extension -eq '.ps1') {
        $parseTokens = $null; $parseErrors = $null
        [void][Management.Automation.Language.Parser]::ParseFile($file.FullName, [ref]$parseTokens, [ref]$parseErrors)
        if ($parseErrors.Count) { $problems.Add("PowerShell syntax: $($file.Name)") }
    }
    if ($file.Extension -in '.md','.ps1','.c','.h','.py','.cjs','.html' -or $file.Name -eq '.gitignore') {
        $body = [IO.File]::ReadAllText($file.FullName)
        if ($body -match '(gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,}|-----BEGIN (RSA |OPENSSH |EC )?PRIVATE KEY-----|AKIA[0-9A-Z]{16})') {
            $problems.Add("Possible secret: $($file.Name)")
        }
        if ($file.Extension -eq '.md') {
            foreach ($match in [regex]::Matches($body, '!?(?:\[[^\]]*\])\(([^)]+)\)')) {
                $target = $match.Groups[1].Value.Trim()
                if ($target -match '^(https?://|mailto:|#)') { continue }
                $target = ($target -split '#')[0]
                if ($target -and -not (Test-Path -LiteralPath (Join-Path $file.DirectoryName ([Uri]::UnescapeDataString($target))))) {
                    $problems.Add("Broken local link: $($file.Name) -> $target")
                }
            }
        }
    }
}
if ($problems.Count) { $problems | ForEach-Object { Write-Output $_ }; throw 'Public package checks failed.' }
Write-Output "PASS: $($files.Count) files; exclusion policy, high-confidence secret patterns, PowerShell syntax and local Markdown links."
Write-Output 'Pattern scanning is not a complete credential or copyright audit; inspect the approved inventory and screenshots.'
