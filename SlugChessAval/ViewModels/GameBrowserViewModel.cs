using ChessCom;
using DynamicData;
using ReactiveUI;
using SlugChessAval.Models;
using SlugChessAval.Services;
using Splat;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Reactive;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Input;

namespace SlugChessAval.ViewModels
{
    public class Person
    {
        public int DepartmentNumber { get; set; }

        public string DeskLocation { get; set; }

        public int EmployeeNumber { get; set; }

        public string FirstName { get; set; }

        public string LastName { get; set; }
    }

    public class GameBrowserViewModel : ViewModelBase, IRoutableViewModel
    {
        public string UrlPathSegment => "/gamebrowser";
        public IScreen HostScreen { get; }

        private CancellationTokenSource _joinGameTokenSource = new CancellationTokenSource();


        public ICommand Cancel => ((MainWindowViewModel)HostScreen).Cancel;

        public ReactiveCommand<Unit, HostedGamesMap> RefreshGamesList { get; }

        public ObservableCollection<MatchModel> MatchModels { get; }
        public ObservableCollection<Person> People { get; }
        public object SelectedItem
        {
            get => _selectedItem;
            set => this.RaiseAndSetIfChanged(ref _selectedItem, value);
        }
        private object _selectedItem;

        public GameBrowserViewModel(IScreen? screen = null)
        {
            HostScreen = screen ?? Locator.Current.GetService<IScreen>();
            People = new ObservableCollection<Person>(GenerateMockPeopleTable());
#if DEBUG
            MatchModels = new ObservableCollection<MatchModel>(MatchModel.FromTestData());
#else
            MatchModels = new ObservableCollection<MatchModel>();
#endif

            RefreshGamesList = ReactiveCommand.CreateFromTask(() => SlugChessService.Client.Call.AvailableGamesAsync(new ChessCom.Void()).ResponseAsync);
            RefreshGamesList.Subscribe(x => { MatchModels.Clear(); MatchModels.AddRange(MatchModel.FromChesscom(x)); });



        }

        private IEnumerable<Person> GenerateMockPeopleTable()
        {
            var defaultPeople = new List<Person>()
            {
                new Person()
                {
                    FirstName = "Pat",
                    LastName = "Patterson",
                    EmployeeNumber = 1010,
                    DepartmentNumber = 100,
                    DeskLocation = "B3F3R5T7"
                },
                new Person()
                {
                    FirstName = "Jean",
                    LastName = "Jones",
                    EmployeeNumber = 973,
                    DepartmentNumber = 200,
                    DeskLocation = "B1F1R2T3"
                },
                new Person()
                {
                    FirstName = "Terry",
                    LastName = "Tompson",
                    EmployeeNumber = 300,
                    DepartmentNumber = 100,
                    DeskLocation = "B3F2R10T1"
                }
            };

            return defaultPeople;
        }

        //private void JoinGame_Click(object sender, RoutedEventArgs e)
        //{
        //    if (dgMatchesDataGrid.SelectedIndex < 0) return;
        //    int selectedId = ((MatchesBind)dgMatchesDataGrid.Items[dgMatchesDataGrid.SelectedIndex]).GetMatchId();
        //    Task.Run(() => {
        //        var token = _joinGameTokenSource.Token;
        //        try
        //        {
        //            MatchResult = _connection.Call.JoinGame(new ChessCom.JoinGameRequest
        //            {
        //                Id = selectedId,
        //                Joiner = _userdata
        //            }, null, null, _joinGameTokenSource.Token);
        //            Dispatcher.Invoke(() => Close());
        //        }
        //        catch (Grpc.Core.RpcException ex)
        //        {
        //            if (!token.IsCancellationRequested)
        //            {
        //                throw ex;
        //            }
        //        }


        //    });
        //}

        //private void dgMatchesDataGrid_SelectionChanged(object sender, SelectionChangedEventArgs e)
        //{
        //    var dataGrid = sender as DataGrid;
        //    if (dataGrid.SelectedIndex == -1)
        //    {
        //        btJoinGame.IsEnabled = false;
        //    }
        //    else
        //    {
        //        btJoinGame.IsEnabled = true;
        //    }
        //}
    }
}
