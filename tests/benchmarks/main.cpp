#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include <windows.h>

#include "endstone/endstone.hpp"

#include "coreclr_delegates.h"
#include "hostfxr.h"
#include "nethost.h"

namespace {

using run_managed_fn = component_entry_point_fn;

std::filesystem::path module_directory() {
    HMODULE module = nullptr;
    GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&module_directory),
        &module);
    wchar_t path[MAX_PATH]{};
    GetModuleFileNameW(module, path, MAX_PATH);
    return std::filesystem::path(path).parent_path();
}

void write_error(const std::filesystem::path& path, const std::string& message) {
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    std::ofstream output(path, std::ios::binary);
    output << "load=failed\n";
    output << "error=" << message << "\n";
    output << "result=FAIL\n";
}

bool host_and_run(const std::filesystem::path& moduleDirectory, std::string& error) {
    auto payload = moduleDirectory / "hosihikari-benchmarks";
    if (!std::filesystem::exists(payload)) {
        payload = moduleDirectory.parent_path() / "hosihikari-benchmarks";
    }
    const auto managedDll = payload / "Hosihikari.Bindings.Endstone.Benchmarks.dll";
    const auto runtimeConfig = payload / "Hosihikari.Bindings.Endstone.Benchmarks.runtimeconfig.json";
    const auto nethostPath = payload / "nethost.dll";

    if (!std::filesystem::exists(managedDll) || !std::filesystem::exists(runtimeConfig)) {
        error = "managed benchmark payload was not found under " + payload.string();
        return false;
    }
    if (!std::filesystem::exists(nethostPath)) {
        error = "nethost.dll was not found under " + payload.string();
        return false;
    }

    HMODULE nethost = LoadLibraryW(nethostPath.c_str());
    if (nethost == nullptr) {
        error = "failed to load nethost.dll";
        return false;
    }

    auto getHostfxrPath = reinterpret_cast<decltype(&get_hostfxr_path)>(GetProcAddress(nethost, "get_hostfxr_path"));
    if (getHostfxrPath == nullptr) {
        error = "get_hostfxr_path was not exported by nethost.dll";
        return false;
    }

    wchar_t hostfxrPath[4096]{};
    size_t hostfxrPathSize = std::size(hostfxrPath);
    get_hostfxr_parameters parameters{};
    parameters.size = sizeof(parameters);
    parameters.assembly_path = managedDll.c_str();
    if (getHostfxrPath(hostfxrPath, &hostfxrPathSize, &parameters) != 0) {
        error = "get_hostfxr_path failed";
        return false;
    }

    HMODULE hostfxr = LoadLibraryW(hostfxrPath);
    if (hostfxr == nullptr) {
        error = "failed to load hostfxr.dll";
        return false;
    }

    auto initialize = reinterpret_cast<hostfxr_initialize_for_runtime_config_fn>(
        GetProcAddress(hostfxr, "hostfxr_initialize_for_runtime_config"));
    auto getRuntimeDelegate = reinterpret_cast<hostfxr_get_runtime_delegate_fn>(
        GetProcAddress(hostfxr, "hostfxr_get_runtime_delegate"));
    auto close = reinterpret_cast<hostfxr_close_fn>(GetProcAddress(hostfxr, "hostfxr_close"));
    if (initialize == nullptr || getRuntimeDelegate == nullptr || close == nullptr) {
        error = "hostfxr entry points were not found";
        return false;
    }

    hostfxr_handle context = nullptr;
    if (initialize(runtimeConfig.c_str(), nullptr, &context) != 0 || context == nullptr) {
        error = "hostfxr_initialize_for_runtime_config failed";
        return false;
    }

    load_assembly_and_get_function_pointer_fn loadAssembly = nullptr;
    if (getRuntimeDelegate(context, hdt_load_assembly_and_get_function_pointer, reinterpret_cast<void**>(&loadAssembly))
            != 0
        || loadAssembly == nullptr) {
        close(context);
        error = "hostfxr_get_runtime_delegate failed";
        return false;
    }

    run_managed_fn run = nullptr;
    const char_t* typeName =
        L"Hosihikari.Bindings.Endstone.Benchmarks.NativeEntryPoint, Hosihikari.Bindings.Endstone.Benchmarks";
    if (loadAssembly(managedDll.c_str(), typeName, L"Run", UNMANAGEDCALLERSONLY_METHOD, nullptr,
                     reinterpret_cast<void**>(&run))
            != 0
        || run == nullptr) {
        close(context);
        error = "load_assembly_and_get_function_pointer failed";
        return false;
    }

    const int result = run(nullptr, 0);
    close(context);
    if (result != 0) {
        error = "managed benchmark entry point returned a failure";
        return false;
    }
    return true;
}

class ShimBenchmarkPlugin : public endstone::Plugin {
public:
    void onEnable() override {
        const auto moduleDirectory = module_directory();

        wchar_t resultPathBuffer[4096]{};
        const DWORD resultPathLength =
            GetEnvironmentVariableW(L"HS_BENCH_RESULT", resultPathBuffer, static_cast<DWORD>(std::size(resultPathBuffer)));
        const std::filesystem::path resultPath = resultPathLength > 0
            ? std::filesystem::path(resultPathBuffer)
            : moduleDirectory / ".." / "data" / "hosihikari" / "shim-benchmarks" / "benchmarks.txt";

        wchar_t shimPathBuffer[4096]{};
        const DWORD shimPathLength =
            GetEnvironmentVariableW(L"HS_BENCH_SHIM", shimPathBuffer, static_cast<DWORD>(std::size(shimPathBuffer)));
        if (shimPathLength == 0) {
            const auto shimPath = moduleDirectory / "Hosihikari.Bindings.Endstone.Shim.dll";
            SetEnvironmentVariableW(L"HS_BENCH_SHIM", shimPath.c_str());
        }

        std::string error;
        if (!host_and_run(moduleDirectory, error)) {
            write_error(resultPath, error);
        }

        getServer().shutdown();
    }
};

} // namespace

ENDSTONE_PLUGIN("shim_benchmarks", "1.0.0", ShimBenchmarkPlugin)
{
    description = "Hosihikari BindingShim benchmarks";
}
