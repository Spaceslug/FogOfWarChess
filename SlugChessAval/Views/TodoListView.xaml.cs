using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace SlugChessAval.Views
{
    public class TodoListView : UserControl
    {
        public TodoListView()
        {
            this.InitializeComponent();
        }

        private void InitializeComponent()
        {
            AvaloniaXamlLoader.Load(this);
        }
    }
}
