﻿using System;
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
using Google.Protobuf;
using System.IO;
using System.Collections.ObjectModel;
using Avalonia.Collections;
using DynamicData.Binding;
using System.Linq;

namespace SlugChessAval.Models
{
    public class MatchModel 
    {

    
        public IObservable<MoveResult> MoveResults { get; private set; }
        public Subject<(TimeSpan whiteTimeLeft, TimeSpan blackTimeLeft, bool currentTurnWhite, bool ticking)> ChessClock { get; private set; }
        public string MatchToken { get; private set; }
        public UserData WhitePlayer { get; private set; }

        public UserData BlackPlayer { get; private set; }

        public BehaviorSubject<PlayerIs> ThisPlayer { get; }
        public IObservable<bool> ThisPlayerColored { get; }
        public IObservable<bool> PlayerIsActive { get; private set; } 
        //public UserData OpponentUserdata { get; private set; }

        public bool IsThisPlayersTurnNow => IsThisPlayersTurn.Take(1).Wait();
        public IObservable<bool> IsThisPlayersTurn { get; } 

        public BehaviorSubject<PlayerIs> CurrentTurnPlayer;

        public ICommand AskForDraw => _askForDraw;
        private readonly ReactiveCommand<Unit, Unit> _askForDraw;
        public ICommand AcceptDraw => _acceptDraw;
        private readonly ReactiveCommand<Unit, Unit> _acceptDraw;
        public ICommand Surrender => _surrender;
        public readonly ReactiveCommand<Unit, Unit> _surrender;

        public IObservable<bool> OpponentAskingForDraw => _opponentAskingForDraw;
        public BehaviorSubject<bool> _opponentAskingForDraw;

        public bool OngoingGameNow => OngoingGame.Take(1).Wait();
        public IObservable<bool> OngoingGame => _ongoingGame;
        public BehaviorSubject<bool> _ongoingGame;



        public AvaloniaList<ChessboardModel> ChessboardPositions { get; } = new AvaloniaList<ChessboardModel>();
        public IObservable<bool> ChessboardPositionsEmpty => _chessboardPositionsEmpty;
        private BehaviorSubject<bool> _chessboardPositionsEmpty { get; } = new BehaviorSubject<bool>(true);

        public MatchModel()
        {
            //ChessboardPositions.
            ChessboardPositions.ObserveCollectionChanges()
                .Subscribe(x => _chessboardPositionsEmpty.OnNext(x.EventArgs.Action == System.Collections.Specialized.NotifyCollectionChangedAction.Reset && (x.EventArgs.NewItems?.Count?? 0) == 0));
            //ChessboardPositions.ObserveCollectionChanges().Subscribe(x =>
            //{
            //    if(x.EventArgs.Action == System.Collections.Specialized.NotifyCollectionChangedAction.Reset && (x.EventArgs.NewItems?.Count ?? 0) == 0 )
            //    {
            //        Serilog.Log.Information("WAEWDAWDW");
            //    }
            //});
            _opponentAskingForDraw = new BehaviorSubject<bool>(false);
            _ongoingGame = new BehaviorSubject<bool>(false);
            MoveResults = Observable.Return(new ChessCom.MoveResult());
            MatchToken = "00000";
            ThisPlayer = new BehaviorSubject<PlayerIs>(PlayerIs.Both);
            ThisPlayerColored = ThisPlayer.Select(x => (x == PlayerIs.White || x == PlayerIs.Black));
            PlayerIsActive = Observable.CombineLatest(ThisPlayerColored, OngoingGame, (x, y) => x && y);
            //TODO: HACKED need to be better
            WhitePlayer = new UserData();
            BlackPlayer = new UserData();
            CurrentTurnPlayer = new BehaviorSubject<PlayerIs>(PlayerIs.White);
            IsThisPlayersTurn = Observable.CombineLatest(CurrentTurnPlayer, ThisPlayer,
                (ctPlayer, thisPlayer) => ctPlayer == thisPlayer).DistinctUntilChanged();
            ChessClock = new Subject<(TimeSpan whiteTimeLeft, TimeSpan blackTimeLeft, bool currentTurnWhite, bool ticking)>();

            _acceptDraw = ReactiveCommand.CreateFromObservable(AcceptDrawImpl, OpponentAskingForDraw);
            _askForDraw = ReactiveCommand.CreateFromObservable(AskForDrawImpl,
                Observable.CombineLatest(
                    OpponentAskingForDraw.Select( x => !x),
                    IsThisPlayersTurn,
                    (x, y) => x && y)
            );
            _surrender = ReactiveCommand.CreateFromObservable(SurrenderImpl, OngoingGame);


        }


        public void NewMatch(string matchToken, IObservable<MoveResult> obsMoveResults, PlayerIs playerIs, UserData opponentUserdata, CompositeDisposable endMatchDisposable)
        {
            ChessboardPositions.Clear();
            _ongoingGame.OnNext(true);
            MoveResults = obsMoveResults;
            MatchToken = matchToken;
            ThisPlayer.OnNext(playerIs);
            //OpponentUserdata = opponentUserdata;
            WhitePlayer = ThisPlayer.Take(1).Wait() == PlayerIs.White ? SlugChessService.Client.UserData : opponentUserdata;
            BlackPlayer = ThisPlayer.Take(1).Wait() == PlayerIs.Black ? SlugChessService.Client.UserData : opponentUserdata;
            MoveResults.Subscribe(
                (moveResult) => Dispatcher.UIThread.InvokeAsync(() => MoveResult(moveResult)),
                (error) => Dispatcher.UIThread.InvokeAsync(() => { _ongoingGame.OnNext(false); }),
                () => Dispatcher.UIThread.InvokeAsync(() => {_ongoingGame.OnNext(false); })
            ).DisposeWith(endMatchDisposable);
        }


        private void MoveResult(MoveResult moveResult)
        {
            if (moveResult.GameState != null)
            {
                ChessboardPositions.Add(ChessboardModel.FromChesscomGamestate(moveResult.GameState, ChessboardModel.ToVisionType(ThisPlayer.Take(1).Wait())));

                if (moveResult.MoveHappned && moveResult.ChessClock != null)
                {
                    ChessClock.OnNext(
                        (TimeSpan.FromSeconds(moveResult.ChessClock.WhiteSecondsLeft),
                        TimeSpan.FromSeconds(moveResult.ChessClock.BlackSecondsLeft),
                        moveResult.GameState.CurrentTurnIsWhite,
                        moveResult.ChessClock.TimerTicking)
                    );
                }
                CurrentTurnPlayer.OnNext(moveResult.GameState.CurrentTurnIsWhite ? PlayerIs.White : PlayerIs.Black);

                if (IsThisPlayersTurnNow && moveResult.MoveHappned)
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
                        if (!_askForDraw.IsExecuting.Take(1).Wait())
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
