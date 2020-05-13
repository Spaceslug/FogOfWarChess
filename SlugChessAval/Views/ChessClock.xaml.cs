using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using SlugChessAval.ViewModels;

namespace SlugChessAval.Views
{
    public class ChessClock : ReactiveUserControl<ChessClockViewModel>
    {
        public ChessClock()
        {
            this.InitializeComponent();
        }

        private void InitializeComponent()
        {
            AvaloniaXamlLoader.Load(this);
        }
    }
}
