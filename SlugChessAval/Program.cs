using System;
using System.Diagnostics;
using System.Collections.Generic;
using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Logging.Serilog;
using Avalonia.ReactiveUI;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using Serilog;
using Serilog.Filters;
using Avalonia.Logging;

namespace SlugChessAval
{

    public static class ShellHelper
    {
        public static void PlaySoundFile(string soundFile)
        {
            Task.Run(() => {
                if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
                {
                    $"aplay \"{soundFile}\"".Bash();
                }
                else if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                {
                    //string SoundLocation = "Assets\\sounds\\time_running_out.wav";
                    $"$PlayWav=New-Object System.Media.SoundPlayer\n$PlayWav.SoundLocation=\"{soundFile}\" \n$PlayWav.playsync()".PowerShell();
                }
                else
                {
                    Console.WriteLine("Not implemented on this platform");
                }
            });
            

        }
            
        public static string Bash(this string cmd)
        {
            var escapedArgs = cmd.Replace("\"", "\\\"");
                
            var process = new Process()
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = "/bin/bash",
                    Arguments = $"-c \"{escapedArgs}\"",
                    RedirectStandardOutput = true,
                    UseShellExecute = false,
                    CreateNoWindow = true,
                }
            };
            process.Start();
            string result = process.StandardOutput.ReadToEnd();
            process.WaitForExit();
            return result;
        }

        public static string PowerShell(this string cmd)
        {
            var escapedArgs = cmd.Replace("\"", "\\\"");
                
            var process = new Process()
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = "powershell.exe",
                    Arguments = $"-NoProfile -ExecutionPolicy unrestricted  {escapedArgs}",
                    RedirectStandardOutput = true,
                    UseShellExecute = false,
                    CreateNoWindow = true,
                }
            };
            process.Start();
            string result = process.StandardOutput.ReadToEnd();
            process.WaitForExit();
            return result;
        }
    }

    class Program
    {
        private static List<string> _launchParms = new List<string>();
        public static bool LaunchedWithParam(string s) => _launchParms.Contains(s);
        public static string GetParamValue(string s) => _launchParms[_launchParms.IndexOf(s)+1];


        // Initialization code. Don't use any Avalonia, third-party APIs or any
        // SynchronizationContext-reliant code before AppMain is called: things aren't initialized
        // yet and stuff might break.
        public static void Main(string[] args) {
            _launchParms = new List<string>(args);
            if (LaunchedWithParam("--help"))
            {
                PrintHelpTextToConsole();
            }
            Log.Information("WEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE");

            BuildAvaloniaApp().StartWithClassicDesktopLifetime(args);
        } 

        // Avalonia configuration, don't remove; also used by visual designer.
        public static AppBuilder BuildAvaloniaApp()
            => AppBuilder.Configure<App>()
                .UsePlatformDetect()
                .LogToDebug()
                .UseReactiveUI();


        public static void PrintHelpTextToConsole()
        {
            Console.WriteLine("SlugChess launch parameters:");
            Console.WriteLine("--help ;prints this text and prevents the launch of SlugChess ");
            Console.WriteLine("--debugLogin [username] ;automaticly logs you in as username with the debug password");
            Console.WriteLine("--port [port] ;sets the portnumber of the SlugChess server to connect to");
        }
    }
}
