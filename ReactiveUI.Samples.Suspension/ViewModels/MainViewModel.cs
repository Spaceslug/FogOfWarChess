
using System;
using System.Reactive;
using System.Reactive.Linq;
using System.Runtime.Serialization;
using System.Windows.Input;
using Avalonia.ReactiveUI;
namespace ReactiveUI.Samples.Suspension.ViewModels
{
    [DataContract]
    public class MainWindowViewModel : ReactiveObject, IScreen
    {
        private readonly ReactiveCommand<Unit, Unit> _search;
        private readonly ReactiveCommand<Unit, Unit> _login;
        private RoutingState _router = new RoutingState();

        public MainWindowViewModel()
        {
            // If the authorization page is currently shown, then
            // we disable the "Open authorization view" button.
            var canLogin = this.WhenAnyObservable(x => x.Router.CurrentViewModel)
                .Select(current => !(current is LoginViewModel));

            _login = ReactiveCommand.Create(
                () => { Router.Navigate.Execute(new LoginViewModel()); },
                canLogin);

            // If the search screen is currently shown, then we
            // disable the "Open search view" button.
            var canSearch = this
                .WhenAnyObservable(x => x.Router.CurrentViewModel)
                .Select(current => !(current is SearchViewModel));

            _search = ReactiveCommand.Create(
                () => { Router.Navigate.Execute(new SearchViewModel()); },
                canSearch);
        }

        [DataMember]
        public RoutingState Router
        {
            get => _router;
            set => this.RaiseAndSetIfChanged(ref _router, value);
        }

        [DataMember]
        public double GuiScaling
        {
            get => _guiScaling;
            set => this.RaiseAndSetIfChanged(ref _guiScaling, value);
        }
        private double _guiScaling = 1; 

        public int NormalFontSize => Convert.ToInt32(12 * _guiScaling);

        public ICommand Search => _search;

        public ICommand Login => _login;
    }
}
