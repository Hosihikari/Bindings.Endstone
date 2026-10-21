# Shim benchmarks

Loads the BindingShim inside a real Endstone server and reports the managed-to-native cost of representative calls. A small native Endstone plugin hosts the CLR through `nethost`/`hostfxr`, loads the managed benchmark assembly, and invokes its `[UnmanagedCallersOnly]` entry point. The managed side calls the generated Interop through the same BindingShim table used by plugins.

## Run

```shell
powershell -File scripts/run-benchmarks.ps1 `
  -ServerRoot C:\path\to\endstone-server `
  -EndstoneRoot C:\path\to\endstone-v0.11.14 `
  -Python C:\path\to\venv\Scripts\python.exe
```

The script builds the solution, the Windows shim, the managed benchmark assembly, and the native host plugin. It installs the plugin under `plugins/`, copies the managed payload and `nethost.dll` to `plugins/hosihikari-benchmarks/`, starts Endstone with `--no-interactive`, and waits for `data/hosihikari/shim-benchmarks/benchmarks.txt`.

Requirements: Windows x64, Endstone 0.11.14 headers, an Endstone server prepared by `python -m endstone`, the `endstone` Python package in that environment, expected-lite/nlohmann_json under the local xmake cache, clang-cl, and the .NET 10 host pack.

## Measurements

Each line reports nanoseconds per call and managed bytes allocated per call after warmup:

- `method.server.get_port`: scalar field through the table.
- `string.server.get_name`: string return, thread-local shim buffer, and managed copy.
- `vector.string.translatable`: `string[]` parameter encoding into the UTF-8 arena and an owned result.
- `vector.int.arraytag`: `int[]` parameter pinning and an owned result.
- `map.permission.create`: `Dictionary<string, bool>` parameter encoding and an owned result.
- `map.permission.children`: map result decoding into a managed dictionary.
- `map.block_data.create`: `Dictionary<string, Variant>` parameter encoding and an owned result.
- `shared.scoreboard`: `shared_ptr` result through a pooled `SharedHandle`.
- `vector.result.plugins`: borrowed pointer-vector result.

Expected result:

```text
load=ok
method.server.get_port=...
string.server.get_name=...
vector.string.translatable=...
vector.int.arraytag=...
map.permission.create=...
map.permission.children=...
map.block_data.create=...
shared.scoreboard=...
vector.result.plugins=...
result=OK
```

The numbers are environment-dependent and are not a CI gate. They complement the LeviLamina benchmark set: the Endstone surface exercises map parameters, variant-valued maps, shared handles, and record factories that the LLAPI slice does not.
