using System;
using System.Collections.Generic;
using System.Text;
using SlugChessAval.Services;
using ReactiveUI;
using System.Runtime.Serialization;
using System.Reactive.Linq;
using System.Reactive;
using System.Windows.Input;
using Grpc.Core;
using System.Threading.Tasks;
using System.Threading;
using System.Timers;
using System.Diagnostics;
using System.Reflection;
using Splat;
using Avalonia;
using SlugChessAval.Models;
using System.Reactive.Subjects;

namespace SlugChessAval.ViewModels
{
    public class MainWindowViewModel : ViewModelBase, IScreen
    {
        //public string Greeting => "Welcome to Avalonia!";
        //public string UserSignatureLabel => "Enter Signature here";
        //public string UserSignature { get; set; }
        //public TodoListViewModel List { get; }
        private static MainWindowViewModel _mainWindowViewModel = new MainWindowViewModel();
        public static void SendNotification(string noti)
        {
            _mainWindowViewModel.Notification = noti;
        }


        public TimeSpan WhiteTimeLeft => new TimeSpan(0, 5, 6);
        //public string Description
        //{
        //    get => _description;
        //    set => this.RaiseAndSetIfChanged(ref _description, value);
        //}
        //private string _description = "WEhaaaaatee";
        //public class Student
        //{
        //    public string Name { get; set; }
        //}
        //public List<Student> Students => new List<Student> { new Student { Name = "Dave" }, new Student { Name = "Britt" } };

        [DataMember]
        public double GuiScaling
        {
            get => _guiScaling;
            set => this.RaiseAndSetIfChanged(ref _guiScaling, value);
        }
        private double _guiScaling = 3;

        [DataMember]
        public double Width
        {
            get => _width;
            set => this.RaiseAndSetIfChanged(ref _guiScaling, _width);
        }
        private double _width = 300;

        public string Title
        {
            get => _title;
            set => this.RaiseAndSetIfChanged(ref _title, value);
        }
        private string _title = "SlugChess";

        public string NavigateBackText
        {
            get => _navigateBackText;
            set => this.RaiseAndSetIfChanged(ref _navigateBackText, value);
        }
        private string _navigateBackText = "Go Back";

        //[DataMember]
        public string Notification
        {
            get => _notification;
            set {
                if (value != "") { _notiTimer.Start(); };
                this.RaiseAndSetIfChanged(ref _notification, value); 
            }
        }
        private string _notification = "";
        private System.Timers.Timer _notiTimer;

        public int Font => 32;

        public int NormalFontSize => Convert.ToInt32(12 * _guiScaling);

        public string WaitingMessage { get; }

        public ICommand Exit => _exit;
        private readonly ReactiveCommand<Unit, Unit> _exit;

        public ICommand NavigateBack => _navigateBack;
        public readonly ReactiveCommand<Unit, Unit> _navigateBack;

        public IObservable<bool> MoveBackEnabled => _moveBackEnabled;
        BehaviorSubject<bool> _moveBackEnabled = new BehaviorSubject<bool>(false);



        public bool ClientActive
        {
            get => _clientActive;
            private set => this.RaiseAndSetIfChanged(ref _clientActive, value);
        }
        private bool _clientActive;


        //[DataMember]
        public RoutingState Router
        {
            get => _router;
            set => this.RaiseAndSetIfChanged(ref _router, value);
        }
        private RoutingState _router = new RoutingState();

        public MainWindowViewModel()
        {
            Title = $"SlugChess v{Program.GetSlugChessVersion()}";

            
            _exit = ReactiveCommand.Create(() => {
                App.Shutdown();
            });
            _notiTimer = new System.Timers.Timer(9000);
            _notiTimer.Elapsed += (Object source, ElapsedEventArgs e) => Notification = "";

            Router.NavigationChanged.Subscribe(x =>
            {
                if (Router.NavigationStack.Count == 1)
                {
                    NavigateBackText = "Go Back";
                    _moveBackEnabled.OnNext(false);
                }
                else if (Router.CurrentViewModel.Take(1).Wait() is PlayViewModel)
                {
                    NavigateBackText = "Logout";
                    _moveBackEnabled.OnNext(true);
                } 
                else 
                {
                    NavigateBackText = "Go Back";
                    _moveBackEnabled.OnNext(true);
                }
            });

            //Application.Current.Ex
            //_exit = ReactiveCommand.Create(() => this.)
            _navigateBack = ReactiveCommand.Create(() => { 
                if(Router.CurrentViewModel.Take(1).Wait() is PlayViewModel)
                {
                    SlugChessService.Client.Logout();
                }
                
                Router.NavigateBack.Execute().Subscribe();
            }, MoveBackEnabled);
            // If the authorization page is currently shown, then
            // we disable the "Open authorization view" button.
            //var canLogin = this.WhenAnyValue(x => x.ClientActive)
            //    .Select(current => Router.CurrentViewModel is );

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
            //Router.Navigate.Execute(new PlayViewModel());
            _mainWindowViewModel = this;

            if (Program.VersionCheckMessage != null)
            {
                WaitingMessage = "Version to old! " + Program.VersionCheckMessage;
                return;
            }
            WaitingMessage = "Waiting on window";

#if DEBUG
            int port = 43326;
#else
            int port = 43327;
#endif

            if(Program.LaunchedWithParam("--port"))
            {
                port = Convert.ToInt32(Program.GetParamValue("--port"));
            }

            Notification = "Connecting to server";
            SlugChessService.Instanciate("hive.spaceslug.no", port);
            SlugChessService.Client.UserLoggedIn.Subscribe(userLoggenIn => 
            {
                if (userLoggenIn)
                {
                    Notification = "Logged in as " + SlugChessService.Client.UserData?.Username ?? "non";
                }
                else
                {
                    Notification = "Logged out";
                }
            });
            //.;
            Router.Navigate.Execute(new StartMenuViewModel(this));
        }


    

        
    }
}
