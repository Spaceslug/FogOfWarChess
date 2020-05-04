using ReactiveUI;
using System;
using System.Collections.Generic;
using System.Text;

namespace SlugChessAval.ViewModels
{
    public class ChessClockViewModel : ViewModelBase
    {
        public TimeSpan WhiteTimeLeft
        {
            get => _whiteTimeLeft;
            set => this.RaiseAndSetIfChanged(ref _whiteTimeLeft, value);
        }
        private TimeSpan _whiteTimeLeft;

        public TimeSpan BlackTimeLeft
        {
            get => _blackTimeLeft;
            set => this.RaiseAndSetIfChanged(ref _blackTimeLeft, value);
        }
        private TimeSpan _blackTimeLeft;

        public int WhiteSecPerMove
        {
            get => _whiteSecPerMove;
            set => this.RaiseAndSetIfChanged(ref _whiteSecPerMove, value);
        }
        private int _whiteSecPerMove;

        public int BlackSecPerMove
        {
            get => _blackSecPerMove;
            set => this.RaiseAndSetIfChanged(ref _blackSecPerMove, value);
        }
        private int _blackSecPerMove;
    }
}
