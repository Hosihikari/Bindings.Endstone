# Hosihikari.Bindings.Endstone

**Endstone** C++ API 的星光绑定表面：CI 生成的原生风格 **Interop** 与原生 **BindingShim**。

## 边界

- `Hosihikari.Bindings.Endstone.Interop` 由 `Hosihikari/Bindings.Generator` 从版本化 IR 生成，逐声明镜像 Endstone：一层指针与引用返回投影为类型化借用包装，`optional_ref<T>` 结果复用同一包装，其余不透明句柄保留 `nint`。ABI 值类型与 shim 加载器来自 `Hosihikari/Bindings.Abi`。
- `native/BindingShim/` 是 C ABI 投影。Endstone 插件通过 `endstone::detail::getServer()` 获取服务器；shim 使用同一引导方式并直接调用虚接口，因此不需要 Endstone 导入库。
- 手写 Idiomatic 层后置；当前公开面是生成的 Interop。

## 当前状态

- Interop：`src/Hosihikari.Bindings.Endstone.Interop/Generated/` 下 217 个生成 C# 文件。
- BindingShim：`native/BindingShim/generated/win-x64-server/endstone/` 中 2,056 个函数。
- 覆盖：319 条声明、1,831 条成员已绑定。

shim 以 `--virtual-dispatch` 模式发射：inline 成员、虚方法（含纯虚接口）、符合条件的模板实例、inline/constexpr 静态字段、`std::optional<T>` 参数/结果、`std::vector<T>` 参数/结果、`std::unique_ptr<T>` 结果、`std::shared_ptr<T>` 结果、`nonstd::expected_lite::expected<T, std::string>` 结果、`std::variant<T...>` 参数/结果、`NotNull`/`Nullable` 共享句柄、借用指针集合、拥有/共享元素数组、chrono 时间值、返回的 `std::function` 可调用句柄、映射为字典的 `unordered_map` 参数与结果、作为共享句柄的 `shared_ptr` 参数、作为所有权转移的 `unique_ptr` 参数、通过 holder 支撑的借用字符串标识符、作为 `Functions` 静态类的命名空间级 inline 函数以及经类型化控件 variant 的 `ModalForm` 控件被投影；非虚 out-of-line 成员保留 Interop stub，并在 `NotSupportedException` 中给出原因，同时标记 `[Obsolete]`，调用时产生 CS0618。宿主拥有的接口对象不释放；只有 shim 创建的对象生成释放器。

`NativeMethods.Load(path)` 加载 shim；`NativeMethods.GetServer()` 返回宿主的 `endstone::Server` 句柄，可用生成的 `Server` 类包装。

## 生成

前端解析 `include/endstone/endstone.hpp` 与 Endstone 依赖头：

```shell
powershell -File ../Generator/frontend/run-endstone.ps1
powershell -File scripts/regenerate.ps1 -IrDirectory ..\Generator\artifacts\endstone-ir -AbiRoot ..\Abi
```

## 构建

```shell
dotnet build Hosihikari.Bindings.Endstone.slnx -c Release
powershell -File native/BindingShim/build.ps1
```

原生构建使用 clang-cl、C++20、MSVC STL 与本地工具链已有的 expected-lite/nlohmann_json 头；不链接 `endstone_core`，调用通过宿主提供的虚接口分发。

## 生成源码门禁

`scripts/verify_generated.py` 校验 provenance、覆盖基线、原生表函数数、表版本与托管表。CI 构建解决方案、运行校验器，并用生成器验证提交的覆盖契约。

## 相关仓库

- `Hosihikari/Bindings.Ir`：schema、模型、读写器与校验器。
- `Hosihikari/Bindings.Abi`：共享 ABI 值类型、加载器与原生头文件。
- `Hosihikari/Bindings.Generator`：libTooling 前端与发射器。
- `Hosihikari/Bindings.LeviLamina`：LeviLamina 表面。

## 许可证

LGPL-3.0-or-later，见 `LICENSE.md`。
