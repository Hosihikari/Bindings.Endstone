#include <fstream>
#include <string>

#include <windows.h>

#include "endstone/endstone.hpp"
#include "binding_table.h"

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

            const auto nameView = table->f1093(handle);
            const std::string actualName(nameView.data, nameView.size);
            const auto versionView = table->f1105(handle);
            const std::string actualVersion(versionView.data, versionView.size);

            out << "expected_name=" << server.getName() << "\n";
            out << "expected_version=" << server.getVersion() << "\n";
            out << "name=" << actualName << "\n";
            out << "version=" << actualVersion << "\n";
            out << "result="
                << (actualName == server.getName() && actualVersion == server.getVersion() ? "OK" : "FAIL")
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
