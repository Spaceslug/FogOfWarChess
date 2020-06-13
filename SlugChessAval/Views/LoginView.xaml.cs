using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using ReactiveUI;
using SlugChessAval.ViewModels;

namespace SlugChessAval.Views
{
    public class LoginView : ReactiveUserControl<LoginViewModel>
    {
        public LoginView()
        {

            AvaloniaXamlLoader.Load(this);
            this.WhenActivated(disposables =>
            {
                // Bind the 'ExampleCommand' to 'ExampleButton' defined above.
                //this.BindCommand(ViewModel, x => x.ClickShit, x => x.ExampleGrid)
                //    .DisposeWith(disposables);
            });
        }

    }
}
