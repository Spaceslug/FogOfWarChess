using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using ReactiveUI;

namespace SlugChessAval.Views
{
    public class LoginView : ReactiveUserControl<LoginView>
    {
        public LoginView()
        {
            this.WhenActivated(disposables =>
            {
                // Bind the 'ExampleCommand' to 'ExampleButton' defined above.
                //this.BindCommand(ViewModel, x => x.ClickShit, x => x.ExampleGrid)
                //    .DisposeWith(disposables);
            });
            AvaloniaXamlLoader.Load(this);
        }

    }
}
