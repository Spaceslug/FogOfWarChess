using Avalonia.Threading;
using ReactiveUI;
using SlugChessAval.Services;
using Splat;
using System;
using System.Collections.Generic;
using System.Reactive;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Input;

namespace SlugChessAval.ViewModels
{
    #pragma warning disable 8612
    public class StartMenuViewModel : ViewModelBase, IRoutableViewModel, IActivatableViewModel
    #pragma warning restore 8612
    {
        public string UrlPathSegment => "/startmenu";
        public IScreen HostScreen { get; }

        public ViewModelActivator Activator { get; } = new ViewModelActivator();

        public string Test
        {
            get => _test;
            set => this.RaiseAndSetIfChanged(ref _test, value);
        }
        private string _test = "...";

        public ICommand MoveToLogin => _moveToLogin;
        private readonly ReactiveCommand<(string?, string?), (string?, string?)> _moveToLogin;
        public ICommand MoveToRegister => _moveToRegister;
        private readonly ReactiveCommand<Unit, Unit> _moveToRegister;

        public ICommand Exit => ((MainWindowViewModel)HostScreen).Exit;

        private bool _haveTriedDebugLogin = false;

        public StartMenuViewModel(IScreen? screen = null)
        {
            HostScreen = screen ?? Locator.Current.GetService<IScreen>();

            SlugChessService.Client.WhenAnyValue(x => x.ConnectionAlive).Subscribe( x => Test = "ConnectionAlive is " + x.ToString());

            var canMoveToLogin = SlugChessService.Client.WhenAnyValue(x => x.ConnectionAlive);

            _moveToLogin = ReactiveCommand.Create(
                ((string? a, string? b) c) => c,
                canMoveToLogin);
            _moveToLogin.Subscribe(((string? a, string? b) c) =>
            {
                HostScreen.Router.Navigate.Execute(new LoginViewModel(null, c.a, c.b)).Subscribe();
            });

            _moveToRegister = ReactiveCommand.Create(
                () => { HostScreen.Router.Navigate.Execute(new RegisterUserViewModel()).Subscribe(); },
                canMoveToLogin);

            this.WhenActivated(disposables =>
            {
#if DEBUG

                if (Program.LaunchedWithParam("--debugLogin") && !_haveTriedDebugLogin)
                {
                    _haveTriedDebugLogin = true;
                    SlugChessService.Client.WhenAnyValue(x => x.ConnectionAlive).Where(conAlive => conAlive).Delay(TimeSpan.FromSeconds(0.2)).Subscribe(x =>
                    {
                        Dispatcher.UIThread.InvokeAsync(() => MoveToLogin.Execute((Program.GetParamValue("--debugLogin"), "dbg_passW0rd")));
                    }).DisposeWith(disposables);

                }
#endif
                Disposable.Create(() =>
                {

                }).DisposeWith(disposables);
            });
        }
    }
}
