param(
    [string]$Version = "17.0.0"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$root = Join-Path $PSScriptRoot "unicode_data\$Version"
$ucdRoot = Join-Path $root "ucd"
$auxRoot = Join-Path $ucdRoot "auxiliary"
$emojiRoot = Join-Path $ucdRoot "emoji"
$extractedRoot = Join-Path $ucdRoot "extracted"

New-Item -ItemType Directory -Path $auxRoot -Force | Out-Null
New-Item -ItemType Directory -Path $emojiRoot -Force | Out-Null
New-Item -ItemType Directory -Path $extractedRoot -Force | Out-Null

$files = @(
    @{
        Uri = "https://www.unicode.org/Public/$Version/ucd/UnicodeData.txt"
        Path = Join-Path $ucdRoot "UnicodeData.txt"
    },
    @{
        Uri = "https://www.unicode.org/Public/$Version/ucd/CompositionExclusions.txt"
        Path = Join-Path $ucdRoot "CompositionExclusions.txt"
    },
    @{
        Uri = "https://www.unicode.org/Public/$Version/ucd/CaseFolding.txt"
        Path = Join-Path $ucdRoot "CaseFolding.txt"
    },
    @{
        Uri = "https://www.unicode.org/Public/$Version/ucd/Scripts.txt"
        Path = Join-Path $ucdRoot "Scripts.txt"
    },
    @{
        Uri = "https://www.unicode.org/Public/$Version/ucd/EastAsianWidth.txt"
        Path = Join-Path $ucdRoot "EastAsianWidth.txt"
    },
    @{
        Uri = "https://www.unicode.org/Public/$Version/ucd/LineBreak.txt"
        Path = Join-Path $ucdRoot "LineBreak.txt"
    },
    @{
        Uri = "https://www.unicode.org/Public/$Version/ucd/extracted/DerivedBidiClass.txt"
        Path = Join-Path $extractedRoot "DerivedBidiClass.txt"
    },
    @{
        Uri = "https://www.unicode.org/Public/$Version/ucd/auxiliary/GraphemeBreakProperty.txt"
        Path = Join-Path $auxRoot "GraphemeBreakProperty.txt"
    },
    @{
        Uri = "https://www.unicode.org/Public/$Version/ucd/auxiliary/WordBreakProperty.txt"
        Path = Join-Path $auxRoot "WordBreakProperty.txt"
    },
    @{
        Uri = "https://www.unicode.org/Public/$Version/ucd/auxiliary/SentenceBreakProperty.txt"
        Path = Join-Path $auxRoot "SentenceBreakProperty.txt"
    },
    @{
        Uri = "https://www.unicode.org/Public/$Version/ucd/auxiliary/GraphemeBreakTest.txt"
        Path = Join-Path $auxRoot "GraphemeBreakTest.txt"
    },
    @{
        Uri = "https://www.unicode.org/Public/$Version/ucd/emoji/emoji-data.txt"
        Path = Join-Path $emojiRoot "emoji-data.txt"
    },
    @{
        Uri = "https://www.unicode.org/Public/$Version/ucd/DerivedCoreProperties.txt"
        Path = Join-Path $ucdRoot "DerivedCoreProperties.txt"
    }
)

foreach ($file in $files)
{
    Write-Host "Downloading $($file.Uri)"
    Invoke-WebRequest -Uri $file.Uri -OutFile $file.Path
}

Write-Host "Unicode data updated under $root"
