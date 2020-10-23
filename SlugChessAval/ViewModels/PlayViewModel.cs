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
using System.Threading;
using Serilog;
using System.Reactive.Subjects;
using SharpDX.Direct2D1.Effects;
using System.Linq;
using Avalonia.Controls;
using System.IO;
using DynamicData.Binding;

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

        //public List<ChessboardModel> ChessboardPositions { get; } = new List<ChessboardModel>();

        public string LastMove
        {
            get => _lastMove;
            set => this.RaiseAndSetIfChanged(ref _lastMove, value);
        }
        private string _lastMove = "...";

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

        public string PlayingAs => (MatchModel?.PlayerIs??PlayerIs.Non) switch 
            { PlayerIs.White => "Playing as White", PlayerIs.Black => "Playing as Black", PlayerIs.Both => "Playing yourself", PlayerIs.Observer => "Watching as Observer", _ => "No game active" };

        public MatchModel MatchModel { get; }
        private string _matchToken { get; set; } = "0000";

        public ICommand MoveToCreateGame => _moveToCreateGame;
        private readonly ReactiveCommand<Unit, Unit> _moveToCreateGame;

        public ICommand MoveToGameBrowser => _moveToGameBrowser;
        private readonly ReactiveCommand<Unit, Unit> _moveToGameBrowser;

        public ICommand ViewPgnReplay => _viewPgnReplay;
        private readonly ReactiveCommand<Window, Unit> _viewPgnReplay;
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
            MatchModel = new MatchModel();
            MatchModel.ChessboardPositions.ObserveCollectionChanges().Where(x => x.EventArgs.Action == System.Collections.Specialized.NotifyCollectionChangedAction.Add)
                .Subscribe(x => MoveDisplayIndex = MatchModel.ChessboardPositions.Count - 1);

            Chessboard = new ChessboardViewModel{ CbModel = ChessboardModel.FromDefault()};
            
            _chessClock = new ChessClockViewModel(MatchModel.IsThisPlayersTurn);
            MatchModel.ChessClock.Subscribe(x => ChessClock.SetTime(x.whiteTimeLeft, x.blackTimeLeft, x.currentTurnWhite, x.ticking));
            _capturedPices = new CapturedPicesViewModel();

            _vmGameBrowser = new GameBrowserViewModel { };
            _vmCreateGame = new CreateGameViewModel { };
            _chatbox = new ChatboxViewModel { };
            Chessboard.MoveFromTo
                .Where(x => MatchModel.IsThisPlayersTurnNow && MatchModel.OngoingGameNow)
                .Subscribe(x => {
                    (string from, string to) = x;
                    SlugChessService.Client.Call.SendMoveAsync(new MovePacket
                    {
                        AskingForDraw = false,
                        CheatMatchevent = MatchEvent.Non,
                        DoingMove = true,
                        MatchToken = _matchToken,
                        Usertoken = SlugChessService.Usertoken,
                        Move = new Move { From = from, To = to, SecSpent = ChessClock.GetSecondsSpent(), Timestamp = Timestamp.FromDateTime(DateTime.UtcNow) },
                    });
                    WaitingOnMoveReply = true;
                }
            );

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
                    int.MaxValue => MatchModel.ChessboardPositions.Count - 1,
                    int.MinValue => 0,
                    _ => i + MoveDisplayIndex
                };
                if (newVal >= MatchModel.ChessboardPositions.Count - 1)
                {
                    MoveDisplayIndex = MatchModel.ChessboardPositions.Count - 1;
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
                Chessboard.CbModel = (i >= 0 ? MatchModel.ChessboardPositions[i] : ChessboardModel.FromDefault());
            });
            this.WhenAnyValue(x => x.WaitingOnMoveReply, x => x.MoveDisplayIndex, (b, i) => !b && i == MatchModel.ChessboardPositions.Count - 1)
                .Subscribe(allowedToSelect => Chessboard.AllowedToSelect = allowedToSelect);
            //Commands for Host and Join game
            var noOngoingGame = MatchModel.OngoingGame.Select(x => !x);
            _moveToCreateGame = ReactiveCommand.Create(
                () => { HostScreen.Router.Navigate.Execute(_vmCreateGame).Subscribe(); },
                noOngoingGame);
            _moveToGameBrowser = ReactiveCommand.Create(
                () => { HostScreen.Router.Navigate.Execute(_vmGameBrowser).Subscribe(); },
                noOngoingGame);
            _viewPgnReplay = ReactiveCommand.CreateFromObservable<Window, Unit>(ViewPgnReplayImpl);
            //Command for back and forward
            var moveIndexNotZero = this.WhenAnyValue(x => x.MoveDisplayIndex, i => i > 0);
            var moveIndexNotMax = this.WhenAnyValue(x => x.MoveDisplayIndex, i => i < MatchModel.ChessboardPositions.Count-1);
            _backEnd = ReactiveCommand.Create(
                () => { _shiftToMove.Execute(int.MinValue).Subscribe(); },
                moveIndexNotZero
                ); ;
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
            //Draw and Surrender
            //AskForDraw = MatchModel.AskForDraw;
            //AcceptDraw = MatchModel.AcceptDraw;
            //Surrender = MatchModel.Surrender;
            //_surrender = ReactiveCommand.CreateFromObservable(MatchModel.SurrenderImpl, MatchModel.OngoingGame);

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
            _matchToken = result.MatchToken;
            //TODO make game rules display
            //TODO make opponent data display
            MainWindowViewModel.SendNotification("You are playing agains " + result.OpponentUserData.Username + " as " + (result.IsWhitePlayer?"white":"black"));
            var playerTime = new TimeSpan(0, result.GameRules.TimeRules.PlayerTime.Minutes, result.GameRules.TimeRules.PlayerTime.Seconds);
            ShellHelper.PlaySoundFile(Program.RootDir + "Assets/sounds/match_start.wav");
            var matchObservable = SlugChessService.Client.GetMatchListener(result.MatchToken);
            CapturedPices = new CapturedPicesViewModel(matchObservable);
            Chatbox.OpponentUsertoken = result.OpponentUserData.Usertoken;
            CompositeDisposable disposablesForEndMatchWhenViewDeactivated = new CompositeDisposable();
            _chessClock.ResetTime(playerTime, playerTime, result.GameRules.TimeRules.SecondsPerMove);
            _chessClock.TimeRanOut.Subscribe((x) =>
            {
                if (x) //TODO: this hack should not be nessesary. Server need to detect time ran out itself
                {
                    _chessClock.StopTimer();
                    var firstMove = Chessboard.CbModel.Moves.First((x) => true);
                    (string from, string to) = (firstMove.Key, firstMove.Value[0]);
                    SlugChessService.Client.Call.SendMoveAsync(new MovePacket
                    {
                        AskingForDraw = false,
                        CheatMatchevent = MatchEvent.Non,
                        DoingMove = true,
                        MatchToken = _matchToken,
                        Usertoken = SlugChessService.Usertoken,
                        Move = new Move { From = from, To = to, SecSpent = ChessClock.GetSecondsSpent(), Timestamp = Timestamp.FromDateTime(DateTime.UtcNow) },
                    });
                    WaitingOnMoveReply = true;
                }
            }).DisposeWith(disposablesForEndMatchWhenViewDeactivated);

            MatchModel.NewMatch(result.MatchToken, matchObservable, result.IsWhitePlayer?PlayerIs.White:PlayerIs.Black, result.OpponentUserData, disposablesForEndMatchWhenViewDeactivated);
            Activator.Deactivated.Subscribe((u) =>
            {
                SlugChessService.Client.Call.SendMoveAsync(new MovePacket
                {
                    CheatMatchevent = MatchEvent.ExpectedClosing,
                    DoingMove = false,
                    MatchToken = _matchToken,
                    Usertoken = SlugChessService.Usertoken,
                });
            }).DisposeWith(disposablesForEndMatchWhenViewDeactivated);
            
            matchObservable.Subscribe(
                (moveResult) => Dispatcher.UIThread.InvokeAsync(() =>
                {
                    if (moveResult.GameState != null)
                    {

                        WaitingOnMoveReply = false;
                    }
                    if (moveResult.GameResult != null)
                    {
                        SaveMatchPgn(moveResult.GameResult.Pgn, MatchModel.WhitePlayer.Username, MatchModel.BlackPlayer.Username);
                    }
                }), 
                (error) => Dispatcher.UIThread.InvokeAsync(() =>
                {
                    MainWindowViewModel.SendNotification("Connection error");
                    SlugChessService.Client.MessageToLocal("Connection error:  " + error.ToString(), "system");
                    ChessClock.StopTimer();
                    Chatbox.OpponentUsertoken = null;
                    disposablesForEndMatchWhenViewDeactivated.Dispose();
                }), () => Dispatcher.UIThread.InvokeAsync(() =>
                {
                    ChessClock.StopTimer();
                    Chatbox.OpponentUsertoken = null;
                    disposablesForEndMatchWhenViewDeactivated.Dispose();
                })
            );
        }
        private IObservable<Unit> ViewPgnReplayImpl(Window window) => Observable.Start(() =>
        {
            OpenFileDialog dialog = new OpenFileDialog { AllowMultiple = false };
            //dialog.Directory = Program.RootDir.Substring(0, Program.RootDir.Length-1);
            dialog.Directory = Program.RootDir + "games_database";
            //dialog.Directory = "D:\\Projects";
            dialog.Title = "Select 'pgn' file to watch replay";
            dialog.Filters.Add(new FileDialogFilter() {Name="chess notation", Extensions = { "pgn" },  });

            string[] result = dialog.ShowAsync(window).Result;
            if (result.Length == 1)
            {
                Dispatcher.UIThread.InvokeAsync(() =>
                {
                    MainWindowViewModel.SendNotification($"PGN filepath '{result[0]}'");
                    var replay = SlugChessService.Client.Call.ProcessReplay(new GameResult { Pgn = File.ReadAllText(result[0]) });
                    //if(replay.Valid) TODO: Server must return if replay was valid!
                    MainWindowViewModel.SendNotification("Replay of " + replay.White + " vs " + replay.Black);
                    var playerTime = new TimeSpan(0, 0, 0);
                    _chessClock.ResetTime(playerTime, playerTime, 0);
                    ShellHelper.PlaySoundFile(Program.RootDir + "Assets/sounds/match_start.wav");
                    CompositeDisposable disposablesForEndMatchWhenViewDeactivated = new CompositeDisposable();

                    var matchObservable = new Subject<MoveResult>();

                    CapturedPices = new CapturedPicesViewModel(matchObservable);
                
                    MatchModel.NewMatch("replay", matchObservable, PlayerIs.Observer, new UserData { Username = "replay" }, disposablesForEndMatchWhenViewDeactivated);
                    matchObservable.Subscribe(
                        (moveResult) => Dispatcher.UIThread.InvokeAsync(() =>
                        {
                            if (moveResult.GameState != null)
                            {
                                WaitingOnMoveReply = false;
                            }
                        })
                    );
                    System.Threading.Tasks.Task.Run(() =>
                    {
                        for (int i = 0; i < replay.GameStates.Count; i++)
                        {
                            Thread.Sleep(100);
                            matchObservable.OnNext(new MoveResult
                            {
                                GameState = replay.GameStates[i],
                                MatchEvent = (i + 1 != replay.GameStates.Count ? MatchEvent.Non : replay.MatchEvent),
                                ChessClock = new ChessClock { BlackSecondsLeft = 0, WhiteSecondsLeft = 0, TimerTicking = false }
                            });

                        }
                        matchObservable.OnCompleted();
                    });
                });
                

            }
        });

        private void SaveMatchPgn(string pgn, string whiteUsername, string blackUsername)
        {
            if (!Directory.Exists(Program.RootDir + "games_database")) Directory.CreateDirectory(Program.RootDir + "games_database");
            if (!Directory.Exists(Program.RootDir + "games_database/last_few")) Directory.CreateDirectory(Program.RootDir + "games_database/last_few");
            File.WriteAllText(Program.RootDir + "games_database/latest.pgn", pgn);
            File.WriteAllText(Program.RootDir + $"games_database/last_few/{DateTime.Now:yyyy-MM-dd-HH-mm-ss}_{whiteUsername}_{blackUsername}.pgn", pgn);
        }
    }
}
