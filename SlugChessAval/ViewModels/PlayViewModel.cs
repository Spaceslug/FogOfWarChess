using ReactiveUI;
using System.Reactive.Linq;
using SlugChessAval.Models;
using Splat;
using System;
using System.Collections.Generic;
using System.Runtime.Serialization;
using System.Text;

namespace SlugChessAval.ViewModels
{
    [DataContract]
    public class PlayViewModel : ViewModelBase, IRoutableViewModel
    {
        public string UrlPathSegment => "/play";
        public IScreen HostScreen { get; }


        public ChessboardViewModel Chessboard { get; private set; }

        public string LastMove
        {
            get => _lastMove;
            set => this.RaiseAndSetIfChanged(ref _lastMove, value);
        }
        private string _lastMove = "...";

        public PlayViewModel(IScreen screen = null)
        {
            HostScreen = screen ?? Locator.Current.GetService<IScreen>();


            Chessboard = new ChessboardViewModel { CbModel = ChessboardModel.FromTestData() };
            
            Chessboard.MoveFromTo.Select(t => t).Subscribe(t => {
                LastMove = $"From={t.from}, To={t.to}";
            });
        }

    }
}
