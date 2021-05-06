using ReactiveUI;
using System;
using System.Collections.Generic;
using System.Text;
using System.Reactive;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using System.Runtime.Serialization;
using System.Windows.Input;
using System.Threading.Tasks;
using Splat;
using SlugChessAval.Services;
using Avalonia.Threading;

namespace SlugChessAval.ViewModels
{
    [DataContract]
    #pragma warning disable 8612
    public class RegisterUserViewModel : ViewModelBase, IRoutableViewModel, IActivatableViewModel
    #pragma warning restore 8612
    {
        public ViewModelActivator Activator { get; }
        public string UrlPathSegment => "/registerUser";
        public IScreen HostScreen { get; }

        public ICommand RegisterUser => _registerUser;
        private readonly ReactiveCommand<(string, string), (ChessCom.RegiserUserFormResult, string, string)> _registerUser;

        public string RegisterMessage
        {
            get => _registerMessage;
            set => this.RaiseAndSetIfChanged(ref _registerMessage, value);
        }
        private string _registerMessage = "";

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

        public string ConfPassword
        {
            get => _confPassword;
            set => this.RaiseAndSetIfChanged(ref _confPassword, value);
        }
        private string _confPassword = "";

        public RegisterUserViewModel(IScreen? screen = null)
        {
            Activator = new ViewModelActivator();
            HostScreen = screen ?? Locator.Current.GetService<IScreen>();

            // When any of the specified properties change, 
            // we check if user input is valid.
            var canRegister = this
                .WhenAnyValue(
                    x => x.Username,
                    x => x.Password,
                    x => x.ConfPassword,
                    (user, pass, confPass) => !string.IsNullOrWhiteSpace(user) && !user.Contains('/') && Encoding.UTF8.GetByteCount(user) < 81 &&
                                    !string.IsNullOrWhiteSpace(pass) && Encoding.UTF8.GetByteCount(pass) < 81 && 
                                    pass == confPass);

            // Buttons bound to the command will stay disabled
            // as long as the command stays disabled.
            _registerUser = ReactiveCommand.CreateFromTask(
                ((string u, string p) x) => Task.Run<(ChessCom.RegiserUserFormResult, string, string)>(
                    () => {
                        return (SlugChessService.Client.Call.RegisterUser(new ChessCom.RegiserUserForm { Username = Username, Password = Password }), x.u, x.p);
                    }),
                canRegister);
            _registerUser.Subscribe(((ChessCom.RegiserUserFormResult result, string u, string p) x) => {
                if (x.result.Success)
                {
                    if(!((HostScreen.Router.CurrentViewModel.Take(1).Wait()) is StartMenuViewModel))
                    {
                        HostScreen.Router.NavigateBack.Execute().Subscribe();
                    }
                    StartMenuViewModel? smvm = HostScreen.Router.CurrentViewModel.Take(1).Wait() as StartMenuViewModel;
                    smvm?.MoveToLogin.Execute((x.u, x.p));
                }
                else
                {
                    RegisterMessage = x.result.FailMessage;
                }
            });


            this.WhenActivated(disposables =>
            {

                Disposable.Create(() =>
                {

                }).DisposeWith(disposables);
            });

        }
    }
}
