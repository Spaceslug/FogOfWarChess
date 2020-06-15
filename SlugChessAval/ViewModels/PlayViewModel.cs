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
using System.Reactive.Disposables;

namespace SlugChessAval.ViewModels
{
    [DataContract]
    public class PlayViewModel : ViewModelBase, IRoutableViewModel, IActivatableViewModel
    {
        public ViewModelActivator Activator { get; } = new ViewModelActivator();
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

        public ChatboxViewModel Chatbox
        {
            get => _chatbox;
            set => this.RaiseAndSetIfChanged(ref _chatbox, value);
        }
        private ChatboxViewModel _chatbox;

        private GameBrowserViewModel _vmGameBrowser { get; }
        private CreateGameViewModel _vmCreateGame { get; }

        public List<ChessboardModel> _chessboardPositions { get; } = new List<ChessboardModel>();

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

        public bool WaitingOnMoveReply
        {
            get => _waitingOnMoveReply;
            set => this.RaiseAndSetIfChanged(ref _waitingOnMoveReply, value);
        }
        private bool _waitingOnMoveReply = false;

        public int MoveDisplayIndex
        {
            get => _moveDisplayIndex;
            set => this.RaiseAndSetIfChanged(ref _moveDisplayIndex, value);
        }
        private int _moveDisplayIndex = -1;

        public string PlayingAs => _playerIs switch { PlayerIs.White => "Playing as White", PlayerIs.Black => "Playing as Black", PlayerIs.Both => "Playing yourself", PlayerIs.Oberserver => "Watching as Observer", _ => "No game active" };
        public PlayerIs _playerIs = PlayerIs.Non;

        private PlayerIs _currentTurnPlayer = PlayerIs.Non;
        private string _matchToken { get; set; } = "0000";

        public ICommand MoveToCreateGame => _moveToCreateGame;
        private readonly ReactiveCommand<Unit, Unit> _moveToCreateGame;

        public ICommand MoveToGameBrowser => _moveToGameBrowser;
        private readonly ReactiveCommand<Unit, Unit> _moveToGameBrowser;
        /// <summary>
        /// Shift the board to display a move int amount of moves forward or backwards
        /// </summary>
        public ICommand ShiftToMove => _shiftToMove;
        private readonly ReactiveCommand<int, Unit> _shiftToMove;

        public ICommand BackEnd => _backEnd;
        private readonly ReactiveCommand<Unit, Unit> _backEnd;

        public ICommand BackOne => _backOne;
        private readonly ReactiveCommand<Unit, Unit> _backOne;

        public ICommand ForwardOne => _forwardOne;
        private readonly ReactiveCommand<Unit, Unit> _forwardOne;

        public ICommand ForwardEnd=> _forwardEnd;
        private readonly ReactiveCommand<Unit, Unit> _forwardEnd;

        //public ReactiveCommand<Unit, LookForMatchResult> ConnectToGame { get; }

        public PlayViewModel(IScreen? screen = null)
        {
            HostScreen = screen ?? Locator.Current.GetService<IScreen>();
            MoveDisplayIndex = -1;
            Chessboard = new ChessboardViewModel { CbModel = ChessboardModel.FromDefault() };
            Chessboard.MoveFromTo.Subscribe(t => {
                //LastMove = $"From={t.from}, To={t.to}";
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
                    WaitingOnMoveReply = true;
                    //TODO prevent Chessboard from selecting until server has responded with a MoveResult

                }
            });
            _chessClock = new ChessClockViewModel(new TimeSpan(), new TimeSpan(), 0);
            _capturedPices = new CapturedPicesViewModel();

            _vmGameBrowser = new GameBrowserViewModel { };
            _vmCreateGame = new CreateGameViewModel { };
            _chatbox = new ChatboxViewModel { };

            Observable.Merge(
                _vmCreateGame.HostGame, _vmGameBrowser.JoinGame)
                .Subscribe((x) => { 
                    if (x.Succes) {
                        HostScreen.Router.NavigateBack.Execute().Subscribe();
                        BootUpMatch(x); 
                    } 
                });

            _shiftToMove = ReactiveCommand.Create<int>(i => 
            {

                int newVal = i switch 
                {
                    int.MaxValue => _chessboardPositions.Count - 1,
                    int.MinValue => 0,
                    _ => i + MoveDisplayIndex
                };
                if (newVal >= _chessboardPositions.Count - 1)
                {
                    MoveDisplayIndex = _chessboardPositions.Count - 1;
                }
                else if(i + MoveDisplayIndex <= 0)
                {
                    MoveDisplayIndex = 0;
                }
                else
                {
                    MoveDisplayIndex = i + MoveDisplayIndex;
                }
            });

            this.WhenAnyValue(x => x.MoveDisplayIndex).Subscribe( i => 
            {
                Chessboard.CbModel = (i >= 0 ? _chessboardPositions[i] : ChessboardModel.FromDefault());
            });
            this.WhenAnyValue(x => x.WaitingOnMoveReply, x => x.MoveDisplayIndex, (b, i) => !b && i == _chessboardPositions.Count - 1)
                .Subscribe(allowedToSelect => Chessboard.AllowedToSelect = allowedToSelect);
            //Commands for Host and Join game
            var canMoveToCreateGame = this.WhenAnyValue(vm => vm.OngoingGame, x => !x);
            _moveToCreateGame = ReactiveCommand.Create(
                () => { HostScreen.Router.Navigate.Execute(_vmCreateGame).Subscribe(); },
                canMoveToCreateGame);
            var canMoveToGameBrowser = this.WhenAnyValue(vm => vm.OngoingGame, x => !x);
            _moveToGameBrowser = ReactiveCommand.Create(
                () => { HostScreen.Router.Navigate.Execute(_vmGameBrowser).Subscribe(); },
                canMoveToGameBrowser);
            //Command for back and forward
            var moveIndexNotZero = this.WhenAnyValue(x => x.MoveDisplayIndex, i => i > 0);
            var moveIndexNotMax = this.WhenAnyValue(x => x.MoveDisplayIndex, i => i < _chessboardPositions.Count-1);
            _backEnd = ReactiveCommand.Create(
                () => { _shiftToMove.Execute(int.MinValue).Subscribe(); },
                moveIndexNotZero
                );
            _backOne = ReactiveCommand.Create(
                () => { _shiftToMove.Execute(-1).Subscribe(); },
                moveIndexNotZero
                );
            _forwardOne = ReactiveCommand.Create(
                () => { _shiftToMove.Execute(1).Subscribe(); },
                moveIndexNotMax
                );
            _forwardEnd = ReactiveCommand.Create(
                () => { _shiftToMove.Execute(int.MaxValue).Subscribe(); },
                moveIndexNotMax
                );
            this.WhenActivated(disposables =>
            {

                Disposable.Create(() =>
                {

                }).DisposeWith(disposables);
            });
        }

        /// <summary>
        /// Allready determined result.Success is true
        /// </summary>
        /// <param name="result"></param>
        public void BootUpMatch(LookForMatchResult result)
        {
            OngoingGame = true;
            _chessboardPositions.Clear();
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
            Chatbox.OpponentUsertoken = result.OpponentUserData.Usertoken;
            var disposerForEndMatchWhenViewDeactivated = Activator.Deactivated.Subscribe((u) =>
            {
                SlugChessService.Client.Call.SendMoveAsync(new MovePacket
                {
                    CheatMatchevent = MatchEvent.ExpectedClosing,
                    DoingMove = false,
                    MatchToken = _matchToken,
                    Usertoken = SlugChessService.Usertoken,
                });
            });
            matchObservable.Subscribe(
            (moveResult) => Dispatcher.UIThread.InvokeAsync(() =>
            {

                if (moveResult.GameState != null)
                {

                    if (moveResult.MoveHappned && moveResult.ChessClock != null)
                    {
                        ChessClock.SetTime(TimeSpan.FromSeconds(moveResult.ChessClock.WhiteSecondsLeft), TimeSpan.FromSeconds(moveResult.ChessClock.BlackSecondsLeft), moveResult.GameState.CurrentTurnIsWhite, moveResult.ChessClock.TimerTicking);
                    }

                    //if (moveResult.GameState.CurrentTurnIsWhite) _currentTurnPlayer = PlayerIs.White; else _currentTurnPlayer = PlayerIs.Black;
                    _currentTurnPlayer = moveResult.GameState.CurrentTurnIsWhite ? PlayerIs.White : _currentTurnPlayer = PlayerIs.Black;
                    _chessboardPositions.Add(ChessboardModel.FromChesscomGamestate(moveResult.GameState));
                    MoveDisplayIndex = _chessboardPositions.Count - 1;
                    WaitingOnMoveReply = false;
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

                // Don't need to do closing here as server will terminate stream next, causing OnCompletet to be called 
                if (moveResult.MatchEvent == MatchEvent.ExpectedClosing)
                {
                    SlugChessService.Client.MessageToLocal("Opponent left the match. I suppose you won. Congratulations!", "system");
                    ((MainWindowViewModel)HostScreen).Notification = "Opponent left match";
                }
                else if (moveResult.MatchEvent == MatchEvent.UnexpectedClosing)
                {
                    SlugChessService.Client.MessageToLocal("Opponent unexpectedly disconnect. Match ended", "system");
                    ((MainWindowViewModel)HostScreen).Notification = "Opponent disconnected";
                }
                else if (moveResult.MatchEvent == ChessCom.MatchEvent.WhiteWin || moveResult.MatchEvent == ChessCom.MatchEvent.BlackWin)
                {
                    SlugChessService.Client.MessageToLocal($"{(moveResult.MatchEvent == MatchEvent.WhiteWin?"White":"Black")} won the match", "system");
                    ((MainWindowViewModel)HostScreen).Notification = $"{(moveResult.MatchEvent == MatchEvent.WhiteWin ? "White" : "Black")} won";
                }
            }), (error) => Dispatcher.UIThread.InvokeAsync(() =>
            {
                ((MainWindowViewModel)HostScreen).Notification = "Connection error";
                SlugChessService.Client.MessageToLocal("Connection error:  " + error.ToString(), "system");
                ChessClock.StopTimer();
                _playerIs = PlayerIs.Non;
                OngoingGame = false;
                Chatbox.OpponentUsertoken = null;
                disposerForEndMatchWhenViewDeactivated.Dispose();
            }), () => Dispatcher.UIThread.InvokeAsync(() =>
            {
                ChessClock.StopTimer();
                _playerIs = PlayerIs.Non;
                OngoingGame = false;
                Chatbox.OpponentUsertoken = null;
                disposerForEndMatchWhenViewDeactivated.Dispose();
            }));
        }

    }
}
