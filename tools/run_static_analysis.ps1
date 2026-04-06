[CmdletBinding()]
param(
	[string[]]$Project = @("unicode_ranges.vcxproj"),
	[string]$Configuration = "Debug",
	[string]$Platform = "x64",
	[string]$PlatformToolset = "",
	[switch]$TreatWarningsAsErrors,
	[switch]$Rebuild
)

$ErrorActionPreference = "Stop"

function Get-MSBuildPath {
	$vswherePath = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
	if (Test-Path -LiteralPath $vswherePath) {
		$installationPath = & $vswherePath -latest -products * -property installationPath
		if ($installationPath) {
			$msbuildPath = Join-Path $installationPath "MSBuild\Current\Bin\MSBuild.exe"
			if (Test-Path -LiteralPath $msbuildPath) {
				return $msbuildPath
			}
		}
	}

	$fallbackPaths = @(
		"C:\Program Files\Microsoft Visual Studio\18\Insiders\MSBuild\Current\Bin\MSBuild.exe",
		"C:\Program Files\Microsoft Visual Studio\2022\Preview\MSBuild\Current\Bin\MSBuild.exe",
		"C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe",
		"C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe",
		"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe",
		"C:\Program Files\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
	)

	foreach ($candidate in $fallbackPaths) {
		if (Test-Path -LiteralPath $candidate) {
			return $candidate
		}
	}

	throw "Could not locate MSBuild.exe. Install Visual Studio Build Tools or edit tools/run_static_analysis.ps1 with your local MSBuild path."
}

$msbuildPath = Get-MSBuildPath
$target = if ($Rebuild) { "Rebuild" } else { "Build" }
$treatWarnings = if ($TreatWarningsAsErrors) { "true" } else { "false" }

foreach ($projectPath in $Project) {
	if (-not (Test-Path -LiteralPath $projectPath)) {
		throw "Project '$projectPath' was not found."
	}

	$displayToolset = if ([string]::IsNullOrWhiteSpace($PlatformToolset)) { "project default" } else { $PlatformToolset }
	Write-Host "Running MSVC static analysis for '$projectPath' ($Configuration|$Platform, toolset: $displayToolset)..." -ForegroundColor Cyan

	$arguments = @(
		$projectPath
		"/t:$target"
		"/p:Configuration=$Configuration"
		"/p:Platform=$Platform"
		"/p:RunCodeAnalysis=true"
		"/p:TreatWarningAsError=$treatWarnings"
	)

	if (-not [string]::IsNullOrWhiteSpace($PlatformToolset)) {
		$arguments += "/p:PlatformToolset=$PlatformToolset"
	}

	& $msbuildPath @arguments

	if ($LASTEXITCODE -ne 0) {
		throw "Static analysis failed for '$projectPath'."
	}
}
