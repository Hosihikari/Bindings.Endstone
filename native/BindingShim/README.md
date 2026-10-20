# Endstone BindingShim

The native side of the Endstone binding: it includes the Endstone public headers and projects eligible declarations into the C ABI described in ADR-0015.

## Target

- Windows x64 with clang-cl, C++20, the MSVC STL, and `/MD`.
- The Endstone public API is reached through host-provided virtual interfaces and the `endstone::detail::getServer()` bootstrap; the shim does not link `endstone_core`.
- The C ABI is a versioned function table returned by `hosihikari_get_binding_table_v1()`. Failures are reported through `hosihikari_get_last_error()`.
- `hosihikari_endstone_get_server()` exposes the host `Server&` as a `void*` handle for the managed bootstrap.

## Layout

```
generated/
  win-x64-server/endstone/binding_table.h    versioned table declaration
  win-x64-server/endstone/binding_table.cpp  extern "C" implementations
build.ps1                                    clang-cl build against the Endstone headers
```

Generated sources are produced by `Hosihikari/Bindings.Generator` with `--virtual-dispatch` and committed here; `scripts/regenerate.ps1` and `scripts/verify_generated.py` maintain the integration.

## Status

The committed table has 1,466 functions. Inline members, virtual interfaces, eligible template instances, and inline/constexpr static fields are projected; non-virtual out-of-line members and unsupported types remain Interop stubs with reasoned messages. The shim builds against the Endstone headers and is loaded by a CLR host in the server process.
