using SlugChessAval.ViewModels;
using System;
using System.Collections.Generic;
using System.Text;

namespace SlugChessAval
{
    public static class DesignData
    {
        public static ChessClockViewModel ChessClockExample { get; } = new ChessClockViewModel(new TimeSpan(0, 12, 2), new TimeSpan(0, 18, 5), 6);
    }
}
