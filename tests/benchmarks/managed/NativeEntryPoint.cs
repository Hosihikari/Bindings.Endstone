using System.Runtime.InteropServices;

namespace Hosihikari.Bindings.Endstone.Benchmarks;

/// <summary>Entry point resolved through <c>hostfxr</c> by the native benchmark plugin.</summary>
public static class NativeEntryPoint
{
    [UnmanagedCallersOnly]
    public static int Run(IntPtr argument, int argumentLength)
    {
        try
        {
            return Benchmarks.Run();
        }
        catch (Exception exception)
        {
            var resultPath = Environment.GetEnvironmentVariable("HS_BENCH_RESULT");
            if (!string.IsNullOrEmpty(resultPath))
            {
                try
                {
                    File.WriteAllText(resultPath, "load=failed\nresult=FAIL\n" + exception + Environment.NewLine);
                }
                catch (IOException)
                {
                }
            }

            return 1;
        }
    }
}
