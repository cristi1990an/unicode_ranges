param(
	[string]$DataRoot = (Join-Path $PSScriptRoot "unicode_data\17.0.0"),
	[string]$UnicodeVersion = ""
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path $PSScriptRoot -Parent
$generatorSource = Join-Path $PSScriptRoot "gen_unicode_tables.rs"
$generatorExe = Join-Path $PSScriptRoot "gen_unicode_tables.exe"
$outputPath = Join-Path $repoRoot "unicode_ranges\unicode_tables_constexpr.hpp"

& rustc $generatorSource -o $generatorExe
try
{
	if ([string]::IsNullOrWhiteSpace($UnicodeVersion))
	{
		$leaf = Split-Path $DataRoot -Leaf
		if ($leaf -match '^\d+\.\d+\.\d+$')
		{
			$UnicodeVersion = $leaf
		}
	}

	if ([string]::IsNullOrWhiteSpace($UnicodeVersion))
	{
		$output = & $generatorExe $DataRoot
	}
	else
	{
		$output = & $generatorExe $DataRoot $UnicodeVersion
	}
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
