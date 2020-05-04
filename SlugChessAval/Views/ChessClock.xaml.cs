using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace SlugChessAval.Views
{
    public class ChessClock : UserControl
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
