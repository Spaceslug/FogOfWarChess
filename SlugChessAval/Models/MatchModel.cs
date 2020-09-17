using System;
using ChessCom;
using Grpc.Core;
using System.Collections.Generic;
using System.Text;
using Avalonia.Threading;
using System.Reactive.Subjects;
using SlugChessAval.Services;
using SlugChessAval.ViewModels;
using System.Reactive;
using ReactiveUI;
using System.Reactive.Linq;
using System.Reactive.Disposables;
using System.Windows.Input;
using System.Threading;

namespace SlugChessAval.Models
{
    public class MatchModel 
    {



        public IObservable<MoveResult> MoveResults { get; private set; }
        public Subject<(TimeSpan whiteTimeLeft, TimeSpan blackTimeLeft, bool currentTurnWhite, bool ticking)> ChessClock { get; private set; }
        public string MatchToken { get; private set; }

        public PlayerIs PlayerIs { get; private set; }

        public bool IsCurrentPlayersTurnNow => IsCurrentPlayersTurn.Take(1).Wait();
        public IObservable<bool> IsCurrentPlayersTurn => CurrentTurnPlayer.Select(currentPlayerIs => currentPlayerIs == PlayerIs);

        public BehaviorSubject<PlayerIs> CurrentTurnPlayer;

        public bool AskingForDrawExecuting { get; private set; } = false;

        public ICommand AskForDraw => _askForDraw;
        private readonly ReactiveCommand<Unit, Unit> _askForDraw;
        public ICommand AcceptDraw => _acceptDraw;
        private readonly ReactiveCommand<Unit, Unit> _acceptDraw;
        public ICommand Surrender => _surrender;
        private readonly ReactiveCommand<Unit, Unit> _surrender;

        public IObservable<bool> OpponentAskingForDraw => _opponentAskingForDraw;
        public Subject<bool> _opponentAskingForDraw = new Subject<bool>();
        public IObservable<bool> MeAskingForDraw => _meAskingForDraw;
        public Subject<bool> _meAskingForDraw = new Subject<bool>();
        public IObservable<bool> OngoingGame => _ongoingGame;
        public Subject<bool> _ongoingGame = new Subject<bool>();

        public MatchModel()
        {
            MoveResults = Observable.Return(new ChessCom.MoveResult());
            MatchToken = "00000";
            PlayerIs = PlayerIs.Both;
            CurrentTurnPlayer = new BehaviorSubject<PlayerIs>(PlayerIs.White);
            ChessClock = new Subject<(TimeSpan whiteTimeLeft, TimeSpan blackTimeLeft, bool currentTurnWhite, bool ticking)>();

            _acceptDraw = ReactiveCommand.CreateFromObservable(AcceptDrawImpl, OpponentAskingForDraw);
            _askForDraw = ReactiveCommand.CreateFromObservable(AskForDrawImpl,
                Observable.CombineLatest(
                    OpponentAskingForDraw.Select( x => !x),
                    IsCurrentPlayersTurn,
                    (x, y) => x && y)
            );
            _surrender = ReactiveCommand.CreateFromObservable(SurrenderImpl, OngoingGame);

            _askForDraw.IsExecuting.Subscribe(x => AskingForDrawExecuting = x);

        }


        public void NewMatch(string matchToken, IObservable<MoveResult> obsMoveResults, PlayerIs playerIs, CompositeDisposable endMatchDisposable)
        {
            MoveResults = obsMoveResults;
            MatchToken = matchToken;
            PlayerIs = PlayerIs;
            MoveResults.Subscribe((moveResult) => Dispatcher.UIThread.InvokeAsync(() => MoveResult(moveResult))).DisposeWith(endMatchDisposable);
        }


        private void MoveResult(MoveResult moveResult)
        {
            if (moveResult.GameState != null)
            {

                if (moveResult.MoveHappned && moveResult.ChessClock != null)
                {
                    ChessClock.OnNext(
                        (TimeSpan.FromSeconds(moveResult.ChessClock.WhiteSecondsLeft),
                        TimeSpan.FromSeconds(moveResult.ChessClock.BlackSecondsLeft),
                        moveResult.GameState.CurrentTurnIsWhite,
                        moveResult.ChessClock.TimerTicking)
                    );
                }

                //if (moveResult.GameState.CurrentTurnIsWhite) _currentTurnPlayer = PlayerIs.White; else _currentTurnPlayer = PlayerIs.Black;
                CurrentTurnPlayer.OnNext(moveResult.GameState.CurrentTurnIsWhite ? PlayerIs.White : PlayerIs.Black);

                if (IsCurrentPlayersTurnNow && moveResult.MoveHappned)
                {
                    ShellHelper.PlaySoundFile(Program.RootDir + "Assets/sounds/move.wav");
                }
            }

            switch (moveResult.MatchEvent)
            {
                // Don't need to do closing here as server will terminate stream next, causing OnCompletet to be called 
                case MatchEvent.ExpectedClosing:
                    {
                        SlugChessService.Client.MessageToLocal("Opponent left the match. I suppose you won. Congratulations!", "system");
                        MainWindowViewModel.SendNotification("Opponent left match");
                    }
                    break;
                case MatchEvent.UnexpectedClosing:
                    {
                        SlugChessService.Client.MessageToLocal("Opponent unexpectedly disconnect. Match ended", "system");
                        MainWindowViewModel.SendNotification("Opponent disconnected");
                    }
                    break;
                case MatchEvent.WhiteWin:
                case MatchEvent.BlackWin:
                    {
                        SlugChessService.Client.MessageToLocal($"{(moveResult.MatchEvent == MatchEvent.WhiteWin ? "White" : "Black")} won the match", "system");
                        MainWindowViewModel.SendNotification($"{(moveResult.MatchEvent == MatchEvent.WhiteWin ? "White" : "Black")} won");
                    }
                    break;
                case MatchEvent.Draw:
                    {
                        SlugChessService.Client.MessageToLocal("Match ended in draw", "system");
                        MainWindowViewModel.SendNotification("Draw");
                    }
                    break;
                case MatchEvent.AskingForDraw:
                    {
                        if (!AskingForDrawExecuting)
                        {
                            SlugChessService.Client.MessageToLocal("Opponent requesting Draw", "system");
                            MainWindowViewModel.SendNotification("Opponent requesting Draw");
                            _opponentAskingForDraw.OnNext(true);
                            DispatcherTimer.RunOnce(() => { _opponentAskingForDraw.OnNext(false); }, TimeSpan.FromSeconds(5));
                        }

                    }
                    break;
                case MatchEvent.AcceptingDraw:
                    {
                        SlugChessService.Client.MessageToLocal("Opponent accepted Draw", "system");
                    }
                    break;
                case MatchEvent.Non:
                    {

                    }
                    break;
            }
        }

        private IObservable<Unit> AcceptDrawImpl() => Observable.Start(() =>
        {
            SlugChessService.Client.Call.SendMoveAsync(new MovePacket
            {
                CheatMatchevent = MatchEvent.AcceptingDraw,
                DoingMove = false,
                MatchToken = MatchToken,
                Usertoken = SlugChessService.Usertoken,
            });
            _opponentAskingForDraw.OnNext(false);
        });

        private IObservable<Unit> AskForDrawImpl() => Observable.Start(() =>
        {
            SlugChessService.Client.Call.SendMoveAsync(new MovePacket
            {
                CheatMatchevent = MatchEvent.AskingForDraw,
                DoingMove = false,
                MatchToken = MatchToken,
                Usertoken = SlugChessService.Usertoken,
            });
            Thread.Sleep(5000); //Holding the call for 5 sec prevents button form being pressed again 
        });

        private IObservable<Unit> SurrenderImpl() => Observable.Start(() =>
        {
            SlugChessService.Client.MessageToLocal("You surredered", "system");
            SlugChessService.Client.Call.SendMoveAsync(new MovePacket
            {
                CheatMatchevent = MatchEvent.ExpectedClosing,
                DoingMove = false,
                MatchToken = MatchToken,
                Usertoken = SlugChessService.Usertoken,
            });
        });
    }
}
