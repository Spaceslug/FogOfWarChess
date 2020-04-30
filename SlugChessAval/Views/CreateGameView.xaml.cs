using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using SlugChessAval.ViewModels;

namespace SlugChessAval.Views
{
    public class CreateGameView : ReactiveUserControl<CreateGameViewModel>
    {
        public CreateGameView()
        {
            this.InitializeComponent();
        }

        private void InitializeComponent()
        {
            AvaloniaXamlLoader.Load(this);
        }
    }
}
