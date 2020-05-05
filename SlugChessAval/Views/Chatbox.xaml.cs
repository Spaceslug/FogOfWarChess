using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace SlugChessAval.Views
{
    public class Chatbox : UserControl
    {
        public Chatbox()
        {
            this.InitializeComponent();
        }

        private void InitializeComponent()
        {
            AvaloniaXamlLoader.Load(this);
        }
    }
}
