using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using ReactiveUI;
using SlugChessAval.ViewModels;

namespace SlugChessAval.Views
{
    public class CreateGameView : ReactiveUserControl<CreateGameViewModel>
    {
        public CreateGameView()
        {
            AvaloniaXamlLoader.Load(this);
            this.WhenActivated(disposables =>
            {

            });
        }
    }
}
