using System;
using System.Collections.Generic;
using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Logging.Serilog;
using Avalonia.ReactiveUI;

namespace SlugChessAval
{


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
