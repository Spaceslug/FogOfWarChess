using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using ReactiveUI;
using SlugChessAval.ViewModels;

namespace SlugChessAval.Views
{
    public class ChessClock : ReactiveUserControl<ChessClockViewModel>
    {
        public ChessClock()
        {
            this.InitializeComponent();
            this.WhenActivated(disposables =>
            {

            });
        }

        private void InitializeComponent()
        {
            AvaloniaXamlLoader.Load(this);
        }
    }
}
