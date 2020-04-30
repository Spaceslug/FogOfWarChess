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
using System.Reactive.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Input;

namespace SlugChessAval.ViewModels
{

    [DataContract]
    public class LoginViewModel : ViewModelBase, IRoutableViewModel
    {
        public string UrlPathSegment => "/login";
        public IScreen HostScreen { get; }

        public ICommand Login => _login;
        private readonly ReactiveCommand<Unit, bool> _login;

        private bool _loginInProgress = false;

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

        public LoginViewModel(IScreen? screen = null)
        {
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
            _login.Subscribe((sucsseded) => {
                if (sucsseded)
                {
                    HostScreen.Router.Navigate.Execute(new PlayViewModel()).Subscribe();
                }
            });
            
        }

        private void HandleLoginAttemptResult(ChessCom.LoginResult result)
        {
            if (result.SuccessfullLogin)
            {
                ((MainWindowViewModel)HostScreen).Notification = result.UserData.Username + " logged in :>";
                //_userdata = new ChessCom.UserData
                //{
                //    Username = result.UserData.Username,
                //    Usertoken = result.UserData.Usertoken,
                //    Elo = result.UserData.Elo
                //};
                //WriteTextNonInvoke("Logged in as " + username);
                //if (result.LoginMessage != "") WriteTextNonInvoke(result.LoginMessage);
                ////_userToken = result.UserToken;
                ////loginButton.Content = "U logged in";
                //loginButton.IsEnabled = false;
                //lookForMatchButton.IsEnabled = true;
                //foreach (var item in ((MenuItem)TopMenu.Items[0]).Items)
                //{
                //    if (item is MenuItem menuitem)
                //    {
                //        if ((string)menuitem.Header != "_Host" || (string)menuitem.Header != "_Browse Games") menuitem.IsEnabled = true;
                //    }
                //}
                //tbLoginStatus.Text = $"{_userdata.Username}";

                //_matchMessageStream = _connection.Call.ChatMessageStream();
                //_matchMessageStream.RequestStream.WriteAsync(new ChessCom.ChatMessage
                //{
                //    SenderUsertoken = _userdata.Usertoken,
                //    ReciverUsertoken = "system",
                //    SenderUsername = _userdata.Username,
                //    Message = "init"
                //});
                //Task.Run(() => MessageCallRunner());
                //_heartbeatTimer.Elapsed += (obj, e) =>
                //{
                //    _connection?.Call.Alive(new ChessCom.Heartbeat { Alive = true });
                //};
                //_heartbeatTimer.AutoReset = true;
                //_heartbeatTimer.Enabled = true;
                ////TODO recive message callback
                ////TODO handle shutdown of message
            }
            else
            {
                Console.WriteLine("Login failed. " + result.LoginMessage);
            }
            HostScreen.Router.NavigateBack.Execute().Subscribe();
            _loginInProgress = false;
        }
    }
}
