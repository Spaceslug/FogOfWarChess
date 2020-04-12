using Avalonia.Interactivity;
using ReactiveUI;
using SlugChessAval.Models;
using Splat;
using System;
using System.Collections.Generic;
using System.Reactive;
using System.Runtime.Serialization;
using System.Text;

namespace SlugChessAval.ViewModels
{
    public class ChessboardViewModel : ViewModelBase, IRoutableViewModel
    {
        public string UrlPathSegment => "/chessboard";
        public IScreen HostScreen { get; }

        [DataMember]
        public ChessboardModel CbModel
        {
            get => _cbModel;
            set => this.RaiseAndSetIfChanged(ref _cbModel, value);
        }
        private ChessboardModel _cbModel;

        public ReactiveCommand<Unit, (string, string)> MoveFromTo;

        public ChessboardViewModel(IScreen screen = null)
        {
            HostScreen = screen ?? Locator.Current.GetService<IScreen>();
            //MoveFromTo = ReactiveCommand.Create(
            //   () => new TodoItem { Description = Description },
            //   okEnabled);
            //Cancel = ReactiveCommand.Create(() => { });
            this.WhenAnyValue(x => x.CbModel, x => UpdateBoard(x));

        }

        public void ChessfieldClicked(object o, RoutedEventArgs e)
        {

        }

        private void UpdateBoard(ChessboardModel model)
        {

        }

        
    }
}
