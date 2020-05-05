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
using ChessCom;
using SlugChessAval.Services;

namespace SlugChessAval.ViewModels
{
    [DataContract]
    public class PlayViewModel : ViewModelBase, IRoutableViewModel
    {
        public string UrlPathSegment => "/play";
        public IScreen HostScreen { get; }


        public ChessboardViewModel Chessboard { get; private set; }
        public ChessClockViewModel ChessClock { get; private set; }
        private GameBrowserViewModel _vmGameBrowser { get; }
        private CreateGameViewModel _vmCreateGame { get; }

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

        public string PlayingAs => _playerIs switch { PlayerIs.White => "Playing as White", PlayerIs.Black => "Playing as Black", PlayerIs.Both => "Playing yourself", PlayerIs.Oberserver => "Watching as Observer", _ => "No game active" };
        public PlayerIs _playerIs = PlayerIs.Non;

        private string _matchToken { get; set; } = "0000";

        public ICommand MoveToCreateGame => _moveToCreateGame;
        private readonly ReactiveCommand<Unit, Unit> _moveToCreateGame;

        public ICommand MoveToGameBrowser => _moveToGameBrowser;
        private readonly ReactiveCommand<Unit, Unit> _moveToGameBrowser;

        public ReactiveCommand<Unit, LookForMatchResult> ConnectToGame { get; }

        public PlayViewModel(IScreen? screen = null)
        {
            HostScreen = screen ?? Locator.Current.GetService<IScreen>();

            Chessboard = new ChessboardViewModel { CbModel = ChessboardModel.FromTestData() };
            Chessboard.MoveFromTo.Subscribe(t => {
                LastMove = $"From={t.from}, To={t.to}";
                if(OngoingGame) //TDODO && isPlayersTurn)
                {
                    SlugChessService.Client.Call.SendMoveAsync( new MovePacket
                    {
                        AskingForDraw = false,
                        CheatMatchevent = MatchEvent.Non,
                        DoingMove = true,
                        MatchToken = _matchToken,
                        Usertoken = SlugChessService.Usertoken,
                        Move = new Move { From=t.from, To=t.to},
                    });
                    //TODO prevent Chessboard from selecting until server has responded with a MoveResult

                }
            });

            _vmGameBrowser = new GameBrowserViewModel { };
            _vmCreateGame = new CreateGameViewModel { };

            Observable.Merge(
                _vmCreateGame.HostGame,
                _vmGameBrowser.JoinGame).Subscribe((x) => { if (x.Succes) BootUpMatch(x); });


            var canMoveToCreateGame = this.WhenAny(vm => vm.OngoingGame, x => !x.Value);
            _moveToCreateGame = ReactiveCommand.Create(
                () => { HostScreen.Router.Navigate.Execute(_vmCreateGame).Subscribe(); },
                canMoveToCreateGame);

            var canMoveToGameBrowser = this.WhenAnyValue(vm => vm.LastMove).Select(x => true);
            _moveToGameBrowser = ReactiveCommand.Create(
                () => { HostScreen.Router.Navigate.Execute(_vmGameBrowser).Subscribe(); },
                canMoveToGameBrowser);
        }

        /// <summary>
        /// Allready determined result.Success is true
        /// </summary>
        /// <param name="result"></param>
        public void BootUpMatch(LookForMatchResult result)
        {
            OngoingGame = true;
            _matchToken = result.MatchToken;
            //TODO make game rules display
            //TODO make opponent data display
            _playerIs = result.IsWhitePlayer ? PlayerIs.White : PlayerIs.Black;
            //TODO play found game audio clip
            SlugChessService.Client.GetMatchListener(result.MatchToken).Subscribe((moveResult) => 
            {

                //TODO set currents turn from value in MoveResult
                //TODO update time in ChessClock from tim date in moveResult
                if (moveResult.GameState != null)
                {
                    Chessboard.CbModel = ChessboardModel.FromChesscomGamestate(moveResult.GameState);
                }


                //Not yet implemented on server
                //if (move.OpponentAskingForDraw) 
                //{
                //    Instance.Dispatcher.Invoke(() =>
                //    {
                //        string popupText = "Draw?";
                //        string textBoxText = "Opponent is asking for draw. Do you accept?";
                //        MessageBoxButton button = MessageBoxButton.YesNo;
                //        MessageBoxImage icon = MessageBoxImage.Error;
                //        var drawResult = MessageBox.Show(textBoxText, popupText, button, icon);
                //        if (drawResult == MessageBoxResult.Yes)
                //        {
                //            _matchStream.RequestStream.WriteAsync(new ChessCom.MovePacket { AskingForDraw = true });
                //            matchEnded = true;
                //        }
                //    });
                //} else if
                if (moveResult.MatchEvent == MatchEvent.UnexpectedClosing)
                {
                    //TODO print message to chat
                    //string popupText = "UnexpextedClosing";
                    //string textBoxText = "Opponents client unexpectedly closed";

                }
                else if (moveResult.MatchEvent == ChessCom.MatchEvent.WhiteWin || moveResult.MatchEvent == ChessCom.MatchEvent.BlackWin)
                {
                    //TODO print in log who won
                    //And send som motification shit
                }
            }, (error) => 
            {
                //TODO Print error to chatbox
                _playerIs = PlayerIs.Non;
                OngoingGame = false;
            }, () => 
            {
                _playerIs = PlayerIs.Non;
                OngoingGame = false;
            });
        }

    }
}
