﻿using Avalonia.Threading;
using Google.Protobuf.WellKnownTypes;
using ReactiveUI;
using System;
using System.Collections.Generic;
using System.Data;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using System.Reactive.Subjects;
using System.Text;
using System.Threading;

namespace SlugChessAval.ViewModels
{
    public class ChessClockViewModel : ViewModelBase, IActivatableViewModel
    {
        public ViewModelActivator Activator { get; }

        public string WhiteTimeLeftString
        {
            get => _whiteTimeLeftString;
            private set => this.RaiseAndSetIfChanged(ref _whiteTimeLeftString, value);
        }
        private string _whiteTimeLeftString = "err";
        public TimeSpan WhiteTimeLeft
        {
            get => _whiteTimeLeft;
            private set { WhiteTimeLeftString = (value.TotalSeconds<0?"-":"") + value.ToString(@"mm\:ss"); _whiteTimeLeft = value; }
        }
        private TimeSpan _whiteTimeLeft;

        public string BlackTimeLeftString
        {
            get => _blackTimeLeftString;
            private set => this.RaiseAndSetIfChanged(ref _blackTimeLeftString, value);
        }
        private string _blackTimeLeftString = "err";
        public TimeSpan BlackTimeLeft
        {
            get => _blackTimeLeft;
            private set { BlackTimeLeftString = (value.TotalSeconds < 0 ? "-" : "") + value.ToString(@"mm\:ss"); _blackTimeLeft = value; }
        }
        private TimeSpan _blackTimeLeft;

        public string WhiteSecPerMove { get; } = "+00s";
        public string BlackSecPerMove { get; } = "+00s";

        public IObservable<int> SecLeft => _secLeft;
        public IObservable<bool> IsCurrentPlayersTurn;

        private Subject<int> _secLeft = new Subject<int>();
        private bool _blockTicker = true;
        private TimeSpan _lastTimeWhite;
        private TimeSpan _lastTimeBlack;
        private DispatcherTimer? _timer;
        private bool _currentTurnWhite;
        private readonly TimeSpan _interval = new TimeSpan(0, 0, 1);

        public ChessClockViewModel(TimeSpan whiteTimeLeft, TimeSpan blackTimeLeft, int secPerMove, IObservable<bool> isCurrentPlayersTurn)
        {
            Activator = new ViewModelActivator();
            IsCurrentPlayersTurn = isCurrentPlayersTurn;

            WhiteSecPerMove = $"+{secPerMove:D2}s";
            BlackSecPerMove = $"+{secPerMove:D2}s";
            WhiteTimeLeft = _lastTimeWhite = whiteTimeLeft;
            BlackTimeLeft = _lastTimeBlack = blackTimeLeft;
            _currentTurnWhite = true;

            this.WhenActivated(disposables =>
            {
                Observable.CombineLatest(
                    SecLeft,
                    IsCurrentPlayersTurn,
                    (x, y) =>  x < 6 && y
                ).Subscribe(both => {
                    if (both) ShellHelper.PlaySoundFile(Program.RootDir + "Assets/sounds/time_running_out.wav");
                }).DisposeWith(disposables);

                _timer = new DispatcherTimer();
                _timer.Interval = _interval;
                _timer.Tick += (o, e) =>
                {
                    if (_blockTicker) return;
                    if (_currentTurnWhite)
                    {
                        WhiteTimeLeft = _whiteTimeLeft.Subtract(_interval);
                        _secLeft.OnNext((int)WhiteTimeLeft.TotalSeconds);
                    }
                    else
                    {
                        BlackTimeLeft = _blackTimeLeft.Subtract(_interval);
                        _secLeft.OnNext((int)BlackTimeLeft.TotalSeconds);
                    }
                };
                _timer.Start();
                // Or use WhenActivated to execute logic
                // when the view model gets deactivated.
                Disposable.Create(() => { 
                    _timer.Stop();
                }).DisposeWith(disposables);
            });
        }

        public int GetSecondsSpent()
        {
            if (_currentTurnWhite)
            {
                return (int)(_lastTimeWhite - WhiteTimeLeft).TotalSeconds;
            }
            else
            {
                return (int)(_lastTimeBlack - BlackTimeLeft).TotalSeconds;
            }
        }

        public void StopTimer() => _blockTicker = true;
        public void StartTimer() => _blockTicker = false;

        public void SetTime(TimeSpan whiteTimeLeft, TimeSpan blackTimeLeft, bool currentTurnWhite, bool ticking)
        {
            _currentTurnWhite = currentTurnWhite;
            _blockTicker = !ticking;
            WhiteTimeLeft = _lastTimeWhite = whiteTimeLeft;
            BlackTimeLeft = _lastTimeBlack = blackTimeLeft;
        }
    }
}
