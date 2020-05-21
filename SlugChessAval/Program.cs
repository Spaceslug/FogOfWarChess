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
            BuildAvaloniaApp().StartWithClassicDesktopLifetime(args);
        } 

        // Avalonia configuration, don't remove; also used by visual designer.
        public static AppBuilder BuildAvaloniaApp()
            => AppBuilder.Configure<App>()
                .UsePlatformDetect()
                .LogToDebug()
                .UseReactiveUI();
    }
}
