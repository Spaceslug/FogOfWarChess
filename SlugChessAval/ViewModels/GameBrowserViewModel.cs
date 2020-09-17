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
using System.Reactive.Disposables;
using System.Reactive.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Input;

namespace SlugChessAval.ViewModels
{
    public class GameBrowserViewModel : ViewModelBase, IRoutableViewModel, IActivatableViewModel
    {
        public string UrlPathSegment => "/gamebrowser";
        public IScreen HostScreen { get; }

        public ViewModelActivator Activator { get; }

        private CancellationTokenSource _joinGameTokenSource = new CancellationTokenSource();

        public ICommand Cancel => ((MainWindowViewModel)HostScreen).Cancel;

        public ReactiveCommand<Unit, HostedGamesMap> RefreshGamesList { get; }
        public ReactiveCommand<Unit, LookForMatchResult> JoinGame { get; }

        public ObservableCollection<MatchInfoModel> MatchInfoModels { get; }
        public MatchInfoModel? SelectedItem
        {
            get => _selectedItem;
            set => this.RaiseAndSetIfChanged(ref _selectedItem, value);
        }
        private MatchInfoModel? _selectedItem;

        //public int SelectedIndex
        //{
        //    get => _selectedIndex;
        //    set => this.RaiseAndSetIfChanged(ref _selectedIndex, value);
        //}
        //private int _selectedIndex = -1;

        public GameBrowserViewModel(IScreen? screen = null)
        {
            HostScreen = screen ?? Locator.Current.GetService<IScreen>();
            Activator = new ViewModelActivator();
#if DEBUG
            MatchInfoModels = new ObservableCollection<MatchInfoModel>(MatchInfoModel.FromTestData());
#else
            MatchModels = new ObservableCollection<MatchModel>();
#endif

            RefreshGamesList = ReactiveCommand.CreateFromTask(() => 
                {
                    return SlugChessService.Client.Call.AvailableGamesAsync(new ChessCom.Void()).ResponseAsync;
                }
            );
            RefreshGamesList.Subscribe(x => { MatchInfoModels.Clear(); MatchInfoModels.AddRange(MatchInfoModel.FromChesscom(x)); });

            var canJoin = this.WhenAnyValue(x => x.SelectedItem).Select(o => o != null);

            JoinGame = ReactiveCommand.CreateFromTask(() =>JoinGameRequest(),
                canJoin);
            //JoinGame.Subscribe((result) => {
            //    if (result.Succes)
            //    {

            //    }
            //});
            this.WhenActivated(disposables =>
            {
                // Use WhenActivated to execute logic
                // when the view model gets activated.
                this.HandleActivation();

                // Or use WhenActivated to execute logic
                // when the view model gets deactivated.
                Disposable.Create(() => this.HandleDeactivation()).DisposeWith(disposables);
            });
        }

        private Task<LookForMatchResult> JoinGameRequest() => Task.Run(() =>
        {
#pragma warning disable CS8602 // Dereference of a possibly null reference.
#pragma warning disable CS8600 // Converting null literal or possible null value to non-nullable type.
            int selectedId = ((MatchInfoModel)SelectedItem).GetMatchId();
#pragma warning restore CS8600 // Converting null literal or possible null value to non-nullable type.
#pragma warning restore CS8602 // Dereference of a possibly null reference.
            var token = _joinGameTokenSource.Token;
            try
            {
                return SlugChessService.Client.Call.JoinGame(new ChessCom.JoinGameRequest
                {
                    Id = selectedId,
                    Joiner = SlugChessService.Client.UserData
                }, null, null, _joinGameTokenSource.Token);
            }
            catch (Grpc.Core.RpcException ex)
            {
                if (!token.IsCancellationRequested)
                {
                    throw ex;
                }
                return new LookForMatchResult { Succes = false };
            }
        });

        private void HandleActivation() 
        {
            RefreshGamesList.Execute().Subscribe();
        }

        private void HandleDeactivation() 
        {
            MatchInfoModels.Clear();
            SelectedItem = null;
        }

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
