﻿using Avalonia.Threading;
using ReactiveUI;
using SlugChessAval.Services;
using Splat;
using System;
using System.Collections.Generic;
using System.Reactive;
using System.Reactive.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Input;

namespace SlugChessAval.ViewModels
{
    public class StartMenuViewModel : ViewModelBase, IRoutableViewModel
    {
        public string UrlPathSegment => "/startmenu";
        public IScreen HostScreen { get; }

        public string Test
        {
            get => _test;
            set => this.RaiseAndSetIfChanged(ref _test, value);
        }
        private string _test = "...";

        public ICommand MoveToLogin => _moveToLogin;
        private readonly ReactiveCommand<Unit, Unit> _moveToLogin;

        public StartMenuViewModel(IScreen? screen = null)
        {
            HostScreen = screen ?? Locator.Current.GetService<IScreen>();

            SlugChessService.Client.WhenAnyValue(x => x.ConnectionAlive).Subscribe( x => Test = "ConnectionAlive is " + x.ToString());

            var canMoveToLogin = SlugChessService.Client.WhenAnyValue(x => x.ConnectionAlive);

            _moveToLogin = ReactiveCommand.Create(
                () => { HostScreen.Router.Navigate.Execute(new LoginViewModel()).Subscribe(); },
                canMoveToLogin);
#if DEBUG
            if (Program.LaunchedWithParam("-debugLogin"))
            {
                SlugChessService.Client.WhenAnyValue(x => x.ConnectionAlive).Delay(TimeSpan.FromSeconds(0.1)).Subscribe(x => 
                {
                    Dispatcher.UIThread.InvokeAsync(() => _moveToLogin.Execute().Subscribe());
                });
                
            }
#endif
        }
    }
}
