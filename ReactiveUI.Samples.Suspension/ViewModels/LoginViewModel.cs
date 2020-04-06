using System.Reactive;
using System.Runtime.Serialization;
using System.Threading.Tasks;
using System.Windows.Input;
using Splat;

namespace ReactiveUI.Samples.Suspension.ViewModels
{
    [DataContract]
    public class LoginViewModel : ReactiveObject, IRoutableViewModel
    {
        private readonly ReactiveCommand<Unit, Unit> _login;
        private string _password;
        private string _username;

        // We inject the IScreen implementation via the constructor.
        // If we receive null, we use Splat.Locator to resolve the
        // default implementation. The parameterless constructor is
        // required for the deserialization feature to work.
        public LoginViewModel(IScreen screen = null) 
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
                () => Task.Delay(1000), // emulate a long-running operation
                canLogin);
        }

        public IScreen HostScreen { get; }

        public string UrlPathSegment => "/login";

        public ICommand Login => _login;

        [DataMember]
        public string Username 
        {
            get => _username;
            set => this.RaiseAndSetIfChanged(ref _username, value);
        }

        // Note: Saving passwords to disk isn't a good idea. 
        public string Password 
        {
            get => _password;
            set => this.RaiseAndSetIfChanged(ref _password, value);
        }
    }
}