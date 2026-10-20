#include <cstring>
#include <fstream>
#include <string>

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

namespace {
void pluginAnchor() {}
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
            auto shared = reinterpret_cast<CreateScoreboardFn>(slots[HS_SERVER_CREATE_SCOREBOARD_INDEX])(handle);
            bool sharedOk = shared.holder != nullptr && shared.target != nullptr && shared.release != nullptr;
            if (sharedOk) {
                reinterpret_cast<ClearSlotFn>(slots[HS_SCOREBOARD_CLEAR_SLOT_INDEX])(shared.target, 0);
                shared.release(shared.holder);
            }

            out << "scoreboard.shared_handle=" << (sharedOk ? "ok" : "FAIL") << "\n";

            out << "result="
                << (actualName == server.getName() && actualVersion == server.getVersion() && vectorsOk
                            && pluginsOk && sharedOk
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
