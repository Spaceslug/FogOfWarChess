using ReactiveUI;
using System.Reactive.Linq;
using SlugChessAval.Models;
using Splat;
using System;
using System.Collections.Generic;
using System.Runtime.Serialization;
using System.Text;
using System.Windows.Input;
using System.Reactive;

namespace SlugChessAval.ViewModels
{
    [DataContract]
    public class PlayViewModel : ViewModelBase, IRoutableViewModel
    {
        public string UrlPathSegment => "/play";
        public IScreen HostScreen { get; }


        public ChessboardViewModel Chessboard { get; private set; }

        public string LastMove
        {
            get => _lastMove;
            set => this.RaiseAndSetIfChanged(ref _lastMove, value);
        }
        private string _lastMove = "...";

        public bool OngoingGame
        {
            get => _ongoingGame;
            set => this.RaiseAndSetIfChanged(ref _ongoingGame, value);
        }
        private bool _ongoingGame = false;

        public ICommand MoveToCreateGame => _moveToCreateGame;
        private readonly ReactiveCommand<Unit, Unit> _moveToCreateGame;

        public ICommand MoveToGameBrowser => _moveToGameBrowser;
        private readonly ReactiveCommand<Unit, Unit> _moveToGameBrowser;

        public PlayViewModel(IScreen? screen = null)
        {
            HostScreen = screen ?? Locator.Current.GetService<IScreen>();


            Chessboard = new ChessboardViewModel { CbModel = ChessboardModel.FromTestData() };
            
            Chessboard.MoveFromTo.Select(t => t).Subscribe(t => {
                LastMove = $"From={t.from}, To={t.to}";
            });

            var canMoveToCreateGame = this.WhenAny(vm => vm.OngoingGame, x => !x.Value);
            _moveToCreateGame = ReactiveCommand.Create(
                () => { HostScreen.Router.Navigate.Execute(new CreateGameViewModel()).Subscribe(); },
                canMoveToCreateGame);

            var canMoveToGameBrowser = this.WhenAnyValue(vm => vm.LastMove).Select(x => true);
            _moveToGameBrowser = ReactiveCommand.Create(
                () => { HostScreen.Router.Navigate.Execute(new GameBrowserViewModel()).Subscribe(); },
                canMoveToGameBrowser);
        }

        public void BootUpMatch(ChessCom.LookForMatchResult result)
        {
            if (result.Succes)
            {

            }
        }

    }
}
