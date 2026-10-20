param(
    [Parameter(Mandatory = $true)]
    [string]$EndstoneRoot,
    [Parameter(Mandatory = $true)]
    [string]$OutputDll,
    [string]$LlvmRoot = "$env:LOCALAPPDATA\Temp\hosihikari-llvm\clang+llvm-23.1.3-x86_64-pc-windows-msvc"
)

$ErrorActionPreference = "Stop"

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$EndstoneRoot = (Resolve-Path $EndstoneRoot).Path
$IncludeRoot = Join-Path $EndstoneRoot "include"
$OutputDll = [System.IO.Path]::GetFullPath($OutputDll)
New-Item -ItemType Directory -Force -Path (Split-Path $OutputDll -Parent) | Out-Null

$VersionFile = Join-Path $IncludeRoot "CMakeLists.txt"
$Version = "0.11.14"
if (Test-Path $VersionFile) {
    $Match = Select-String -Path $VersionFile -Pattern 'project\s*\(\s*endstone\s+VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)' | Select-Object -First 1
    if ($Match) { $Version = $Match.Matches[0].Groups[1].Value }
}

$GeneratedRoot = Join-Path $RepoRoot "artifacts\test-generated\include"
$GeneratedHeader = Join-Path $GeneratedRoot "endstone\version.h"
New-Item -ItemType Directory -Force -Path (Split-Path $GeneratedHeader -Parent) | Out-Null
$Parts = $Version.Split('.')
[System.IO.File]::WriteAllText(
    $GeneratedHeader,
    "#pragma once`n#define ENDSTONE_VERSION_MAJOR $($Parts[0])`n#define ENDSTONE_VERSION_MINOR $($Parts[1])`n#define ENDSTONE_VERSION_PATCH $($Parts[2])`n#define ENDSTONE_API_VERSION `"$($Parts[0]).$($Parts[1])`"`n#ifndef ENDSTONE_VERSION`n#define ENDSTONE_VERSION `"$Version`"`n#endif`n",
    (New-Object System.Text.UTF8Encoding($false)))

$Expected = Get-ChildItem "$env:LOCALAPPDATA\.xmake\packages\e\expected-lite" -Recurse -Filter expected.hpp -ErrorAction SilentlyContinue |
    Select-Object -First 1
if (-not $Expected) { throw "expected-lite headers were not found." }
$Nlohmann = Get-ChildItem "$env:LOCALAPPDATA\.xmake\packages\n\nlohmann_json" -Recurse -Filter json.hpp -ErrorAction SilentlyContinue |
    Select-Object -First 1
if (-not $Nlohmann) { throw "nlohmann_json headers were not found." }

$VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$VsRoot = & $VsWhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $VsRoot) { throw "Visual Studio with the C++ toolchain was not found." }

$Msvc = Get-ChildItem (Join-Path $VsRoot "VC\Tools\MSVC") | Sort-Object Name | Select-Object -Last 1
$SdkRoot = "C:\Program Files (x86)\Windows Kits\10\Include"
$Sdk = Get-ChildItem $SdkRoot | Sort-Object Name | Select-Object -Last 1

$Args = @(
    "/std:c++20",
    "/EHsc",
    "/MD",
    "/LD",
    "/DNOMINMAX",
    "/DWIN32_LEAN_AND_MEAN",
    "/I", $IncludeRoot,
    "/I", (Join-Path $IncludeRoot "endstone"),
    "/I", (Join-Path $RepoRoot "native\BindingShim\generated\win-x64-server\endstone"),
    "/I", $GeneratedRoot,
    "/I", (Split-Path (Split-Path $Expected.FullName -Parent) -Parent),
    "/I", (Split-Path (Split-Path $Nlohmann.FullName -Parent) -Parent),
    "/imsvc", (Join-Path $Msvc.FullName "include"),
    "/imsvc", (Join-Path $SdkRoot "$($Sdk.Name)\ucrt"),
    "/imsvc", (Join-Path $SdkRoot "$($Sdk.Name)\shared"),
    "/imsvc", (Join-Path $SdkRoot "$($Sdk.Name)\um"),
    (Join-Path $PSScriptRoot "main.cpp"),
    "/Fe:$OutputDll"
)

& (Join-Path $LlvmRoot "bin\clang-cl.exe") @Args
if ($LASTEXITCODE -ne 0) {
    throw "clang-cl exited with $LASTEXITCODE"
}

Write-Output "Built $OutputDll"
