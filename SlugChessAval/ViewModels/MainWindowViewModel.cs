using System;
using System.Collections.Generic;
using System.Text;
using SlugChessAval.Services;
using ReactiveUI;
using System.Runtime.Serialization;
using System.Reactive.Linq;

namespace SlugChessAval.ViewModels
{
    public class MainWindowViewModel : ViewModelBase, IScreen
    {
        public string Greeting => "Welcome to Avalonia!";
        public string UserSignatureLabel => "Enter Signature here";
        public string UserSignature { get; set; }
        public TodoListViewModel List { get; }


        [DataMember]
        public RoutingState Router
        {
            get => _router;
            set => this.RaiseAndSetIfChanged(ref _router, value);
        }
        private RoutingState _router = new RoutingState();

        public MainWindowViewModel()
        {
            // If the authorization page is currently shown, then
            // we disable the "Open authorization view" button.
            //var canLogin = this.WhenAnyObservable(x => x.Router.CurrentViewModel)
            //    .Select(current => !(current is LoginViewModel));

            //_login = ReactiveCommand.Create(
            //    () => { Router.Navigate.Execute(new LoginViewModel()); },
            //    canLogin);

            // If the search screen is currently shown, then we
            // disable the "Open search view" button.
            //var canSearch = this
            //    .WhenAnyObservable(x => x.Router.CurrentViewModel)
            //    .Select(current => !(current is SearchViewModel));

            //_search = ReactiveCommand.Create(
            //    () => { Router.Navigate.Execute(new SearchViewModel()); },
            //    canSearch);
        }

        
    }
}
