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

namespace SlugChessAval.Models
{
    public class MatchModel
    {

        public class MatchTime
        {

        }

        public IObservable<MoveResult> MoveResults { get; private set; }
        public Subject<(TimeSpan whiteTimeLeft, TimeSpan blackTimeLeft, bool currentTurnWhite, bool ticking)> ChessClock { get; private set; }
        public string MatchToken { get; private set; }

        public PlayerIs PlayerIs { get; private set; }

        public IObservable<bool> IsCurrentPlayersTurn => CurrentTurnPlayer.Select(currentPlayerIs => currentPlayerIs == PlayerIs);
        public IObservable<bool> TestIsCurrentPlayersTurn; 

        public BehaviorSubject<PlayerIs> CurrentTurnPlayer;

        public MatchModel(string matchToken, IObservable<MoveResult> obsMoveResults, PlayerIs playerIs)
        {
            MoveResults = obsMoveResults;
            MatchToken = matchToken;
            PlayerIs = playerIs;
            CurrentTurnPlayer = new BehaviorSubject<PlayerIs>(PlayerIs.White);
            TestIsCurrentPlayersTurn = CurrentTurnPlayer.Select(currentPlayerIs => currentPlayerIs == PlayerIs);
            ChessClock = new Subject<(TimeSpan whiteTimeLeft, TimeSpan blackTimeLeft, bool currentTurnWhite, bool ticking)>();

            MoveResults.Subscribe(
            (moveResult) => Dispatcher.UIThread.InvokeAsync(() =>
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
                    _chessboardPositions.Add(ChessboardModel.FromChesscomGamestate(moveResult.GameState));
                    MoveDisplayIndex = _chessboardPositions.Count - 1;
                    WaitingOnMoveReply = false;
                    if (IsCurrentPlayersTurn.First() && moveResult.MoveHappned)
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
                                OpponentAskingForDraw = true;
                                DispatcherTimer.RunOnce(() => { OpponentAskingForDraw = false; }, TimeSpan.FromSeconds(5));
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
            }));
        }
    }
}
