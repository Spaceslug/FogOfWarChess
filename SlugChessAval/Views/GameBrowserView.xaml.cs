using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using ReactiveUI;
using SlugChessAval.ViewModels;

namespace SlugChessAval.Views
{
    public class GameBrowserView : ReactiveUserControl<GameBrowserViewModel>
    {
        public GameBrowserView()
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
