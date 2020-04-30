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
        //public Button MyButton;
        public Chessboard()
        {
            this.WhenActivated(disposables =>
            {
                // Bind the 'ExampleCommand' to 'ExampleButton' defined above.
                //this.BindCommand(ViewModel, x => x.ClickShit, x => x.ExampleGrid)
                //    .DisposeWith(disposables);

            });
            AvaloniaXamlLoader.Load(this);
            //ViewModel.mainChessboardGrid = this.FindControl<Grid>("mainChessboardGrid");
            //MyButton = this.FindControl<Button>("myButton");
        }

        private void ChessfieldClicked(object o, PointerReleasedEventArgs a)
        {
            ViewModel.ChessfieldClicked((Border)o);
        }


    }
}
