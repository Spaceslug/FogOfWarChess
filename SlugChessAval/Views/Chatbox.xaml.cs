using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using ReactiveUI;
using SlugChessAval.ViewModels;

namespace SlugChessAval.Views
{
    public class Chatbox : ReactiveUserControl<ChatboxViewModel>
    {
        public Chatbox()
        {
            AvaloniaXamlLoader.Load(this);
            this.WhenActivated(disposables =>
            {

            });
            DataContext = new ChatboxViewModel();
        }

    }
}
