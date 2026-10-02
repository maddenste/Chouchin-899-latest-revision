# Offline release packaging only. Never copies personal configuration.
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$AppPath,
    [Parameter(Mandatory)][string]$OutputPath
)
$ErrorActionPreference = 'Stop'
$expectedApp = 'E77836597F056C7627609B982298AACE6E8952CF9A5FDDBA80FA302699DE1707'
if ((Get-FileHash -LiteralPath $AppPath -Algorithm SHA256).Hash -ne $expectedApp) {
    throw 'This packager accepts the released R16 APP only.'
}
if (Test-Path -LiteralPath $OutputPath) { throw 'Output already exists.' }
$app = [IO.File]::ReadAllBytes((Resolve-Path -LiteralPath $AppPath).Path)
if ($app.Length -ne 326160 -or $app[0] -ne 0x69 -or $app[1] -ne 0x5a -or $app[2] -ne 0) {
    throw 'Unexpected APP size or boot header.'
}
$full = [byte[]]::new(0x200000)
[Array]::Fill($full, [byte]0xff)
[Array]::Copy($app, $full, $app.Length)
$stream = [IO.File]::Open($OutputPath, [IO.FileMode]::CreateNew)
try { $stream.Write($full, 0, $full.Length) } finally { $stream.Dispose() }
Get-FileHash -LiteralPath $OutputPath -Algorithm SHA256
Write-Host 'Clean 2 MiB FULL created. Flash at offset zero; enter Wi-Fi settings after restarting.'
