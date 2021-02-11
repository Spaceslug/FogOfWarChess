using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using ReactiveUI;
using SlugChessAval.Models;
using SlugChessAval.Services;
using Splat;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reactive;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Input;

namespace SlugChessAval.ViewModels
{

    [DataContract]
    public class LoginViewModel : ViewModelBase, IRoutableViewModel, IActivatableViewModel
    {
        public ViewModelActivator Activator { get; }
        public string UrlPathSegment => "/login";
        public IScreen HostScreen { get; }

        public ICommand Login => _login;
        private readonly ReactiveCommand<Unit, ChessCom.LoginResult> _login;

        public string LoginMessage
        {
            get => _loginMessage;
            set => this.RaiseAndSetIfChanged(ref _loginMessage, value);
        }
        private string _loginMessage = "";

        [DataMember]
        public string Username
        {
            get => _username;
            set => this.RaiseAndSetIfChanged(ref _username, value);
        }
        private string _username = "";

        // Note: Saving passwords to disk isn't a good idea. 
        public string Password
        {
            get => _password;
            set => this.RaiseAndSetIfChanged(ref _password, value);
        }
        private string _password = "";

        public LoginViewModel(IScreen? screen = null, string? username = null, string? password = null)
        {
            Activator = new ViewModelActivator();
            HostScreen = screen ?? Locator.Current.GetService<IScreen>();
            
            // When any of the specified properties change, 
            // we check if user input is valid.
            var canLogin = this
                .WhenAnyValue(
                    x => x.Username,
                    x => x.Password,
                    (user, pass) => !string.IsNullOrWhiteSpace(user) &&
                                    !string.IsNullOrWhiteSpace(pass));

            // Buttons bound to the command will stay disabled
            // as long as the command stays disabled.
            _login = ReactiveCommand.CreateFromTask(
                () => SlugChessService.Client.LoginInUserAsync(Username, Password), 
                canLogin);
            _login.Subscribe((result) => {
                HandleLoginAttemptResult(result);
            });

            if(username != null && password != null)
            {
                Username = username;
                Password = password;
                _login.Execute().Subscribe();
            }
            this.WhenActivated(disposables =>
            {

                Disposable.Create(() =>
                {

                }).DisposeWith(disposables);
            });
        }

        private void HandleLoginAttemptResult(ChessCom.LoginResult result)
        {
            if (result.SuccessfullLogin)
            {
                MainWindowViewModel.SendNotification(result.UserData.Username + " logged in :>");

                //SlugChessService.Client.MessageToLocal("Logged in as " + result.UserData.Username, "system");
                Serilog.Log.Information("Logged in as " + result.UserData.Username);

                HostScreen.Router.Navigate.Execute(new PlayViewModel()).Subscribe();
                ////TODO recive message callback
                ////TODO handle shutdown of message

                if (result.LoginMessage != "")
                {
                    Serilog.Log.Information("Login Message: " + result.LoginMessage);
                    SlugChessService.Client.MessageToLocal("Login Message: " + result.LoginMessage, "system");
                }
            }
            else
            {
                // The knower of problem sends the notification
                //((MainWindowViewModel)HostScreen).Notification = "Login failed: " + result.LoginMessage;
            }
            LoginMessage = result.LoginMessage;



            //_loginInProgress = false;
        }
    }
}
