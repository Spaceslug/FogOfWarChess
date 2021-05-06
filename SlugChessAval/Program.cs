using System;
using System.Diagnostics;
using System.Collections.Generic;
using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.ReactiveUI;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using Avalonia.Logging;
using System.Reflection;
using System.IO;
using System.Threading;

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

        //public static string RootDir = (Path.GetDirectoryName(Process.GetCurrentProcess().MainModule.FileName) ?? ".") + "/";
        public static string RootDir = OtherRootDir;
        public static string OtherRootDir
        {
            get
            {
                var name = "/" + System.Reflection.Assembly.GetEntryAssembly()?.GetName().Name+".dll" ?? "";
                var fullpath = System.Reflection.Assembly.GetEntryAssembly()?.Location ?? "";
                return fullpath.Replace(name, "") + "/";
            }
        }

        public static string? VersionCheckMessage = null;


        // Initialization code. Don't use any Avalonia, third-party APIs or any
        // SynchronizationContext-reliant code before AppMain is called: things aren't initialized
        // yet and stuff might break.
        public static void Main(string[] args) {
            _launchParms = new List<string>(args);
            if (LaunchedWithParam("--help"))
            {
                PrintHelpTextToConsole();
                return;
            }
            if (LaunchedWithParam("--version"))
            {
                Console.WriteLine(GetSlugChessVersion());
                return;
            }
            if (File.Exists($"{RootDir}{UpdaterFilename()}.new"))
            {
                Thread.Sleep(100);
                File.Delete($"{RootDir}{UpdaterFilename()}");
                File.Move($"{RootDir}{UpdaterFilename()}.new", $"{RootDir}{UpdaterFilename()}");
            }
            if (!LaunchedWithParam("--no-updatecheck"))
            {
                var result = GetUpdateNeededFromUpdater();
                switch (result)
                {
                    case "0":
                        Console.WriteLine("Version is up to date");
                        break;
                    case "1":
                        Console.WriteLine("Version out of date. Starting updater and exiting");
                        StartUpdaterUpdate();
                        return;
                    case "2":
                        Console.WriteLine("New Version availeble and you should update");
                        break;
                    case "3":
                        VersionCheckMessage = "New Version availeble. You must manualy download new version. Copy over appstate.json to preserve settings";
                        Console.WriteLine(VersionCheckMessage);
                        break;
                    default:
                        Console.WriteLine(result);
                        break;
                }
            }
            Console.WriteLine("WEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE");

            BuildAvaloniaApp().StartWithClassicDesktopLifetime(args, Avalonia.Controls.ShutdownMode.OnLastWindowClose);
        } 

        // Avalonia configuration, don't remove; also used by visual designer.
        public static AppBuilder BuildAvaloniaApp()
            => AppBuilder.Configure<App>()
                .UsePlatformDetect()
                .LogToTrace()
                .UseReactiveUI();


        public static void PrintHelpTextToConsole()
        {
            Console.WriteLine("SlugChess launch parameters:");
            Console.WriteLine("--help                  ;prints this text and prevents the launch of ");
            Console.WriteLine("   SlugChess ");
            Console.WriteLine("--version               ;prints the version of SlugChess             ");
            Console.WriteLine("--debugLogin [username] ;automaticly logs you in as username with the");
            Console.WriteLine("   debug password");
            Console.WriteLine("--port [port]           ;sets the portnumber of the SlugChess server ");
            Console.WriteLine("   to connect to");
            Console.WriteLine("--no-updatecheck        ;prevent update check from running");
        }

        public static string UpdaterFilename()
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                return "SlugChessUpdater";
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                return "SlugChessUpdater.exe";
            }
            else
            {
                throw new PlatformNotSupportedException("fuck this. I'm out");
            }
        }

        public static string GetSlugChessVersion()
        {
            var ver = FileVersionInfo.GetVersionInfo(Assembly.GetExecutingAssembly().Location);
            return $"{ver.FileMajorPart}.{ver.FileMinorPart}.{ver.FileBuildPart}";
        }

        public static string GetUpdateNeededFromUpdater()
        {
            
            var proc = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = UpdaterFilename(),
                    Arguments = $"--check-version {GetSlugChessVersion()}" ,
                    UseShellExecute = false,
                    RedirectStandardOutput = true,
                    CreateNoWindow = true
                }
            };
            string line = "";
            proc.Start();
            while (!proc.StandardOutput.EndOfStream)
            {
                line += proc.StandardOutput.ReadLine();
                // do something with line
            }
            proc.Close();
            return line;
        }

        public static void StartUpdaterUpdate()
        {
            var proc = new Process
            {
                StartInfo = new ProcessStartInfo
                {
                    FileName = $"{RootDir}{UpdaterFilename()}",
                    Arguments = $"--run-update",
                    UseShellExecute = true,
                    RedirectStandardOutput = false,
                    CreateNoWindow = false
                }
            };
            proc.Start();
        }

        public static string GetSafeFilename(string filename)
        {

            return string.Join("_", filename.Split(Path.GetInvalidFileNameChars()));

        }
    }
}
