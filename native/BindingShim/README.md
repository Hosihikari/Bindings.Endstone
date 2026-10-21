# Endstone BindingShim

The native side of the Endstone binding: it includes the Endstone public headers and projects eligible declarations into the C ABI described in ADR-0015.

## Target

- Windows x64 with clang-cl, C++20, the MSVC STL, and `/MD`; Linux x64 with g++/clang++ and C++20.
- The Endstone public API is reached through host-provided virtual interfaces and the `endstone::detail::getServer()` bootstrap; the shim does not link `endstone_core`.
- The C ABI is a versioned function table returned by `hosihikari_get_binding_table_v1()`. Failures are reported through `hosihikari_get_last_error()`.
- `hosihikari_endstone_get_server()` exposes the host `Server&` as a `void*` handle for the managed bootstrap.

## Layout

```
generated/
  win-x64-server/endstone/binding_table.h    versioned table declaration
  win-x64-server/endstone/binding_table.cpp  extern "C" implementations
build.ps1                                    clang-cl build against the Endstone headers
build.sh                                     g++/clang++ build for Linux
```

Generated sources are produced by `Hosihikari/Bindings.Generator` with `--virtual-dispatch` and committed here; `scripts/regenerate.ps1` and `scripts/verify_generated.py` maintain the integration.

## Linux build

`build.sh` compiles the committed table against an Endstone checkout and the expected-lite/nlohmann_json headers. It generates `artifacts/endstone-generated/include/endstone/version.h` from the Endstone `include/CMakeLists.txt`, links a position-independent shared library, and checks that both entry points are exported.

```shell
./native/BindingShim/build.sh \
  --endstone-root /path/to/endstone \
  --expected-lite-include /path/to/expected-lite/include \
  --nlohmann-include /path/to/json/include
```

The CI `linux-shim` job checks out Endstone `v0.11.14` and pinned expected-lite/nlohmann_json tags, then runs this script. The Windows and Linux shims expose the same versioned table.

## Status

The committed table has 2,040 functions. Inline members, virtual interfaces, eligible template instances, inline/constexpr static fields, assignment operators on non-template classes, base/derived cast helpers, director factories for eligible callback interfaces, optional parameters/results, vector parameters/results, shared-pointer results, expected results, variant values, and unordered_map parameters/results are projected; non-virtual out-of-line members and unsupported types remain Interop stubs with reasoned messages. The shim builds against the Endstone headers and is loaded by a CLR host in the server process.
