using Avalonia;
using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using ReactiveUI.Samples.Suspension.ViewModels;
using ReactiveUI.Samples.Suspension.Views;
using ReactiveUI.Samples.Suspension.Drivers;
using Splat;

namespace ReactiveUI.Samples.Suspension
{
    public class App : Application
    {
        public override void Initialize()
        {
            AvaloniaXamlLoader.Load(this);
        }
        
        public override void OnFrameworkInitializationCompleted()
        {
            // Initialize suspension hooks.
            var suspension = new AutoSuspendHelper(ApplicationLifetime);
            RxApp.SuspensionHost.CreateNewAppState = () => new MainWindowViewModel();
            RxApp.SuspensionHost.SetupDefaultSuspendResume(new NewtonsoftJsonSuspensionDriver("appstate.json"));
            suspension.OnFrameworkInitializationCompleted();

            // Read main view model state from disk.
            var state = RxApp.SuspensionHost.GetAppState<MainWindowViewModel>();
            Locator.CurrentMutable.RegisterConstant<IScreen>(state);

            // Register views.
            Locator.CurrentMutable.Register<IViewFor<SearchViewModel>>(() => new SearchView());
            Locator.CurrentMutable.Register<IViewFor<LoginViewModel>>(() => new LoginView());

            // Show the main window.
            new MainWindowView { DataContext = Locator.Current.GetService<IScreen>() }.Show();
            base.OnFrameworkInitializationCompleted();
        }
   }
}