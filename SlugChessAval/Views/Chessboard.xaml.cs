using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using ReactiveUI;
using SlugChessAval.ViewModels;

namespace SlugChessAval.Views
{
    public class Chessboard : ReactiveUserControl<ChessboardViewModel>
    {
        public Chessboard()
        {
            this.WhenActivated(disposables =>
            {
                // Bind the 'ExampleCommand' to 'ExampleButton' defined above.
                //this.BindCommand(ViewModel, x => x.ClickShit, x => x.ExampleGrid)
                //    .DisposeWith(disposables);
            });
            AvaloniaXamlLoader.Load(this);
        }

        private void ChessfieldClicked(object o, PointerReleasedEventArgs a)
        {

        }


    }
}
