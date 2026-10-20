# Runtime smoke

Loads the committed BindingShim inside a real Endstone server through a small C++ plugin.

## Requirements

- Windows x64.
- Endstone 0.11.14 headers (a checkout or `git archive` of the tag).
- An Endstone server folder prepared by `python -m endstone` (the plugin folder is created on first run).
- The `endstone` Python package installed in the Python environment passed to the script.
- expected-lite and nlohmann_json headers available under the local xmake package cache.
- A local clang-cl toolchain.

## Run

```shell
powershell -File scripts/run-runtime-smoke.ps1 `
  -ServerRoot C:\path\to\endstone-server `
  -EndstoneRoot C:\path\to\endstone-v0.11.14 `
  -Python C:\path\to\venv\Scripts\python.exe
```

The script builds the solution, the native shim, and the C++ plugin; copies the plugin and shim into the server; starts Endstone with `--no-interactive`; and waits for `shim-smoke-result.txt`.

The plugin verifies:

- the shim DLL loads from the Endstone plugin cache;
- `hosihikari_endstone_get_server()` returns the same `Server&` as the plugin API;
- `Server::getName()` and `Server::getVersion()` through the BindingShim match the native calls;
- string and integer vector parameters reach `Translatable::Create` and `ArrayTag<int>::Create`;
- `PluginManager::getPlugins()` returns borrowed `Plugin*` addresses;
- `Server::createScoreboard()` returns a `shared_ptr` handle whose target serves `Scoreboard::clearSlot` and releases cleanly;
- `Scoreboard::getObjectives()` returns an owned `Objective[]` (empty on a fresh scoreboard);
- `PlayerBanList::getEntries()` returns `SharedHandle[]` and `getBanEntry("missing")` returns null;
- `addBan` with a 60-second duration, lookup, `isBanned`, and `removeBan` round-trip through `SharedHandle`;
- a string `std::variant` round-trips through `ActionForm::setContent`/`getContent`, including index and string accessors.

Expected result:

```text
load=ok
server.match=true
expected_name=Endstone
expected_version=0.11.14
name=Endstone
version=0.11.14
translatable.string_vector=ok
arraytag.int_vector=ok
plugin_manager.plugin_vector=ok
scoreboard.shared_handle=ok
scoreboard.owned_array=ok
ban_list.shared_array=ok
ban_list.add_remove=ok
action_form.string_variant=ok
result=OK
```

The smoke is not part of CI; it requires a local Endstone runtime.
