param(
	[Parameter(Mandatory = $true)]
	[string]$Name,

	[Parameter(Mandatory = $true)]
	[string]$DestinationRoot,

	[string]$ManifestPath = "",

	[string]$RootOverride,

	[switch]$Force
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($ManifestPath))
{
	$ManifestPath = Join-Path $PSScriptRoot "comparative_benchmarks\\dependencies.json"
}

function Get-ManifestEntry
{
	param(
		[string]$ManifestFile,
		[string]$DependencyName
	)

	if (-not (Test-Path -LiteralPath $ManifestFile))
	{
		throw "Comparative dependency manifest not found: $ManifestFile"
	}

	$manifest = ConvertTo-NativeObject (Get-Content -LiteralPath $ManifestFile -Raw | ConvertFrom-Json)
	if (-not $manifest.ContainsKey($DependencyName))
	{
		throw "Comparative dependency '$DependencyName' is not defined in $ManifestFile"
	}

	return $manifest[$DependencyName]
}

function ConvertTo-NativeObject
{
	param($InputObject)

	if ($null -eq $InputObject)
	{
		return $null
	}

	if (($InputObject -is [string]) -or ($InputObject.GetType().IsValueType))
	{
		return $InputObject
	}

	if ($InputObject -is [System.Collections.IDictionary])
	{
		$hash = @{}
		foreach ($key in $InputObject.Keys)
		{
			$hash[[string]$key] = ConvertTo-NativeObject $InputObject[$key]
		}
		return $hash
	}

	if (($InputObject -is [System.Collections.IEnumerable]) -and -not ($InputObject -is [string]))
	{
		$values = @()
		foreach ($item in $InputObject)
		{
			$values += ,(ConvertTo-NativeObject $item)
		}
		return $values
	}

	$properties = @($InputObject.PSObject.Properties)
	if ($InputObject.PSObject -and $properties.Length -gt 0)
	{
		$hash = @{}
		foreach ($property in $properties)
		{
			$hash[$property.Name] = ConvertTo-NativeObject $property.Value
		}
		return $hash
	}

	return $InputObject
}

function Test-ExpectedFiles
{
	param(
		[string]$Root,
		[object[]]$ExpectedFiles
	)

	foreach ($relative in $ExpectedFiles)
	{
		if (-not (Test-Path -LiteralPath (Join-Path $Root $relative)))
		{
			return $false
		}
	}

	return $true
}

function New-EmptyDirectory
{
	param([string]$Path)

	if (Test-Path -LiteralPath $Path)
	{
		$resolved = (Resolve-Path -LiteralPath $Path).Path
		Remove-Item -LiteralPath $resolved -Recurse -Force
	}

	New-Item -ItemType Directory -Path $Path -Force | Out-Null
}

function Write-Stamp
{
	param(
		[string]$StampPath,
		[string]$DependencyName,
		[hashtable]$Entry,
		[string]$ResolvedRoot
	)

	$stamp = @{
		name = $DependencyName
		version = $Entry.version
		source_kind = $Entry.source.kind
		root = $ResolvedRoot
	}

	$stamp | ConvertTo-Json | Set-Content -LiteralPath $StampPath -Encoding UTF8
}

function Read-Stamp
{
	param([string]$StampPath)

	if (-not (Test-Path -LiteralPath $StampPath))
	{
		return $null
	}

	return ConvertTo-NativeObject (Get-Content -LiteralPath $StampPath -Raw | ConvertFrom-Json)
}

function Use-OverrideRoot
{
	param(
		[string]$OverridePath,
		[hashtable]$Entry,
		[string]$DependencyName
	)

	if (-not (Test-Path -LiteralPath $OverridePath))
	{
		throw "Override root does not exist: $OverridePath"
	}

	$resolved = (Resolve-Path -LiteralPath $OverridePath).Path
	if (-not (Test-ExpectedFiles -Root $resolved -ExpectedFiles $Entry.layout.expected_files))
	{
		throw "Override root '$resolved' does not contain the expected files for '$DependencyName'"
	}

	return $resolved
}

$entry = Get-ManifestEntry -ManifestFile $ManifestPath -DependencyName $Name

if ($RootOverride)
{
	$overrideRoot = Use-OverrideRoot -OverridePath $RootOverride -Entry $entry -DependencyName $Name
	Write-Output $overrideRoot
	return
}

$destinationRoot = [System.IO.Path]::GetFullPath($DestinationRoot)
New-Item -ItemType Directory -Path $destinationRoot -Force | Out-Null

$targetRoot = Join-Path $destinationRoot $Name
$stampPath = Join-Path $targetRoot ".utf8_ranges_dependency.json"
$existingStamp = Read-Stamp -StampPath $stampPath

if (-not $Force -and $existingStamp)
{
	$stampVersion = [string]$existingStamp.version
	$stampKind = [string]$existingStamp.source_kind
	if ($stampVersion -eq [string]$entry.version `
		-and $stampKind -eq [string]$entry.source.kind `
		-and (Test-ExpectedFiles -Root $targetRoot -ExpectedFiles $entry.layout.expected_files))
	{
		Write-Output $targetRoot
		return
	}
}

New-EmptyDirectory -Path $targetRoot

$kind = [string]$entry.source.kind
switch ($kind)
{
	"github_release_asset_zip"
	{
		$repo = [string]$entry.source.repo
		$tag = [string]$entry.source.tag
		$asset = [string]$entry.source.asset
		$url = "https://github.com/$repo/releases/download/$tag/$asset"

		$tempZip = [System.IO.Path]::Combine([System.IO.Path]::GetTempPath(), ([System.IO.Path]::GetRandomFileName() + ".zip"))
		$tempExtractRoot = [System.IO.Path]::Combine([System.IO.Path]::GetTempPath(), [System.IO.Path]::GetRandomFileName())
		New-Item -ItemType Directory -Path $tempExtractRoot -Force | Out-Null
		try
		{
			Invoke-WebRequest -Uri $url -OutFile $tempZip
			Expand-Archive -LiteralPath $tempZip -DestinationPath $tempExtractRoot

			$sourceRoot = $tempExtractRoot
			if ($entry.layout.ContainsKey("root_subdirectory") -and $entry.layout.root_subdirectory)
			{
				$sourceRoot = Join-Path $tempExtractRoot ([string]$entry.layout.root_subdirectory)
			}

			if (-not (Test-ExpectedFiles -Root $sourceRoot -ExpectedFiles $entry.layout.expected_files))
			{
				throw "Fetched archive for '$Name' did not contain the expected files under '$sourceRoot'"
			}

			Get-ChildItem -LiteralPath $sourceRoot -Force | Copy-Item -Destination $targetRoot -Recurse -Force
		}
		finally
		{
			if (Test-Path -LiteralPath $tempZip)
			{
				Remove-Item -LiteralPath $tempZip -Force
			}
			if (Test-Path -LiteralPath $tempExtractRoot)
			{
				Remove-Item -LiteralPath $tempExtractRoot -Recurse -Force
			}
		}
	}

	"git_clone"
	{
		$repoUrl = [string]$entry.source.repo_url
		$tag = [string]$entry.source.tag
		& git clone --branch $tag --depth 1 $repoUrl $targetRoot
		if ($LASTEXITCODE -ne 0)
		{
			throw "git clone failed for '$Name' from '$repoUrl'"
		}

		if (-not (Test-ExpectedFiles -Root $targetRoot -ExpectedFiles $entry.layout.expected_files))
		{
			throw "Cloned repository for '$Name' did not contain the expected files under '$targetRoot'"
		}
	}

	default
	{
		throw "Unsupported comparative dependency source kind: $kind"
	}
}

Write-Stamp -StampPath $stampPath -DependencyName $Name -Entry $entry -ResolvedRoot $targetRoot
Write-Output $targetRoot
