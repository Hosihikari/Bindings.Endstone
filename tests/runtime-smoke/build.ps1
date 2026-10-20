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

$ServerFile = Get-ChildItem (Join-Path $RepoRoot "src\Hosihikari.Bindings.Endstone.Interop\Generated") -Filter "Server_*.g.cs" |
    Select-Object -First 1
if (-not $ServerFile) { throw "Generated Server interop was not found." }
$ServerText = Get-Content $ServerFile.FullName -Raw
$NameMatch = [regex]::Match($ServerText, 'getName\(\)[\s\S]*?TablePtr->f(\d+)')
$VersionMatch = [regex]::Match($ServerText, 'getVersion\(\)[\s\S]*?TablePtr->f(\d+)')
if (-not $NameMatch.Success -or -not $VersionMatch.Success) {
    throw "Could not find Server::getName/getVersion table indices."
}

function Get-TableIndex {
    param([string]$Path, [string]$Pattern, [string]$Description)
    $File = Get-ChildItem (Join-Path $RepoRoot "src\Hosihikari.Bindings.Endstone.Interop\Generated") -Filter $Path |
        Select-Object -First 1
    if (-not $File) { throw "Generated interop for $Description was not found." }
    $Text = Get-Content $File.FullName -Raw
    $Match = [regex]::Match($Text, $Pattern)
    if (-not $Match.Success) { throw "Could not find the $Description table index." }
    return $Match.Groups[1].Value
}

$TranslatableCreate = Get-TableIndex "Translatable_*.g.cs" 'Create\(string text, string\[\] @params\)[\s\S]*?TablePtr->f(\d+)' "Translatable::Create"
$TranslatableEmpty = Get-TableIndex "Translatable_*.g.cs" 'empty\(\)[\s\S]*?TablePtr->f(\d+)' "Translatable::empty"
$TranslatableText = Get-TableIndex "Translatable_*.g.cs" 'getText\(\)[\s\S]*?TablePtr->f(\d+)' "Translatable::getText"
$TranslatableDestroy = Get-TableIndex "Translatable_*.g.cs" 'Dispose\(\)[\s\S]*?TablePtr->f(\d+)' "Translatable::Dispose"
$ArrayTagCreate = Get-TableIndex "ArrayTag_int__*.g.cs" 'Create\(int\[\] v\)[\s\S]*?TablePtr->f(\d+)' "ArrayTag_int_::Create"
$ArrayTagSize = Get-TableIndex "ArrayTag_int__*.g.cs" 'size\(\)[\s\S]*?TablePtr->f(\d+)' "ArrayTag_int_::size"
$ArrayTagDestroy = Get-TableIndex "ArrayTag_int__*.g.cs" 'Dispose\(\)[\s\S]*?TablePtr->f(\d+)' "ArrayTag_int_::Dispose"
$ServerPluginManager = Get-TableIndex "Server_*.g.cs" 'getPluginManager\(\)[\s\S]*?TablePtr->f(\d+)' "Server::getPluginManager"
$PluginManagerPlugins = Get-TableIndex "PluginManager_*.g.cs" 'getPlugins\(\)[\s\S]*?TablePtr->f(\d+)' "PluginManager::getPlugins"
$ServerCreateScoreboard = Get-TableIndex "Server_*.g.cs" 'createScoreboard\(\)[\s\S]*?TablePtr->f(\d+)' "Server::createScoreboard"
$ScoreboardClearSlot = Get-TableIndex "Scoreboard_*.g.cs" 'clearSlot\(int slot\)[\s\S]*?TablePtr->f(\d+)' "Scoreboard::clearSlot"

$GeneratedDirectory = Join-Path $RepoRoot "src\Hosihikari.Bindings.Endstone.Interop\Generated"
$ActionFormCreate = Get-TableIndex "ActionForm_*.g.cs" 'Create\(\)[\s\S]*?TablePtr->f(\d+)' "ActionForm::Create"
$ActionFormSetContent = Get-TableIndex "ActionForm_*.g.cs" 'setContent\([\s\S]*?TablePtr->f(\d+)' "ActionForm::setContent"
$ActionFormGetContent = Get-TableIndex "ActionForm_*.g.cs" 'getContent\(\)[\s\S]*?TablePtr->f(\d+)' "ActionForm::getContent"
$ActionFormDestroy = Get-TableIndex "ActionForm_*.g.cs" 'Dispose\(\)[\s\S]*?TablePtr->f(\d+)' "ActionForm::Dispose"

$VariantFile = Get-ChildItem $GeneratedDirectory -Filter "Variant_*.g.cs" |
    Where-Object { (Get-Content $_.FullName -Raw) -match 'CreateTranslatable' } |
    Select-Object -First 1
if (-not $VariantFile) { throw "The string/Translatable variant wrapper was not found." }
$VariantText = Get-Content $VariantFile.FullName -Raw
$VariantCreateString = [regex]::Match($VariantText, 'CreateString\(string value\)[\s\S]*?TablePtr->f(\d+)').Groups[1].Value
$VariantIndex = [regex]::Match($VariantText, 'public int Index => NativeMethods\.TablePtr->f(\d+)').Groups[1].Value
$VariantGetString = [regex]::Match($VariantText, 'AsString\(\)[\s\S]*?TablePtr->f(\d+)').Groups[1].Value
$VariantDestroy = [regex]::Match($VariantText, 'Dispose\(\)[\s\S]*?TablePtr->f(\d+)').Groups[1].Value
if (-not $VariantCreateString -or -not $VariantIndex -or -not $VariantGetString -or -not $VariantDestroy) {
    throw "Could not find the string/Translatable variant table indices."
}

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
    "/DHS_NAME_INDEX=$($NameMatch.Groups[1].Value)",
    "/DHS_VERSION_INDEX=$($VersionMatch.Groups[1].Value)",
    "/DHS_TRANSLATABLE_CREATE_INDEX=$TranslatableCreate",
    "/DHS_TRANSLATABLE_EMPTY_INDEX=$TranslatableEmpty",
    "/DHS_TRANSLATABLE_TEXT_INDEX=$TranslatableText",
    "/DHS_TRANSLATABLE_DESTROY_INDEX=$TranslatableDestroy",
    "/DHS_ARRAYTAG_INT_CREATE_INDEX=$ArrayTagCreate",
    "/DHS_ARRAYTAG_INT_SIZE_INDEX=$ArrayTagSize",
    "/DHS_ARRAYTAG_INT_DESTROY_INDEX=$ArrayTagDestroy",
    "/DHS_SERVER_PLUGIN_MANAGER_INDEX=$ServerPluginManager",
    "/DHS_PLUGIN_MANAGER_PLUGINS_INDEX=$PluginManagerPlugins",
    "/DHS_SERVER_CREATE_SCOREBOARD_INDEX=$ServerCreateScoreboard",
    "/DHS_SCOREBOARD_CLEAR_SLOT_INDEX=$ScoreboardClearSlot",
    "/DHS_ACTION_FORM_CREATE_INDEX=$ActionFormCreate",
    "/DHS_ACTION_FORM_SET_CONTENT_INDEX=$ActionFormSetContent",
    "/DHS_ACTION_FORM_GET_CONTENT_INDEX=$ActionFormGetContent",
    "/DHS_ACTION_FORM_DESTROY_INDEX=$ActionFormDestroy",
    "/DHS_VARIANT_CREATE_STRING_INDEX=$VariantCreateString",
    "/DHS_VARIANT_INDEX_INDEX=$VariantIndex",
    "/DHS_VARIANT_GET_STRING_INDEX=$VariantGetString",
    "/DHS_VARIANT_DESTROY_INDEX=$VariantDestroy",
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
