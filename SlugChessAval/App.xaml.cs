using Avalonia;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Markup.Xaml;
using SlugChessAval.ViewModels;
using SlugChessAval.Views;
using SlugChessAval.Services;
using ReactiveUI;
using Splat;
using Avalonia.ReactiveUI;
using SlugChessAval.Drivers;
using System.IO;
using System.Reflection;

namespace SlugChessAval
{
    public class App : Application
    {
        public override void Initialize()
        {
            AvaloniaXamlLoader.Load(this);
        }

        public override void OnFrameworkInitializationCompleted()
        {
            AssetBank.LoadAssets();

            // Initialize suspension hooks.
            var suspension = new AutoSuspendHelper(ApplicationLifetime);
            RxApp.SuspensionHost.CreateNewAppState = () => new MainWindowViewModel();
            //RxApp.SuspensionHost.SetupDefaultSuspendResume(new NewtonsoftJsonSuspensionDriver("appstate.json"));
            RxApp.SuspensionHost.SetupDefaultSuspendResume(new NewtonsoftJsonSuspensionDriver(Program.RootDir + "/appstate.json"));
            suspension.OnFrameworkInitializationCompleted();

            // Read main view model state from disk.
            var state = RxApp.SuspensionHost.GetAppState<MainWindowViewModel>();
            Locator.CurrentMutable.RegisterConstant<IScreen>(state);
            // Register views.
            Locator.CurrentMutable.Register<IViewFor<ChessboardViewModel>>(() => new Chessboard());
            Locator.CurrentMutable.Register<IViewFor<PlayViewModel>>(() => new PlayView());
            Locator.CurrentMutable.Register<IViewFor<LoginViewModel>>(() => new LoginView());
            Locator.CurrentMutable.Register<IViewFor<StartMenuViewModel>>(() => new StartMenuView());
            Locator.CurrentMutable.Register<IViewFor<CreateGameViewModel>>(() => new CreateGameView());
            Locator.CurrentMutable.Register<IViewFor<GameBrowserViewModel>>(() => new GameBrowserView());
            Locator.CurrentMutable.Register<IViewFor<ChessClockViewModel>>(() => new ChessClock());
            Locator.CurrentMutable.Register<IViewFor<CapturedPicesViewModel>>(() => new CapturedPices());
            Locator.CurrentMutable.Register<IViewFor<ChatboxViewModel>>(() => new Chatbox());


            // Show the main window.
            new MainWindow { DataContext = Locator.Current.GetService<IScreen>() }.Show();
            base.OnFrameworkInitializationCompleted();
        }
    }
}
