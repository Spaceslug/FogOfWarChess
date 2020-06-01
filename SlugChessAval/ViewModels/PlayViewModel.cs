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
using Avalonia.Threading;
using Google.Protobuf.WellKnownTypes;

namespace SlugChessAval.ViewModels
{
    [DataContract]
    public class PlayViewModel : ViewModelBase, IRoutableViewModel
    {
        public string UrlPathSegment => "/play";
        public IScreen HostScreen { get; }


        public ChessboardViewModel Chessboard { get; private set; }
        public ChessClockViewModel ChessClock
        {
            get => _chessClock;
            set => this.RaiseAndSetIfChanged(ref _chessClock, value);
        }
        private ChessClockViewModel _chessClock;

        public CapturedPicesViewModel CapturedPices
        {
            get => _capturedPices;
            set => this.RaiseAndSetIfChanged(ref _capturedPices, value);
        }
        private CapturedPicesViewModel _capturedPices;

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

        private PlayerIs _currentTurnPlayer = PlayerIs.Non;
        private string _matchToken { get; set; } = "0000";

        public ICommand MoveToCreateGame => _moveToCreateGame;
        private readonly ReactiveCommand<Unit, Unit> _moveToCreateGame;

        public ICommand MoveToGameBrowser => _moveToGameBrowser;
        private readonly ReactiveCommand<Unit, Unit> _moveToGameBrowser;

        //public ReactiveCommand<Unit, LookForMatchResult> ConnectToGame { get; }

        public PlayViewModel(IScreen? screen = null)
        {
            HostScreen = screen ?? Locator.Current.GetService<IScreen>();

            Chessboard = new ChessboardViewModel { CbModel = ChessboardModel.FromDefault() };
            Chessboard.MoveFromTo.Subscribe(t => {
                LastMove = $"From={t.from}, To={t.to}";
                if(OngoingGame && (_playerIs == _currentTurnPlayer))
                {
                    SlugChessService.Client.Call.SendMoveAsync( new MovePacket
                    {
                        AskingForDraw = false,
                        CheatMatchevent = MatchEvent.Non,
                        DoingMove = true,
                        MatchToken = _matchToken,
                        Usertoken = SlugChessService.Usertoken,
                        Move = new Move { From=t.from, To=t.to, SecSpent=ChessClock.GetSecondsSpent(), Timestamp=Timestamp.FromDateTime(DateTime.UtcNow)},
                    });
                    //TODO prevent Chessboard from selecting until server has responded with a MoveResult

                }
            });
            _chessClock = new ChessClockViewModel(new TimeSpan(), new TimeSpan(), 0);
            _capturedPices = new CapturedPicesViewModel();

            _vmGameBrowser = new GameBrowserViewModel { };
            _vmCreateGame = new CreateGameViewModel { };

            Observable.Merge(
                _vmCreateGame.HostGame,
                _vmGameBrowser.JoinGame).Subscribe((x) => { 
                    if (x.Succes) {
                        HostScreen.Router.NavigateBack.Execute().Subscribe();
                        BootUpMatch(x); 
                    } 
                });


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
            ((MainWindowViewModel)HostScreen).Notification = "You are playing agains " + result.OpponentUserData.Username + " as " + (result.IsWhitePlayer?"white":"black");
            _playerIs = result.IsWhitePlayer ? PlayerIs.White : PlayerIs.Black;
            var playerTime = new TimeSpan(0, result.GameRules.TimeRules.PlayerTime.Minutes, result.GameRules.TimeRules.PlayerTime.Seconds);
            ChessClock = new ChessClockViewModel(playerTime, playerTime, result.GameRules.TimeRules.SecondsPerMove);
            //TODO play found game audio clip
            var matchObservable = SlugChessService.Client.GetMatchListener(result.MatchToken);
            CapturedPices = new CapturedPicesViewModel(matchObservable);
            matchObservable.Subscribe((moveResult) => 
            {

                if (moveResult.GameState.CurrentTurnIsWhite) _currentTurnPlayer = PlayerIs.White; else _currentTurnPlayer = PlayerIs.Black;
                if (moveResult.MoveHappned)
                {
                    ChessClock.SetTime(TimeSpan.FromSeconds(moveResult.ChessClock.WhiteSecondsLeft), TimeSpan.FromSeconds(moveResult.ChessClock.BlackSecondsLeft), moveResult.GameState.CurrentTurnIsWhite, moveResult.ChessClock.TimerTicking);
                }
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
                    ChessClock.StopTimer();
                    OngoingGame = false;
                    _playerIs = PlayerIs.Oberserver;
                    //TODO print message to chat
                    //string popupText = "UnexpextedClosing";
                    //string textBoxText = "Opponents client unexpectedly closed";

                }
                else if (moveResult.MatchEvent == ChessCom.MatchEvent.WhiteWin || moveResult.MatchEvent == ChessCom.MatchEvent.BlackWin)
                {
                    ChessClock.StopTimer();
                    OngoingGame = false;
                    _playerIs = PlayerIs.Oberserver;
                    //TODO print in log who won
                    //And send som motification shit
                }
            }, (error) => Dispatcher.UIThread.InvokeAsync(() =>
            {

                //TODO Print error to chatbox
                ((MainWindowViewModel)HostScreen).Notification = "Connection error:  " + error.ToString();
                _playerIs = PlayerIs.Non;
                OngoingGame = false;
            }), () => Dispatcher.UIThread.InvokeAsync(() =>
            {
                _playerIs = PlayerIs.Non;
                OngoingGame = false;
            }));
        }

    }
}
