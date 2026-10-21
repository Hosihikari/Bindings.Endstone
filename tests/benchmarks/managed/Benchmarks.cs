using System.Diagnostics;
using System.Globalization;
using System.Text;
using Hosihikari.Bindings.Endstone.Interop;
using Hosihikari.Bindings.Endstone.Interop.endstone;
using Hosihikari.Bindings.Endstone.Interop.endstone.nbt;

namespace Hosihikari.Bindings.Endstone.Benchmarks;

internal static class Benchmarks
{
    private static int _intSink;
    private static ulong _sink;
    private static string? _stringSink;

    private static void RunBenchmark(
        StringBuilder output,
        string name,
        int iterations,
        int warmup,
        Action action)
    {
        for (var index = 0; index < warmup; index++)
        {
            action();
        }

        var allocatedBefore = GC.GetAllocatedBytesForCurrentThread();
        var start = Stopwatch.GetTimestamp();
        for (var index = 0; index < iterations; index++)
        {
            action();
        }

        var elapsed = Stopwatch.GetTimestamp() - start;
        var allocated = GC.GetAllocatedBytesForCurrentThread() - allocatedBefore;
        var nanoseconds = (double)elapsed / Stopwatch.Frequency * 1_000_000_000 / iterations;
        var bytes = (double)allocated / iterations;
        output.Append(name)
            .Append('=')
            .Append(nanoseconds.ToString("F2", CultureInfo.InvariantCulture))
            .Append(" ns/op, ")
            .Append(bytes.ToString("F1", CultureInfo.InvariantCulture))
            .AppendLine(" B/op");
    }

    public static int Run()
    {
        var resultPath = Environment.GetEnvironmentVariable("HS_BENCH_RESULT");
        var shimPath = Environment.GetEnvironmentVariable("HS_BENCH_SHIM");
        if (string.IsNullOrEmpty(resultPath) || string.IsNullOrEmpty(shimPath))
        {
            throw new InvalidOperationException("HS_BENCH_RESULT and HS_BENCH_SHIM must be set.");
        }

        Directory.CreateDirectory(Path.GetDirectoryName(resultPath)!);
        var log = new StringBuilder();

        try
        {
            NativeMethods.Load(shimPath);
            log.AppendLine("load=ok");

            var server = Server.Borrow(NativeMethods.GetServer());
            RunBenchmark(log, "method.server.get_port", 200_000, 10_000, () => _intSink = server.getPort());
            RunBenchmark(log, "string.server.get_name", 200_000, 10_000, () => _stringSink = server.getName());

            var translatableParameters = new[] { "one", "two" };
            RunBenchmark(log, "vector.string.translatable", 50_000, 5_000, () =>
            {
                using var value = Translatable.Create("hosihikari-benchmark", translatableParameters);
                _sink ^= (ulong)value.Handle;
            });

            var numbers = new[] { 1, 2, 3, 4 };
            RunBenchmark(log, "vector.int.arraytag", 100_000, 5_000, () =>
            {
                using var value = ArrayTag_int_.Create(numbers);
                _sink ^= (ulong)value.Handle;
            });

            var children = new Dictionary<string, bool> { ["child"] = true };
            RunBenchmark(log, "map.permission.create", 50_000, 5_000, () =>
            {
                using var value = Permission.Create(
                    "hosihikari.benchmark.permission",
                    "benchmark",
                    1,
                    children);
                _sink ^= (ulong)value.Handle;
            });

            using var permission = Permission.Create(
                "hosihikari.benchmark.children",
                "benchmark",
                1,
                children);
            RunBenchmark(log, "map.permission.children", 100_000, 5_000, () =>
            {
                var result = permission.getChildren();
                _sink ^= (ulong)result.Count;
            });

            using var blockState = Variant_7c287813.CreateBool(true);
            var blockStates = new Dictionary<string, Variant_7c287813>
            {
                ["persistent_bit"] = blockState,
            };
            RunBenchmark(log, "map.block_data.create", 20_000, 2_000, () =>
            {
                using var value = server.createBlockData("minecraft:oak_leaves", blockStates);
                _sink ^= (ulong)(value?.Handle ?? 0);
            });

            RunBenchmark(log, "shared.scoreboard", 200_000, 10_000, () =>
            {
                using var value = server.createScoreboard();
                _sink ^= (ulong)(value?.Target ?? 0);
            });

            var pluginManager = server.getPluginManager();
            RunBenchmark(log, "vector.result.plugins", 100_000, 5_000, () =>
            {
                var plugins = pluginManager.getPlugins();
                _sink ^= (ulong)plugins.Length;
            });

            log.AppendLine("result=OK");
        }
        catch (Exception exception)
        {
            log.AppendLine("result=FAIL");
            log.AppendLine(exception.ToString());
        }

        File.WriteAllText(resultPath, log.ToString());
        return 0;
    }
}
