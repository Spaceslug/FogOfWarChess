using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using SlugChessAval.ViewModels;

namespace SlugChessAval.Views
{
    public class GameBrowserView : ReactiveUserControl<GameBrowserViewModel>
    {
        public GameBrowserView()
        {
            this.InitializeComponent();
        }

        private void InitializeComponent()
        {
            AvaloniaXamlLoader.Load(this);
        }

        private void SelectionChanged(object o, SelectionChangedEventArgs a)
        {
            //ViewModel.ChessfieldClicked((Border)o);
        }
    }
}
