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

$GeneratedDirectory = Join-Path $RepoRoot "src\Hosihikari.Bindings.Endstone.Interop\Generated"
$NativeMethodsFile = Get-ChildItem $GeneratedDirectory -Filter "NativeMethods.g.cs" | Select-Object -First 1
if (-not $NativeMethodsFile) { throw "NativeMethods.g.cs was not found." }
$NativeMethodsText = Get-Content $NativeMethodsFile.FullName -Raw
$TableFieldMatches = [regex]::Matches($NativeMethodsText, 'public delegate\* unmanaged\[Cdecl\]<[^;]+> (hs_[A-Za-z0-9_]+);')
$TableFieldIndex = @{}
for ($FieldIndex = 0; $FieldIndex -lt $TableFieldMatches.Count; $FieldIndex++) {
    $TableFieldIndex[$TableFieldMatches[$FieldIndex].Groups[1].Value] = $FieldIndex
}
if ($TableFieldIndex.Count -eq 0) { throw "No table fields were found in NativeMethods.g.cs." }

function Resolve-TableField {
    param([string]$Name)
    if ([string]::IsNullOrEmpty($Name)) { return "" }
    if (-not $TableFieldIndex.ContainsKey($Name)) { throw "Unknown table field '$Name'." }
    return [string]$TableFieldIndex[$Name]
}

$ServerFile = Get-ChildItem (Join-Path $RepoRoot "src\Hosihikari.Bindings.Endstone.Interop\Generated") -Filter "Server_*.g.cs" |
    Select-Object -First 1
if (-not $ServerFile) { throw "Generated Server interop was not found." }
$ServerText = Get-Content $ServerFile.FullName -Raw
$NameMatch = [regex]::Match($ServerText, 'getName\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)')
$VersionMatch = [regex]::Match($ServerText, 'getVersion\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)')
if (-not $NameMatch.Success -or -not $VersionMatch.Success) {
    throw "Could not find Server::getName/getVersion table indices."
}

function Get-TableIndex {
    param([string]$Path, [string]$Pattern, [string]$Description)
    $File = Get-ChildItem $GeneratedDirectory -Filter $Path |
        Select-Object -First 1
    if (-not $File) { throw "Generated interop for $Description was not found." }
    $Text = Get-Content $File.FullName -Raw
    $Match = [regex]::Match($Text, $Pattern)
    if (-not $Match.Success) { throw "Could not find the $Description table index." }
    return Resolve-TableField $Match.Groups[1].Value
}

$TranslatableCreate = Get-TableIndex "Translatable_*.g.cs" 'Create\(string text, string\[\] @params\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Translatable::Create"
$TranslatableEmpty = Get-TableIndex "Translatable_*.g.cs" 'empty\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Translatable::empty"
$TranslatableText = Get-TableIndex "Translatable_*.g.cs" 'getText\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Translatable::getText"
$TranslatableDestroy = Get-TableIndex "Translatable_*.g.cs" 'Dispose\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Translatable::Dispose"
$ArrayTagCreate = Get-TableIndex "ArrayTag_int__*.g.cs" 'Create\(int\[\] v\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ArrayTag_int_::Create"
$ArrayTagSize = Get-TableIndex "ArrayTag_int__*.g.cs" 'size\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ArrayTag_int_::size"
$ArrayTagDestroy = Get-TableIndex "ArrayTag_int__*.g.cs" 'Dispose\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ArrayTag_int_::Dispose"
$ServerPluginManager = Get-TableIndex "Server_*.g.cs" 'getPluginManager\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Server::getPluginManager"
$PluginManagerPlugins = Get-TableIndex "PluginManager_*.g.cs" 'getPlugins\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "PluginManager::getPlugins"
$ServerCreateScoreboard = Get-TableIndex "Server_*.g.cs" 'createScoreboard\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Server::createScoreboard"
$ScoreboardClearSlot = Get-TableIndex "Scoreboard_*.g.cs" 'clearSlot\(int slot\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Scoreboard::clearSlot"
$ScoreboardObjectives = Get-TableIndex "Scoreboard_*.g.cs" 'getObjectives\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Scoreboard::getObjectives"
$ServerBanList = Get-TableIndex "Server_*.g.cs" 'getBanList\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Server::getBanList"
$PlayerBanEntries = Get-TableIndex "PlayerBanList_*.g.cs" 'getEntries\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "PlayerBanList::getEntries"
$PlayerBanEntry = Get-TableIndex "PlayerBanList_*.g.cs" 'getBanEntry\(string name\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "PlayerBanList::getBanEntry"
$PlayerBanAdd = Get-TableIndex "PlayerBanList_*.g.cs" 'addBan\(string name, string\? reason, long duration, string\? source\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "PlayerBanList::addBan"
$PlayerBanRemove = Get-TableIndex "PlayerBanList_*.g.cs" 'removeBan\(string name\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "PlayerBanList::removeBan"
$PlayerBanned = Get-TableIndex "PlayerBanList_*.g.cs" 'isBanned\(string name\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "PlayerBanList::isBanned"
$ServerItemFactory = Get-TableIndex "Server_*.g.cs" 'getItemFactory\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Server::getItemFactory"
$ItemFactoryGetMeta = Get-TableIndex "ItemFactory_*.g.cs" 'getItemMeta\([\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ItemFactory::getItemMeta"
$ItemMetaAddEnchant = Get-TableIndex "ItemMeta_*.g.cs" 'addEnchant\([\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ItemMeta::addEnchant"
$ItemMetaGetEnchants = Get-TableIndex "ItemMeta_*.g.cs" 'getEnchants\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ItemMeta::getEnchants"
$ItemMetaDestroy = Get-TableIndex "ItemMeta_*.g.cs" 'Dispose\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ItemMeta::Dispose"
$ModalFormAddControl = Get-TableIndex "ModalForm_*.g.cs" 'addControl\([\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ModalForm::addControl"
$ModalFormGetControls = Get-TableIndex "ModalForm_*.g.cs" 'getControls\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ModalForm::getControls"
$DropdownDestroy = Get-TableIndex "Dropdown_*.g.cs" 'Dispose\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Dropdown::Dispose"
$ControlVariantFile = Get-ChildItem $GeneratedDirectory -Filter "Variant_*.g.cs" |
    Where-Object { (Get-Content $_.FullName -Raw) -match 'AsStepSlider' } |
    Select-Object -First 1
if (-not $ControlVariantFile) { throw "The form-control variant wrapper was not found." }
$ControlVariantText = Get-Content $ControlVariantFile.FullName -Raw
$ControlVariantCreateDropdown = Resolve-TableField ([regex]::Match($ControlVariantText, 'CreateDropdown\([\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)').Groups[1].Value)
$ControlVariantIndex = Resolve-TableField ([regex]::Match($ControlVariantText, 'public int Index => NativeMethods\.TablePtr->(hs_[A-Za-z0-9_]+)').Groups[1].Value)
$ControlVariantAsDropdown = Resolve-TableField ([regex]::Match($ControlVariantText, 'AsDropdown\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)').Groups[1].Value)
$ControlVariantDestroy = Resolve-TableField ([regex]::Match($ControlVariantText, 'Dispose\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)').Groups[1].Value)
if (-not $ControlVariantCreateDropdown -or -not $ControlVariantIndex -or -not $ControlVariantAsDropdown -or -not $ControlVariantDestroy) {
    throw "Could not find the form-control variant table indices."
}
$PermissionCreate = Get-TableIndex "Permission_*.g.cs" 'Create\(string name, [\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Permission::Create"
$PermissionGetChildren = Get-TableIndex "Permission_*.g.cs" 'getChildren\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Permission::getChildren"
$PermissionDestroy = Get-TableIndex "Permission_*.g.cs" 'Dispose\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Permission::Dispose"
$PermissionName = Get-TableIndex "Permission_*.g.cs" 'getName\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Permission::getName"
$PluginManagerAddPermission = Get-TableIndex "PluginManager_*.g.cs" 'addPermission\([\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "PluginManager::addPermission"
$CommandCreate = Get-TableIndex "Command_*.g.cs" 'Create\(string name, [\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Command::Create"
$CommandGetAliases = Get-TableIndex "Command_*.g.cs" 'getAliases\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Command::getAliases"
$CommandDestroy = Get-TableIndex "Command_*.g.cs" 'Dispose\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Command::Dispose"
$ItemTypeIdCreate = Get-TableIndex "Identifier_endstone__ItemType_*.g.cs" 'Create[A-Za-z0-9_]*\(string s\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Identifier<ItemType>::Create"
$ItemTypeIdKey = Get-TableIndex "Identifier_endstone__ItemType_*.g.cs" 'getKey\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Identifier<ItemType>::getKey"
$ItemTypeIdDestroy = Get-TableIndex "Identifier_endstone__ItemType_*.g.cs" 'Dispose\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Identifier<ItemType>::Dispose"
$ItemStackCreate = Get-TableIndex "ItemStack_*.g.cs" 'Create\([\s\S]*?int amount, int data\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ItemStack::Create"
$ItemStackGetAmount = Get-TableIndex "ItemStack_*.g.cs" 'getAmount\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ItemStack::getAmount"
$ItemStackGetData = Get-TableIndex "ItemStack_*.g.cs" 'getData\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ItemStack::getData"
$ItemStackAssign = Get-TableIndex "ItemStack_*.g.cs" 'operator_assign_[A-Za-z0-9_]+\([\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ItemStack::operator="
$ItemStackDestroy = Get-TableIndex "ItemStack_*.g.cs" 'Dispose\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ItemStack::Dispose"
$ServerCreateBlockData = Get-TableIndex "Server_*.g.cs" 'createBlockData\(string type, [\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Server::createBlockData"
$BlockDataGetBlockStates = Get-TableIndex "BlockData_*.g.cs" 'getBlockStates\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "BlockData::getBlockStates"
$BlockDataDestroy = Get-TableIndex "BlockData_*.g.cs" 'Dispose\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "BlockData::Dispose"
$BoolVariantFile = Get-ChildItem $GeneratedDirectory -Filter "Variant_*.g.cs" |
    Where-Object { (Get-Content $_.FullName -Raw) -match 'AsInt32' -and (Get-Content $_.FullName -Raw) -match 'AsBool' } |
    Select-Object -First 1
if (-not $BoolVariantFile) { throw "The bool/string/int variant wrapper was not found." }
$BoolVariantText = Get-Content $BoolVariantFile.FullName -Raw
$BoolVariantIndex = Resolve-TableField ([regex]::Match($BoolVariantText, 'public int Index => NativeMethods\.TablePtr->(hs_[A-Za-z0-9_]+)').Groups[1].Value)
$BoolVariantAsBool = Resolve-TableField ([regex]::Match($BoolVariantText, 'AsBool\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)').Groups[1].Value)
$BoolVariantDestroy = Resolve-TableField ([regex]::Match($BoolVariantText, 'Dispose\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)').Groups[1].Value)
if (-not $BoolVariantIndex -or -not $BoolVariantAsBool -or -not $BoolVariantDestroy) {
    throw "Could not find the bool/string/int variant table indices."
}
$ServerCommandSender = Get-TableIndex "Server_*.g.cs" 'getCommandSender\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Server::getCommandSender"
$PermissiblePermissionLevel = Get-TableIndex "Permissible_*.g.cs" 'getPermissionLevel\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "Permissible::getPermissionLevel"
$CommandSenderName = Get-TableIndex "CommandSender_*.g.cs" 'getName\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "CommandSender::getName"
$WrapperCreate = Get-TableIndex "CommandSenderWrapper_*.g.cs" 'Create\([\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "CommandSenderWrapper::Create"
$WrapperPermissionLevel = Get-TableIndex "CommandSenderWrapper_*.g.cs" 'getPermissionLevel\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "CommandSenderWrapper::getPermissionLevel"
$WrapperName = Get-TableIndex "CommandSenderWrapper_*.g.cs" 'getName\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "CommandSenderWrapper::getName"
$WrapperDestroy = Get-TableIndex "CommandSenderWrapper_*.g.cs" 'Dispose\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "CommandSenderWrapper::Dispose"
$WrapperAsCommandSender = Get-TableIndex "CommandSenderWrapper_*.g.cs" 'AsCommandSender\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "CommandSenderWrapper::AsCommandSender"
$CommandSenderTryAsWrapper = Get-TableIndex "CommandSender_*.g.cs" 'TryAsCommandSenderWrapper\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "CommandSender::TryAsCommandSenderWrapper"
$CommandSenderTryAsActor = Get-TableIndex "CommandSender_*.g.cs" 'TryAsActor\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "CommandSender::TryAsActor"

$ActionFormCreate = Get-TableIndex "ActionForm_*.g.cs" 'Create\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ActionForm::Create"
$ActionFormSetContent = Get-TableIndex "ActionForm_*.g.cs" 'setContent\([\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ActionForm::setContent"
$ActionFormGetContent = Get-TableIndex "ActionForm_*.g.cs" 'getContent\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ActionForm::getContent"
$ActionFormDestroy = Get-TableIndex "ActionForm_*.g.cs" 'Dispose\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ActionForm::Dispose"
$ActionFormSetOnSubmit = Get-TableIndex "ActionForm_*.g.cs" 'setOnSubmit\([\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ActionForm::setOnSubmit"
$ActionFormGetOnSubmit = Get-TableIndex "ActionForm_*.g.cs" 'getOnSubmit\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)' "ActionForm::getOnSubmit"

$CallableFile = Get-ChildItem $GeneratedDirectory -Filter "Function_*.g.cs" |
    Where-Object { (Get-Content $_.FullName -Raw) -match 'Invoke\(nint arg0, int arg1\)' } |
    Select-Object -First 1
if (-not $CallableFile) { throw "The Player*/int callable wrapper was not found." }
$CallableText = Get-Content $CallableFile.FullName -Raw
$CallableInvoke = Resolve-TableField ([regex]::Match($CallableText, 'Invoke\(nint arg0, int arg1\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)').Groups[1].Value)
$CallableDestroy = Resolve-TableField ([regex]::Match($CallableText, 'Dispose\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)').Groups[1].Value)
if (-not $CallableInvoke -or -not $CallableDestroy) {
    throw "Could not find the Player*/int callable table indices."
}

$VariantFile = Get-ChildItem $GeneratedDirectory -Filter "Variant_*.g.cs" |
    Where-Object { (Get-Content $_.FullName -Raw) -match 'CreateTranslatable' } |
    Select-Object -First 1
if (-not $VariantFile) { throw "The string/Translatable variant wrapper was not found." }
$VariantText = Get-Content $VariantFile.FullName -Raw
$VariantCreateString = Resolve-TableField ([regex]::Match($VariantText, 'CreateString\(string value\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)').Groups[1].Value)
$VariantIndex = Resolve-TableField ([regex]::Match($VariantText, 'public int Index => NativeMethods\.TablePtr->(hs_[A-Za-z0-9_]+)').Groups[1].Value)
$VariantGetString = Resolve-TableField ([regex]::Match($VariantText, 'AsString\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)').Groups[1].Value)
$VariantDestroy = Resolve-TableField ([regex]::Match($VariantText, 'Dispose\(\)[\s\S]*?TablePtr->(hs_[A-Za-z0-9_]+)').Groups[1].Value)
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
    "/DHS_NAME_INDEX=$(Resolve-TableField $NameMatch.Groups[1].Value)",
    "/DHS_VERSION_INDEX=$(Resolve-TableField $VersionMatch.Groups[1].Value)",
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
    "/DHS_SCOREBOARD_OBJECTIVES_INDEX=$ScoreboardObjectives",
    "/DHS_SERVER_BAN_LIST_INDEX=$ServerBanList",
    "/DHS_PLAYER_BAN_ENTRIES_INDEX=$PlayerBanEntries",
    "/DHS_PLAYER_BAN_ENTRY_INDEX=$PlayerBanEntry",
    "/DHS_PLAYER_BAN_ADD_INDEX=$PlayerBanAdd",
    "/DHS_PLAYER_BAN_REMOVE_INDEX=$PlayerBanRemove",
    "/DHS_PLAYER_BANNED_INDEX=$PlayerBanned",
    "/DHS_SERVER_ITEM_FACTORY_INDEX=$ServerItemFactory",
    "/DHS_ITEM_FACTORY_GET_META_INDEX=$ItemFactoryGetMeta",
    "/DHS_ITEM_META_ADD_ENCHANT_INDEX=$ItemMetaAddEnchant",
    "/DHS_ITEM_META_GET_ENCHANTS_INDEX=$ItemMetaGetEnchants",
    "/DHS_ITEM_META_DESTROY_INDEX=$ItemMetaDestroy",
    "/DHS_MODAL_FORM_ADD_CONTROL_INDEX=$ModalFormAddControl",
    "/DHS_MODAL_FORM_GET_CONTROLS_INDEX=$ModalFormGetControls",
    "/DHS_DROPDOWN_DESTROY_INDEX=$DropdownDestroy",
    "/DHS_CONTROL_VARIANT_CREATE_DROPDOWN_INDEX=$ControlVariantCreateDropdown",
    "/DHS_CONTROL_VARIANT_INDEX_INDEX=$ControlVariantIndex",
    "/DHS_CONTROL_VARIANT_AS_DROPDOWN_INDEX=$ControlVariantAsDropdown",
    "/DHS_CONTROL_VARIANT_DESTROY_INDEX=$ControlVariantDestroy",
    "/DHS_PERMISSION_CREATE_INDEX=$PermissionCreate",
    "/DHS_PERMISSION_GET_CHILDREN_INDEX=$PermissionGetChildren",
    "/DHS_PERMISSION_DESTROY_INDEX=$PermissionDestroy",
    "/DHS_PERMISSION_NAME_INDEX=$PermissionName",
    "/DHS_PLUGIN_MANAGER_ADD_PERMISSION_INDEX=$PluginManagerAddPermission",
    "/DHS_COMMAND_CREATE_INDEX=$CommandCreate",
    "/DHS_COMMAND_GET_ALIASES_INDEX=$CommandGetAliases",
    "/DHS_COMMAND_DESTROY_INDEX=$CommandDestroy",
    "/DHS_ITEM_TYPE_ID_CREATE_INDEX=$ItemTypeIdCreate",
    "/DHS_ITEM_TYPE_ID_KEY_INDEX=$ItemTypeIdKey",
    "/DHS_ITEM_TYPE_ID_DESTROY_INDEX=$ItemTypeIdDestroy",
    "/DHS_ITEM_STACK_CREATE_INDEX=$ItemStackCreate",
    "/DHS_ITEM_STACK_GET_AMOUNT_INDEX=$ItemStackGetAmount",
    "/DHS_ITEM_STACK_GET_DATA_INDEX=$ItemStackGetData",
    "/DHS_ITEM_STACK_ASSIGN_INDEX=$ItemStackAssign",
    "/DHS_ITEM_STACK_DESTROY_INDEX=$ItemStackDestroy",
    "/DHS_SERVER_CREATE_BLOCK_DATA_INDEX=$ServerCreateBlockData",
    "/DHS_BLOCK_DATA_GET_BLOCK_STATES_INDEX=$BlockDataGetBlockStates",
    "/DHS_BLOCK_DATA_DESTROY_INDEX=$BlockDataDestroy",
    "/DHS_BOOL_VARIANT_INDEX_INDEX=$BoolVariantIndex",
    "/DHS_BOOL_VARIANT_AS_BOOL_INDEX=$BoolVariantAsBool",
    "/DHS_BOOL_VARIANT_DESTROY_INDEX=$BoolVariantDestroy",
    "/DHS_SERVER_COMMAND_SENDER_INDEX=$ServerCommandSender",
    "/DHS_PERMISSIBLE_PERMISSION_LEVEL_INDEX=$PermissiblePermissionLevel",
    "/DHS_COMMAND_SENDER_NAME_INDEX=$CommandSenderName",
    "/DHS_COMMAND_SENDER_WRAPPER_CREATE_INDEX=$WrapperCreate",
    "/DHS_COMMAND_SENDER_WRAPPER_PERMISSION_LEVEL_INDEX=$WrapperPermissionLevel",
    "/DHS_COMMAND_SENDER_WRAPPER_NAME_INDEX=$WrapperName",
    "/DHS_COMMAND_SENDER_WRAPPER_DESTROY_INDEX=$WrapperDestroy",
    "/DHS_COMMAND_SENDER_WRAPPER_AS_COMMAND_SENDER_INDEX=$WrapperAsCommandSender",
    "/DHS_COMMAND_SENDER_TRY_AS_WRAPPER_INDEX=$CommandSenderTryAsWrapper",
    "/DHS_COMMAND_SENDER_TRY_AS_ACTOR_INDEX=$CommandSenderTryAsActor",
    "/DHS_ACTION_FORM_CREATE_INDEX=$ActionFormCreate",
    "/DHS_ACTION_FORM_SET_CONTENT_INDEX=$ActionFormSetContent",
    "/DHS_ACTION_FORM_GET_CONTENT_INDEX=$ActionFormGetContent",
    "/DHS_ACTION_FORM_DESTROY_INDEX=$ActionFormDestroy",
    "/DHS_ACTION_FORM_SET_ON_SUBMIT_INDEX=$ActionFormSetOnSubmit",
    "/DHS_ACTION_FORM_GET_ON_SUBMIT_INDEX=$ActionFormGetOnSubmit",
    "/DHS_CALLABLE_INVOKE_INDEX=$CallableInvoke",
    "/DHS_CALLABLE_DESTROY_INDEX=$CallableDestroy",
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
