#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <variant>

#include <windows.h>

#include "endstone/endstone.hpp"
#include "binding_table.h"

#ifndef HS_NAME_INDEX
#error "HS_NAME_INDEX must be defined by build.ps1"
#endif
#ifndef HS_VERSION_INDEX
#error "HS_VERSION_INDEX must be defined by build.ps1"
#endif
#ifndef HS_TRANSLATABLE_CREATE_INDEX
#error "HS_TRANSLATABLE_CREATE_INDEX must be defined by build.ps1"
#endif
#ifndef HS_TRANSLATABLE_EMPTY_INDEX
#error "HS_TRANSLATABLE_EMPTY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_TRANSLATABLE_TEXT_INDEX
#error "HS_TRANSLATABLE_TEXT_INDEX must be defined by build.ps1"
#endif
#ifndef HS_TRANSLATABLE_DESTROY_INDEX
#error "HS_TRANSLATABLE_DESTROY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ARRAYTAG_INT_CREATE_INDEX
#error "HS_ARRAYTAG_INT_CREATE_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ARRAYTAG_INT_SIZE_INDEX
#error "HS_ARRAYTAG_INT_SIZE_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ARRAYTAG_INT_DESTROY_INDEX
#error "HS_ARRAYTAG_INT_DESTROY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_SERVER_PLUGIN_MANAGER_INDEX
#error "HS_SERVER_PLUGIN_MANAGER_INDEX must be defined by build.ps1"
#endif
#ifndef HS_PLUGIN_MANAGER_PLUGINS_INDEX
#error "HS_PLUGIN_MANAGER_PLUGINS_INDEX must be defined by build.ps1"
#endif
#ifndef HS_SERVER_CREATE_SCOREBOARD_INDEX
#error "HS_SERVER_CREATE_SCOREBOARD_INDEX must be defined by build.ps1"
#endif
#ifndef HS_SCOREBOARD_CLEAR_SLOT_INDEX
#error "HS_SCOREBOARD_CLEAR_SLOT_INDEX must be defined by build.ps1"
#endif
#ifndef HS_SCOREBOARD_OBJECTIVES_INDEX
#error "HS_SCOREBOARD_OBJECTIVES_INDEX must be defined by build.ps1"
#endif
#ifndef HS_SERVER_BAN_LIST_INDEX
#error "HS_SERVER_BAN_LIST_INDEX must be defined by build.ps1"
#endif
#ifndef HS_PLAYER_BAN_ENTRIES_INDEX
#error "HS_PLAYER_BAN_ENTRIES_INDEX must be defined by build.ps1"
#endif
#ifndef HS_PLAYER_BAN_ENTRY_INDEX
#error "HS_PLAYER_BAN_ENTRY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_PLAYER_BAN_ADD_INDEX
#error "HS_PLAYER_BAN_ADD_INDEX must be defined by build.ps1"
#endif
#ifndef HS_PLAYER_BAN_REMOVE_INDEX
#error "HS_PLAYER_BAN_REMOVE_INDEX must be defined by build.ps1"
#endif
#ifndef HS_PLAYER_BANNED_INDEX
#error "HS_PLAYER_BANNED_INDEX must be defined by build.ps1"
#endif
#ifndef HS_SERVER_ITEM_FACTORY_INDEX
#error "HS_SERVER_ITEM_FACTORY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ITEM_FACTORY_GET_META_INDEX
#error "HS_ITEM_FACTORY_GET_META_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ITEM_META_ADD_ENCHANT_INDEX
#error "HS_ITEM_META_ADD_ENCHANT_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ITEM_META_GET_ENCHANTS_INDEX
#error "HS_ITEM_META_GET_ENCHANTS_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ITEM_META_DESTROY_INDEX
#error "HS_ITEM_META_DESTROY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_MODAL_FORM_ADD_CONTROL_INDEX
#error "HS_MODAL_FORM_ADD_CONTROL_INDEX must be defined by build.ps1"
#endif
#ifndef HS_MODAL_FORM_GET_CONTROLS_INDEX
#error "HS_MODAL_FORM_GET_CONTROLS_INDEX must be defined by build.ps1"
#endif
#ifndef HS_DROPDOWN_DESTROY_INDEX
#error "HS_DROPDOWN_DESTROY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_CONTROL_VARIANT_CREATE_DROPDOWN_INDEX
#error "HS_CONTROL_VARIANT_CREATE_DROPDOWN_INDEX must be defined by build.ps1"
#endif
#ifndef HS_CONTROL_VARIANT_INDEX_INDEX
#error "HS_CONTROL_VARIANT_INDEX_INDEX must be defined by build.ps1"
#endif
#ifndef HS_CONTROL_VARIANT_AS_DROPDOWN_INDEX
#error "HS_CONTROL_VARIANT_AS_DROPDOWN_INDEX must be defined by build.ps1"
#endif
#ifndef HS_CONTROL_VARIANT_DESTROY_INDEX
#error "HS_CONTROL_VARIANT_DESTROY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_PERMISSION_CREATE_INDEX
#error "HS_PERMISSION_CREATE_INDEX must be defined by build.ps1"
#endif
#ifndef HS_PERMISSION_GET_CHILDREN_INDEX
#error "HS_PERMISSION_GET_CHILDREN_INDEX must be defined by build.ps1"
#endif
#ifndef HS_PERMISSION_DESTROY_INDEX
#error "HS_PERMISSION_DESTROY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_PERMISSION_NAME_INDEX
#error "HS_PERMISSION_NAME_INDEX must be defined by build.ps1"
#endif
#ifndef HS_PLUGIN_MANAGER_ADD_PERMISSION_INDEX
#error "HS_PLUGIN_MANAGER_ADD_PERMISSION_INDEX must be defined by build.ps1"
#endif
#ifndef HS_COMMAND_CREATE_INDEX
#error "HS_COMMAND_CREATE_INDEX must be defined by build.ps1"
#endif
#ifndef HS_COMMAND_GET_ALIASES_INDEX
#error "HS_COMMAND_GET_ALIASES_INDEX must be defined by build.ps1"
#endif
#ifndef HS_COMMAND_DESTROY_INDEX
#error "HS_COMMAND_DESTROY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ITEM_TYPE_ID_CREATE_INDEX
#error "HS_ITEM_TYPE_ID_CREATE_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ITEM_TYPE_ID_KEY_INDEX
#error "HS_ITEM_TYPE_ID_KEY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ITEM_TYPE_ID_DESTROY_INDEX
#error "HS_ITEM_TYPE_ID_DESTROY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ITEM_STACK_CREATE_INDEX
#error "HS_ITEM_STACK_CREATE_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ITEM_STACK_GET_AMOUNT_INDEX
#error "HS_ITEM_STACK_GET_AMOUNT_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ITEM_STACK_GET_DATA_INDEX
#error "HS_ITEM_STACK_GET_DATA_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ITEM_STACK_ASSIGN_INDEX
#error "HS_ITEM_STACK_ASSIGN_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ITEM_STACK_DESTROY_INDEX
#error "HS_ITEM_STACK_DESTROY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_SERVER_CREATE_BLOCK_DATA_INDEX
#error "HS_SERVER_CREATE_BLOCK_DATA_INDEX must be defined by build.ps1"
#endif
#ifndef HS_BLOCK_DATA_GET_BLOCK_STATES_INDEX
#error "HS_BLOCK_DATA_GET_BLOCK_STATES_INDEX must be defined by build.ps1"
#endif
#ifndef HS_BLOCK_DATA_DESTROY_INDEX
#error "HS_BLOCK_DATA_DESTROY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_BOOL_VARIANT_INDEX_INDEX
#error "HS_BOOL_VARIANT_INDEX_INDEX must be defined by build.ps1"
#endif
#ifndef HS_BOOL_VARIANT_AS_BOOL_INDEX
#error "HS_BOOL_VARIANT_AS_BOOL_INDEX must be defined by build.ps1"
#endif
#ifndef HS_BOOL_VARIANT_DESTROY_INDEX
#error "HS_BOOL_VARIANT_DESTROY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_SERVER_COMMAND_SENDER_INDEX
#error "HS_SERVER_COMMAND_SENDER_INDEX must be defined by build.ps1"
#endif
#ifndef HS_PERMISSIBLE_PERMISSION_LEVEL_INDEX
#error "HS_PERMISSIBLE_PERMISSION_LEVEL_INDEX must be defined by build.ps1"
#endif
#ifndef HS_COMMAND_SENDER_NAME_INDEX
#error "HS_COMMAND_SENDER_NAME_INDEX must be defined by build.ps1"
#endif
#ifndef HS_COMMAND_SENDER_WRAPPER_CREATE_INDEX
#error "HS_COMMAND_SENDER_WRAPPER_CREATE_INDEX must be defined by build.ps1"
#endif
#ifndef HS_COMMAND_SENDER_WRAPPER_PERMISSION_LEVEL_INDEX
#error "HS_COMMAND_SENDER_WRAPPER_PERMISSION_LEVEL_INDEX must be defined by build.ps1"
#endif
#ifndef HS_COMMAND_SENDER_WRAPPER_NAME_INDEX
#error "HS_COMMAND_SENDER_WRAPPER_NAME_INDEX must be defined by build.ps1"
#endif
#ifndef HS_COMMAND_SENDER_WRAPPER_DESTROY_INDEX
#error "HS_COMMAND_SENDER_WRAPPER_DESTROY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ACTION_FORM_CREATE_INDEX
#error "HS_ACTION_FORM_CREATE_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ACTION_FORM_SET_CONTENT_INDEX
#error "HS_ACTION_FORM_SET_CONTENT_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ACTION_FORM_GET_CONTENT_INDEX
#error "HS_ACTION_FORM_GET_CONTENT_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ACTION_FORM_DESTROY_INDEX
#error "HS_ACTION_FORM_DESTROY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ACTION_FORM_SET_ON_SUBMIT_INDEX
#error "HS_ACTION_FORM_SET_ON_SUBMIT_INDEX must be defined by build.ps1"
#endif
#ifndef HS_ACTION_FORM_GET_ON_SUBMIT_INDEX
#error "HS_ACTION_FORM_GET_ON_SUBMIT_INDEX must be defined by build.ps1"
#endif
#ifndef HS_CALLABLE_INVOKE_INDEX
#error "HS_CALLABLE_INVOKE_INDEX must be defined by build.ps1"
#endif
#ifndef HS_CALLABLE_DESTROY_INDEX
#error "HS_CALLABLE_DESTROY_INDEX must be defined by build.ps1"
#endif
#ifndef HS_VARIANT_CREATE_STRING_INDEX
#error "HS_VARIANT_CREATE_STRING_INDEX must be defined by build.ps1"
#endif
#ifndef HS_VARIANT_INDEX_INDEX
#error "HS_VARIANT_INDEX_INDEX must be defined by build.ps1"
#endif
#ifndef HS_VARIANT_GET_STRING_INDEX
#error "HS_VARIANT_GET_STRING_INDEX must be defined by build.ps1"
#endif
#ifndef HS_VARIANT_DESTROY_INDEX
#error "HS_VARIANT_DESTROY_INDEX must be defined by build.ps1"
#endif

namespace {
void pluginAnchor() {}

int g_submitCalls = 0;
int g_submitReleases = 0;

void smokeSubmit(void *, void *, int) { ++g_submitCalls; }
void smokeSubmitRelease(void *) { ++g_submitReleases; }
void smokeNoopMessage(void *, void *) {}
}  // namespace

class ShimSmokePlugin : public endstone::Plugin {
public:
    void onEnable() override
    {
        std::ofstream out("shim-smoke-result.txt");
        auto &server = getServer();

        try {
            HMODULE self = nullptr;
            GetModuleHandleExA(
                GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                reinterpret_cast<LPCSTR>(&pluginAnchor),
                &self
            );
            char path[MAX_PATH]{};
            GetModuleFileNameA(self, path, MAX_PATH);
            std::string directory(path);
            const auto separator = directory.find_last_of("\\/");
            if (separator != std::string::npos) {
                directory.resize(separator + 1);
            }

            std::string shimPath = directory + "Hosihikari.Bindings.Endstone.Shim.dll";
            auto shim = LoadLibraryA(shimPath.c_str());
            if (!shim) {
                shimPath = directory + "..\\Hosihikari.Bindings.Endstone.Shim.dll";
                shim = LoadLibraryA(shimPath.c_str());
            }
            if (!shim) {
                WIN32_FIND_DATAA data{};
                const auto search = FindFirstFileA(
                    (directory + "Hosihikari.Bindings.Endstone.Shim-*.dll").c_str(),
                    &data
                );
                if (search != INVALID_HANDLE_VALUE) {
                    shimPath = directory + data.cFileName;
                    shim = LoadLibraryA(shimPath.c_str());
                    FindClose(search);
                }
            }
            out << "path=" << shimPath << "\n";
            out << "load=" << (shim ? "ok" : "failed") << "\n";
            if (!shim) {
                out << "winerror=" << GetLastError() << "\n";
            }
            if (!shim) {
                out << "result=FAIL\n";
                server.shutdown();
                return;
            }

            auto getTable = reinterpret_cast<const HosihikariBindingTableV1 *(*)()>(
                GetProcAddress(shim, "hosihikari_get_binding_table_v1")
            );
            auto getServer = reinterpret_cast<void *(*)()>(
                GetProcAddress(shim, "hosihikari_endstone_get_server")
            );
            if (!getTable || !getServer) {
                out << "exports=failed\n";
                out << "result=FAIL\n";
                FreeLibrary(shim);
                server.shutdown();
                return;
            }

            const auto *table = getTable();
            auto *handle = getServer();
            out << "server.match=" << (handle == &server ? "true" : "false") << "\n";

            const auto *slots = reinterpret_cast<void *const *>(
                reinterpret_cast<const char *>(table) + 2 * sizeof(std::uint32_t)
            );
            const auto nameFunction = reinterpret_cast<HosihikariStringView (*)(void *)>(slots[HS_NAME_INDEX]);
            const auto nameView = nameFunction(handle);
            const std::string actualName(nameView.data, nameView.size);
            const auto versionFunction = reinterpret_cast<HosihikariStringView (*)(void *)>(slots[HS_VERSION_INDEX]);
            const auto versionView = versionFunction(handle);
            const std::string actualVersion(versionView.data, versionView.size);

            out << "expected_name=" << server.getName() << "\n";
            out << "expected_version=" << server.getVersion() << "\n";
            out << "name=" << actualName << "\n";
            out << "version=" << actualVersion << "\n";
            bool vectorsOk = true;

            using CreateTranslatableFn = void *(*)(const char *, std::size_t, const HosihikariStringView *, std::size_t);
            using CreateArrayTagFn = void *(*)(const int *, std::size_t);
            using BoolMethodFn = bool (*)(void *);
            using TextMethodFn = HosihikariStringView (*)(void *);
            using SizeMethodFn = std::size_t (*)(void *);
            using DestroyFn = void (*)(void *);

            const char *paramText[] = {"alpha", "", "gamma"};
            HosihikariStringView paramViews[3]{};
            for (std::size_t index = 0; index < 3; ++index) {
                paramViews[index] = {paramText[index], std::strlen(paramText[index])};
            }

            auto translatable = reinterpret_cast<CreateTranslatableFn>(slots[HS_TRANSLATABLE_CREATE_INDEX])(
                "vector", 6, paramViews, 3);
            if (translatable) {
                const auto isEmpty = reinterpret_cast<BoolMethodFn>(slots[HS_TRANSLATABLE_EMPTY_INDEX])(translatable);
                const auto textView = reinterpret_cast<TextMethodFn>(slots[HS_TRANSLATABLE_TEXT_INDEX])(translatable);
                const bool ok = !isEmpty && std::string(textView.data, textView.size) == "vector";
                out << "translatable.string_vector=" << (ok ? "ok" : "FAIL") << "\n";
                vectorsOk = vectorsOk && ok;
                reinterpret_cast<DestroyFn>(slots[HS_TRANSLATABLE_DESTROY_INDEX])(translatable);
            }
            else {
                out << "translatable.string_vector=failed\n";
                vectorsOk = false;
            }

            const int values[] = {4, 5, 6};
            auto arrayTag = reinterpret_cast<CreateArrayTagFn>(slots[HS_ARRAYTAG_INT_CREATE_INDEX])(values, 3);
            if (arrayTag) {
                const bool ok = reinterpret_cast<SizeMethodFn>(slots[HS_ARRAYTAG_INT_SIZE_INDEX])(arrayTag) == 3;
                out << "arraytag.int_vector=" << (ok ? "ok" : "FAIL") << "\n";
                vectorsOk = vectorsOk && ok;
                reinterpret_cast<DestroyFn>(slots[HS_ARRAYTAG_INT_DESTROY_INDEX])(arrayTag);
            }
            else {
                out << "arraytag.int_vector=failed\n";
                vectorsOk = false;
            }

            using GetPluginManagerFn = void *(*)(void *);
            using GetPluginsFn = HosihikariArrayView (*)(void *);
            auto pluginManager = reinterpret_cast<GetPluginManagerFn>(slots[HS_SERVER_PLUGIN_MANAGER_INDEX])(handle);
            bool pluginsOk = false;
            if (pluginManager) {
                const auto plugins = reinterpret_cast<GetPluginsFn>(slots[HS_PLUGIN_MANAGER_PLUGINS_INDEX])(pluginManager);
                const auto items = static_cast<void *const *>(const_cast<void *>(plugins.data));
                pluginsOk = plugins.size >= 1;
                for (std::size_t index = 0; index < plugins.size; ++index) {
                    pluginsOk = pluginsOk && items[index] != nullptr;
                }
            }

            out << "plugin_manager.plugin_vector=" << (pluginsOk ? "ok" : "FAIL") << "\n";

            using CreateScoreboardFn = HosihikariSharedHandle (*)(void *);
            using ClearSlotFn = void (*)(void *, std::int32_t);
            using GetObjectivesFn = HosihikariArrayView (*)(void *);
            auto shared = reinterpret_cast<CreateScoreboardFn>(slots[HS_SERVER_CREATE_SCOREBOARD_INDEX])(handle);
            bool sharedOk = shared.holder != nullptr && shared.target != nullptr && shared.release != nullptr;
            bool objectivesOk = false;
            if (sharedOk) {
                reinterpret_cast<ClearSlotFn>(slots[HS_SCOREBOARD_CLEAR_SLOT_INDEX])(shared.target, 0);
                const auto objectives = reinterpret_cast<GetObjectivesFn>(slots[HS_SCOREBOARD_OBJECTIVES_INDEX])(shared.target);
                objectivesOk = objectives.size == 0;
                shared.release(shared.holder);
            }

            out << "scoreboard.shared_handle=" << (sharedOk ? "ok" : "FAIL") << "\n";
            out << "scoreboard.owned_array=" << (objectivesOk ? "ok" : "FAIL") << "\n";

            using GetBanListFn = void *(*)(void *);
            using GetBanEntriesFn = HosihikariArrayView (*)(void *);
            using GetBanEntryFn = HosihikariSharedHandle (*)(void *, const char *, std::size_t);
            using AddBanFn = HosihikariSharedHandle (*)(
                void *, const char *, std::size_t, const char *, std::size_t, std::int64_t, const char *, std::size_t, bool, bool);
            using RemoveBanFn = void (*)(void *, const char *, std::size_t);
            using IsBannedFn = bool (*)(void *, const char *, std::size_t);
            bool containersOk = false;
            bool banRoundTripOk = false;
            if (auto *banList = reinterpret_cast<GetBanListFn>(slots[HS_SERVER_BAN_LIST_INDEX])(handle)) {
                const auto entries = reinterpret_cast<GetBanEntriesFn>(slots[HS_PLAYER_BAN_ENTRIES_INDEX])(banList);
                containersOk = true;
                const auto sharedItems = static_cast<HosihikariSharedHandle const *>(entries.data);
                for (std::size_t index = 0; index < entries.size; ++index) {
                    if (sharedItems[index].release == nullptr) {
                        containersOk = false;
                        continue;
                    }

                    sharedItems[index].release(sharedItems[index].holder);
                }

                auto missing = reinterpret_cast<GetBanEntryFn>(slots[HS_PLAYER_BAN_ENTRY_INDEX])(
                    banList, "hosihikari-missing", 18);
                containersOk = containersOk && missing.holder == nullptr && missing.release == nullptr;
                if (missing.release) {
                    missing.release(missing.holder);
                }

                const char *banName = "hosihikari-smoke-ban";
                const std::size_t banNameSize = std::strlen(banName);
                const auto removeBan = reinterpret_cast<RemoveBanFn>(slots[HS_PLAYER_BAN_REMOVE_INDEX]);
                const auto isBanned = reinterpret_cast<IsBannedFn>(slots[HS_PLAYER_BANNED_INDEX]);
                removeBan(banList, banName, banNameSize);
                auto added = reinterpret_cast<AddBanFn>(slots[HS_PLAYER_BAN_ADD_INDEX])(
                    banList,
                    banName,
                    banNameSize,
                    "smoke",
                    5,
                    60,
                    "hosihikari",
                    10,
                    true,
                    true);
                banRoundTripOk = added.holder != nullptr && added.target != nullptr;
                if (added.release) {
                    added.release(added.holder);
                }

                auto fetched = reinterpret_cast<GetBanEntryFn>(slots[HS_PLAYER_BAN_ENTRY_INDEX])(
                    banList, banName, banNameSize);
                banRoundTripOk = banRoundTripOk && fetched.holder != nullptr;
                if (fetched.release) {
                    fetched.release(fetched.holder);
                }

                banRoundTripOk = banRoundTripOk && isBanned(banList, banName, banNameSize);
                removeBan(banList, banName, banNameSize);
                banRoundTripOk = banRoundTripOk && !isBanned(banList, banName, banNameSize);
            }

            out << "ban_list.shared_array=" << (containersOk ? "ok" : "FAIL") << "\n";
            out << "ban_list.add_remove=" << (banRoundTripOk ? "ok" : "FAIL") << "\n";

            using GetItemFactoryFn = void *(*)(void *);
            using GetItemMetaFn = void *(*)(void *, void *);
            using AddEnchantFn = bool (*)(void *, void *, std::int32_t, bool);
            using GetEnchantsFn = HosihikariArrayView (*)(void *);
            bool itemMetaMapOk = false;
            if (auto *factory = reinterpret_cast<GetItemFactoryFn>(slots[HS_SERVER_ITEM_FACTORY_INDEX])(handle)) {
                endstone::ItemTypeId itemType("minecraft:diamond_sword");
                auto *meta = reinterpret_cast<GetItemMetaFn>(slots[HS_ITEM_FACTORY_GET_META_INDEX])(factory, &itemType);
                if (meta) {
                    endstone::EnchantmentId enchantment("minecraft:sharpness");
                    const bool added = reinterpret_cast<AddEnchantFn>(slots[HS_ITEM_META_ADD_ENCHANT_INDEX])(
                        meta, &enchantment, 5, true);
                    const auto enchants = reinterpret_cast<GetEnchantsFn>(slots[HS_ITEM_META_GET_ENCHANTS_INDEX])(meta);
                    const auto *entries = static_cast<HosihikariMapEntry const *>(enchants.data);
                    itemMetaMapOk = added && enchants.size == 1 && entries[0].key != 0
                        && static_cast<int>(entries[0].value) == 5;
                    reinterpret_cast<DestroyFn>(slots[HS_ITEM_META_DESTROY_INDEX])(meta);
                }
            }

            out << "item_meta.map=" << (itemMetaMapOk ? "ok" : "FAIL") << "\n";

            using ModalAddControlFn = void *(*)(void *, void *);
            using ModalGetControlsFn = HosihikariArrayView (*)(void *);
            using VariantCreateDropdownFn = void *(*)(void *);
            using VariantReadIndexFn = std::int32_t (*)(void *);
            using VariantAsDropdownFn = void *(*)(void *);
            bool modalControlOk = false;
            {
                endstone::ModalForm modal;
                endstone::Dropdown dropdown;
                auto *control = reinterpret_cast<VariantCreateDropdownFn>(
                    slots[HS_CONTROL_VARIANT_CREATE_DROPDOWN_INDEX])(&dropdown);
                if (control) {
                    auto *added = reinterpret_cast<ModalAddControlFn>(slots[HS_MODAL_FORM_ADD_CONTROL_INDEX])(
                        &modal, control);
                    const auto controls = reinterpret_cast<ModalGetControlsFn>(
                        slots[HS_MODAL_FORM_GET_CONTROLS_INDEX])(&modal);
                    auto *const *items = static_cast<void *const *>(controls.data);
                    modalControlOk = added != nullptr && controls.size == 1 && items[0] != nullptr
                        && reinterpret_cast<VariantReadIndexFn>(
                            slots[HS_CONTROL_VARIANT_INDEX_INDEX])(items[0]) == 0;
                    if (controls.size == 1 && items[0] != nullptr) {
                        auto *copy = reinterpret_cast<VariantAsDropdownFn>(
                            slots[HS_CONTROL_VARIANT_AS_DROPDOWN_INDEX])(items[0]);
                        modalControlOk = modalControlOk && copy != nullptr;
                        if (copy) {
                            reinterpret_cast<DestroyFn>(slots[HS_DROPDOWN_DESTROY_INDEX])(copy);
                        }
                    }
                    for (std::size_t index = 0; index < controls.size; ++index) {
                        reinterpret_cast<DestroyFn>(slots[HS_CONTROL_VARIANT_DESTROY_INDEX])(items[index]);
                    }
                    reinterpret_cast<DestroyFn>(slots[HS_CONTROL_VARIANT_DESTROY_INDEX])(control);
                }
            }

            out << "modal_form.controls=" << (modalControlOk ? "ok" : "FAIL") << "\n";

            using PermissionCreateFn = void *(*)(const char *, std::size_t, const char *, std::size_t, std::int32_t, HosihikariArrayView);
            using GetChildrenFn = HosihikariArrayView (*)(void *);
            bool permissionMapOk = false;
            {
                const std::string permissionName = "hosihikari.smoke.permission";
                const std::string description = "smoke";
                const char *childName = "child";
                HosihikariMapEntry children[1]{};
                children[0].key = reinterpret_cast<std::uint64_t>(childName);
                children[0].keySize = std::strlen(childName);
                children[0].value = 1;
                auto *permission = reinterpret_cast<PermissionCreateFn>(slots[HS_PERMISSION_CREATE_INDEX])(
                    permissionName.data(), permissionName.size(), description.data(), description.size(), 1,
                    HosihikariArrayView{children, 1});
                if (permission) {
                    const auto result = reinterpret_cast<GetChildrenFn>(
                        slots[HS_PERMISSION_GET_CHILDREN_INDEX])(permission);
                    const auto *entries = static_cast<HosihikariMapEntry const *>(result.data);
                    permissionMapOk = result.size == 1 && entries[0].keySize == std::strlen(childName)
                        && std::string(reinterpret_cast<const char *>(entries[0].key), entries[0].keySize) == childName
                        && entries[0].value != 0;
                    reinterpret_cast<DestroyFn>(slots[HS_PERMISSION_DESTROY_INDEX])(permission);
                }
            }

            out << "permission.map=" << (permissionMapOk ? "ok" : "FAIL") << "\n";

            using AddPermissionFn = void *(*)(void *, void *);
            using PermissionNameFn = HosihikariStringView (*)(void *);
            bool uniquePtrParamOk = false;
            if (pluginManager) {
                const std::string addedName = "hosihikari.smoke.unique_ptr";
                auto *addedPermission = reinterpret_cast<PermissionCreateFn>(slots[HS_PERMISSION_CREATE_INDEX])(
                    addedName.data(), addedName.size(), "", 0, 1, HosihikariArrayView{nullptr, 0});
                if (addedPermission) {
                    auto *registered = reinterpret_cast<AddPermissionFn>(
                        slots[HS_PLUGIN_MANAGER_ADD_PERMISSION_INDEX])(pluginManager, addedPermission);
                    if (registered) {
                        const auto view = reinterpret_cast<PermissionNameFn>(slots[HS_PERMISSION_NAME_INDEX])(registered);
                        uniquePtrParamOk = std::string(view.data, view.size) == addedName;
                    }
                }
            }

            out << "unique_ptr.param=" << (uniquePtrParamOk ? "ok" : "FAIL") << "\n";

            using CommandCreateFn = void *(*)(
                const char *, std::size_t, const char *, std::size_t,
                const HosihikariStringView *, std::size_t,
                const HosihikariStringView *, std::size_t,
                const HosihikariStringView *, std::size_t);
            using CommandAliasesFn = HosihikariArrayView (*)(void *);
            bool multiVectorOk = false;
            {
                const std::string commandName = "hosihikari-smoke";
                const std::string commandDescription = "multi vector";
                const char *usageText[] = {"/hosihikari-smoke"};
                const char *aliasText[] = {"smoke-a", "smoke-b"};
                const char *permissionText[] = {"hosihikari.smoke"};
                HosihikariStringView usages[1] = {{usageText[0], std::strlen(usageText[0])}};
                HosihikariStringView aliases[2] = {
                    {aliasText[0], std::strlen(aliasText[0])},
                    {aliasText[1], std::strlen(aliasText[1])},
                };
                HosihikariStringView permissions[1] = {{permissionText[0], std::strlen(permissionText[0])}};
                auto *command = reinterpret_cast<CommandCreateFn>(slots[HS_COMMAND_CREATE_INDEX])(
                    commandName.data(), commandName.size(), commandDescription.data(), commandDescription.size(),
                    usages, 1, aliases, 2, permissions, 1);
                if (command) {
                    const auto result = reinterpret_cast<CommandAliasesFn>(
                        slots[HS_COMMAND_GET_ALIASES_INDEX])(command);
                    const auto *items = static_cast<HosihikariStringView const *>(result.data);
                    multiVectorOk = result.size == 2
                        && std::string(items[0].data, items[0].size) == "smoke-a"
                        && std::string(items[1].data, items[1].size) == "smoke-b";
                    reinterpret_cast<DestroyFn>(slots[HS_COMMAND_DESTROY_INDEX])(command);
                }
            }

            out << "multi_vector.ctor=" << (multiVectorOk ? "ok" : "FAIL") << "\n";

            using IdentifierCreateFn = void *(*)(const char *, std::size_t);
            using IdentifierStringFn = HosihikariStringView (*)(void *);
            bool borrowedStringOk = false;
            {
                const std::string itemTypeName = "minecraft:diamond_sword";
                auto *itemType = reinterpret_cast<IdentifierCreateFn>(slots[HS_ITEM_TYPE_ID_CREATE_INDEX])(
                    itemTypeName.data(), itemTypeName.size());
                if (itemType) {
                    const auto key = reinterpret_cast<IdentifierStringFn>(
                        slots[HS_ITEM_TYPE_ID_KEY_INDEX])(itemType);
                    borrowedStringOk = std::string(key.data, key.size) == "diamond_sword";
                    if (auto *factory = reinterpret_cast<GetItemFactoryFn>(
                            slots[HS_SERVER_ITEM_FACTORY_INDEX])(handle)) {
                        auto *meta = reinterpret_cast<GetItemMetaFn>(
                            slots[HS_ITEM_FACTORY_GET_META_INDEX])(factory, itemType);
                        borrowedStringOk = borrowedStringOk && meta != nullptr;
                        if (meta) {
                            reinterpret_cast<DestroyFn>(slots[HS_ITEM_META_DESTROY_INDEX])(meta);
                        }
                    }
                    reinterpret_cast<DestroyFn>(slots[HS_ITEM_TYPE_ID_DESTROY_INDEX])(itemType);
                }
            }

            out << "borrowed_string.ctor=" << (borrowedStringOk ? "ok" : "FAIL") << "\n";

            using ItemStackCreateFn = void *(*)(void *, int, int);
            using ItemStackIntFn = int (*)(void *);
            using ItemStackAssignFn = void *(*)(void *, void *);
            bool itemStackAssignOk = false;
            {
                const std::string itemTypeName = "minecraft:diamond_sword";
                auto *itemType = reinterpret_cast<IdentifierCreateFn>(slots[HS_ITEM_TYPE_ID_CREATE_INDEX])(
                    itemTypeName.data(), itemTypeName.size());
                if (itemType) {
                    auto *target = reinterpret_cast<ItemStackCreateFn>(
                        slots[HS_ITEM_STACK_CREATE_INDEX])(itemType, 1, 0);
                    auto *source = reinterpret_cast<ItemStackCreateFn>(
                        slots[HS_ITEM_STACK_CREATE_INDEX])(itemType, 7, 3);
                    if (target && source) {
                        reinterpret_cast<ItemStackAssignFn>(slots[HS_ITEM_STACK_ASSIGN_INDEX])(target, source);
                        itemStackAssignOk = reinterpret_cast<ItemStackIntFn>(
                                                slots[HS_ITEM_STACK_GET_AMOUNT_INDEX])(target)
                                == 7
                            && reinterpret_cast<ItemStackIntFn>(
                                   slots[HS_ITEM_STACK_GET_DATA_INDEX])(target)
                                == 3;
                    }
                    if (source) {
                        reinterpret_cast<DestroyFn>(slots[HS_ITEM_STACK_DESTROY_INDEX])(source);
                    }
                    if (target) {
                        reinterpret_cast<DestroyFn>(slots[HS_ITEM_STACK_DESTROY_INDEX])(target);
                    }
                    reinterpret_cast<DestroyFn>(slots[HS_ITEM_TYPE_ID_DESTROY_INDEX])(itemType);
                }
            }

            out << "assignment.operator=" << (itemStackAssignOk ? "ok" : "FAIL") << "\n";

            using CreateBlockDataFn = void *(*)(void *, const char *, std::size_t, HosihikariArrayView);
            using GetBlockStatesFn = HosihikariArrayView (*)(void *);
            using VariantAsBoolFn = bool (*)(void *);
            bool blockStateMapOk = false;
            {
                const std::string blockType = "minecraft:oak_leaves";
                const std::string stateKey = "persistent_bit";
                std::variant<bool, std::string, int> stateValue = true;
                HosihikariMapEntry states[1]{};
                states[0].key = reinterpret_cast<std::uint64_t>(stateKey.data());
                states[0].keySize = stateKey.size();
                states[0].value = reinterpret_cast<std::uint64_t>(&stateValue);
                auto *blockData = reinterpret_cast<CreateBlockDataFn>(slots[HS_SERVER_CREATE_BLOCK_DATA_INDEX])(
                    handle, blockType.data(), blockType.size(), HosihikariArrayView{states, 1});
                if (blockData) {
                    const auto result = reinterpret_cast<GetBlockStatesFn>(
                        slots[HS_BLOCK_DATA_GET_BLOCK_STATES_INDEX])(blockData);
                    const auto *entries = static_cast<HosihikariMapEntry const *>(result.data);
                    for (std::size_t index = 0; index < result.size; ++index) {
                        auto *state = reinterpret_cast<void *>(entries[index].value);
                        if (entries[index].keySize == stateKey.size()
                            && std::string(reinterpret_cast<const char *>(entries[index].key), entries[index].keySize) == stateKey) {
                            blockStateMapOk = state != nullptr
                                && reinterpret_cast<VariantReadIndexFn>(
                                    slots[HS_BOOL_VARIANT_INDEX_INDEX])(state) == 0
                                && reinterpret_cast<VariantAsBoolFn>(
                                    slots[HS_BOOL_VARIANT_AS_BOOL_INDEX])(state);
                        }
                        if (state) {
                            reinterpret_cast<DestroyFn>(slots[HS_BOOL_VARIANT_DESTROY_INDEX])(state);
                        }
                    }
                    reinterpret_cast<DestroyFn>(slots[HS_BLOCK_DATA_DESTROY_INDEX])(blockData);
                }
            }

            out << "block_data.map=" << (blockStateMapOk ? "ok" : "FAIL") << "\n";

            using GetCommandSenderFn = void *(*)(void *);
            using WrapperCreateFn = void *(*)(void *, void *, void *, void *, void *, void *, void *);
            using IntMethodFn = std::int32_t (*)(void *);
            using GetNameFn = HosihikariStringView (*)(void *);
            bool mutableReferenceCtorOk = false;
            if (auto *sender = reinterpret_cast<GetCommandSenderFn>(slots[HS_SERVER_COMMAND_SENDER_INDEX])(handle)) {
                auto *wrapper = reinterpret_cast<WrapperCreateFn>(slots[HS_COMMAND_SENDER_WRAPPER_CREATE_INDEX])(
                    sender,
                    reinterpret_cast<void *>(&smokeNoopMessage),
                    nullptr,
                    nullptr,
                    reinterpret_cast<void *>(&smokeNoopMessage),
                    nullptr,
                    nullptr);
                if (wrapper) {
                    const auto senderLevel = reinterpret_cast<IntMethodFn>(
                        slots[HS_PERMISSIBLE_PERMISSION_LEVEL_INDEX])(sender);
                    const auto wrapperLevel = reinterpret_cast<IntMethodFn>(
                        slots[HS_COMMAND_SENDER_WRAPPER_PERMISSION_LEVEL_INDEX])(wrapper);
                    const auto senderName = reinterpret_cast<GetNameFn>(slots[HS_COMMAND_SENDER_NAME_INDEX])(sender);
                    const auto wrapperName = reinterpret_cast<GetNameFn>(
                        slots[HS_COMMAND_SENDER_WRAPPER_NAME_INDEX])(wrapper);
                    mutableReferenceCtorOk = senderLevel == wrapperLevel
                        && std::string(senderName.data, senderName.size)
                            == std::string(wrapperName.data, wrapperName.size);
                    reinterpret_cast<DestroyFn>(slots[HS_COMMAND_SENDER_WRAPPER_DESTROY_INDEX])(wrapper);
                }
            }

            out << "mutable_reference.ctor=" << (mutableReferenceCtorOk ? "ok" : "FAIL") << "\n";

            using CreateActionFormFn = void *(*)();
            using SetContentFn = void *(*)(void *, void *);
            using GetContentFn = void *(*)(void *);
            using VariantCreateStringFn = void *(*)(const char *, std::size_t);
            using VariantIndexFn = std::int32_t (*)(void *);
            using VariantGetStringFn = HosihikariStringView (*)(void *);
            using DestroyFn = void (*)(void *);

            auto *actionForm = reinterpret_cast<CreateActionFormFn>(slots[HS_ACTION_FORM_CREATE_INDEX])();
            auto *content = reinterpret_cast<VariantCreateStringFn>(slots[HS_VARIANT_CREATE_STRING_INDEX])("hello", 5);
            bool variantOk = actionForm != nullptr && content != nullptr;
            if (variantOk) {
                reinterpret_cast<SetContentFn>(slots[HS_ACTION_FORM_SET_CONTENT_INDEX])(actionForm, content);
                auto *returned = reinterpret_cast<GetContentFn>(slots[HS_ACTION_FORM_GET_CONTENT_INDEX])(actionForm);
                variantOk = returned != nullptr
                    && reinterpret_cast<VariantIndexFn>(slots[HS_VARIANT_INDEX_INDEX])(returned) == 0;
                if (variantOk) {
                    const auto text = reinterpret_cast<VariantGetStringFn>(slots[HS_VARIANT_GET_STRING_INDEX])(returned);
                    variantOk = std::string(text.data, text.size) == "hello";
                }

                if (returned) {
                    reinterpret_cast<DestroyFn>(slots[HS_VARIANT_DESTROY_INDEX])(returned);
                }
            }

            using SetOnSubmitFn = void *(*)(void *, void *, void *, void *);
            using GetOnSubmitFn = void *(*)(void *);
            using InvokeSubmitFn = void (*)(void *, void *, std::int32_t);
            bool callableOk = false;
            if (actionForm) {
                reinterpret_cast<SetOnSubmitFn>(slots[HS_ACTION_FORM_SET_ON_SUBMIT_INDEX])(
                    actionForm,
                    reinterpret_cast<void *>(&smokeSubmit),
                    nullptr,
                    reinterpret_cast<void *>(&smokeSubmitRelease));
                auto *submit = reinterpret_cast<GetOnSubmitFn>(slots[HS_ACTION_FORM_GET_ON_SUBMIT_INDEX])(actionForm);
                if (submit) {
                    reinterpret_cast<InvokeSubmitFn>(slots[HS_CALLABLE_INVOKE_INDEX])(submit, nullptr, 0);
                    callableOk = g_submitCalls == 1;
                    reinterpret_cast<DestroyFn>(slots[HS_CALLABLE_DESTROY_INDEX])(submit);
                }
            }

            if (content) {
                reinterpret_cast<DestroyFn>(slots[HS_VARIANT_DESTROY_INDEX])(content);
            }

            if (actionForm) {
                reinterpret_cast<DestroyFn>(slots[HS_ACTION_FORM_DESTROY_INDEX])(actionForm);
            }

            callableOk = callableOk && g_submitReleases == 1;
            out << "action_form.string_variant=" << (variantOk ? "ok" : "FAIL") << "\n";
            out << "callable.round_trip=" << (callableOk ? "ok" : "FAIL") << "\n";

            out << "result="
                << (actualName == server.getName() && actualVersion == server.getVersion() && vectorsOk
                            && pluginsOk && sharedOk && objectivesOk && containersOk && banRoundTripOk
                            && variantOk && callableOk
                        ? "OK"
                        : "FAIL")
                << "\n";

            FreeLibrary(shim);
        }
        catch (std::exception const &exception) {
            out << "exception=" << exception.what() << "\n";
            out << "result=FAIL\n";
        }

        out.close();
        server.shutdown();
    }
};

ENDSTONE_PLUGIN("shim_smoke", "1.0.0", ShimSmokePlugin)
{
    description = "Hosihikari BindingShim smoke test";
}
