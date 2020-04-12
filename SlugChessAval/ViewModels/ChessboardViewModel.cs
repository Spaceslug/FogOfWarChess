using Avalonia.Interactivity;
using ReactiveUI;
using SlugChessAval.Models;
using Splat;
using System;
using System.Collections.Generic;
using System.Reactive;
using System.Text;

namespace SlugChessAval.ViewModels
{
    public class ChessboardViewModel : ViewModelBase, IRoutableViewModel
    {
        public string UrlPathSegment => "/chessboard";
        public IScreen HostScreen { get; }



        public ReactiveCommand<Unit, (string, string)> MoveFromTo;

        public ChessboardViewModel(IScreen screen = null)
        {
            HostScreen = screen ?? Locator.Current.GetService<IScreen>();
            //MoveFromTo = ReactiveCommand.Create(
            //   () => new TodoItem { Description = Description },
            //   okEnabled);
            //Cancel = ReactiveCommand.Create(() => { });

        }

        public void ChessfieldClicked(object o, RoutedEventArgs e)
        {

        }
    }
}
