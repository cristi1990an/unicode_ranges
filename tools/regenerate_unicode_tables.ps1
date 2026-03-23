param(
	[string]$DataRoot = (Join-Path $PSScriptRoot "unicode_data\17.0.0")
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path $PSScriptRoot -Parent
$generatorSource = Join-Path $PSScriptRoot "gen_unicode_tables.rs"
$generatorExe = Join-Path $PSScriptRoot "gen_unicode_tables.exe"
$outputPath = Join-Path $repoRoot "utf8_ranges\unicode_tables.hpp"

& rustc $generatorSource -o $generatorExe
try
{
	$output = & $generatorExe $DataRoot
	$utf8NoBom = [System.Text.UTF8Encoding]::new($false)
	[System.IO.File]::WriteAllText($outputPath, ($output -join [Environment]::NewLine), $utf8NoBom)
}
finally
{
	if (Test-Path $generatorExe)
	{
		Remove-Item -Force $generatorExe
	}
}
