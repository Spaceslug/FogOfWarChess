using Avalonia.Threading;
using ReactiveUI;
using System;
using System.Collections.Generic;
using System.Data;
using System.Text;
using System.Threading;

namespace SlugChessAval.ViewModels
{
    public class ChessClockViewModel : ViewModelBase
    {
        public string WhiteTimeLeftString => WhiteTimeLeft.ToString(@"mm\:ss");
        public TimeSpan WhiteTimeLeft
        {
            get => _whiteTimeLeft;
            private set => this.RaiseAndSetIfChanged(ref _whiteTimeLeft, value);
        }
        private TimeSpan _whiteTimeLeft;

        public string BlackTimeLeftString => BlackTimeLeft.ToString(@"mm\:ss");
        public TimeSpan BlackTimeLeft
        {
            get => _blackTimeLeft;
            private set => this.RaiseAndSetIfChanged(ref _blackTimeLeft, value);
        }
        private TimeSpan _blackTimeLeft;

        public string WhiteSecPerMove { get; } = "+00s";
        public string BlackSecPerMove { get; } = "+00s";

        private TimeSpan _lastTimeWhite;
        private TimeSpan _lastTimeBlack;
        private DispatcherTimer _timer;
        private bool _currentTurnWhite;

        public ChessClockViewModel(TimeSpan whiteTimeLeft, TimeSpan blackTimeLeft, int secPerMove)
        {
            WhiteSecPerMove = $"+{secPerMove.ToString("D2")}s";
            BlackSecPerMove = $"+{secPerMove.ToString("D2")}s";
            WhiteTimeLeft = _lastTimeWhite = whiteTimeLeft; 
            BlackTimeLeft = _lastTimeBlack = blackTimeLeft;
            _timer = new DispatcherTimer();
            _timer.Interval = new TimeSpan(0,0,1);
            _currentTurnWhite = true;
        }



        public void SetTime(TimeSpan whiteTimeLeft, TimeSpan blackTimeLeft, bool currentTurnWhite, bool ticking)
        {
            _timer.Stop();
            _currentTurnWhite = currentTurnWhite;
            if (ticking)
            {
                _timer = new DispatcherTimer(new TimeSpan(0, 0, 1), DispatcherPriority.Normal, (o, e) =>
                {
                    if (currentTurnWhite)
                    {
                       WhiteTimeLeft = WhiteTimeLeft.Subtract(_timer.Interval);
                    }
                    else
                    {
                        BlackTimeLeft = BlackTimeLeft.Subtract(_timer.Interval);
                    }

                });
                _timer.Start();
                
            }
            WhiteTimeLeft = _lastTimeWhite = whiteTimeLeft;
            BlackTimeLeft = _lastTimeBlack = blackTimeLeft;
        }
    }
}
