using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using ReactiveUI.Samples.Suspension.ViewModels;

namespace ReactiveUI.Samples.Suspension.Views
{
    public sealed class LoginView : ReactiveUserControl<LoginViewModel>
    {
        public Grid ExampleGrid => this.FindControl<Grid>("gBoy");

        public LoginView()
        {
            this.WhenActivated(disposables =>
            {
                // Bind the 'ExampleCommand' to 'ExampleButton' defined above.
                this.BindCommand(ViewModel, x => x.ClickShit, x => x.ExampleGrid)
                    .DisposeWith(disposables);
            });
            AvaloniaXamlLoader.Load(this);
        }

        private void Thing(object sender, PointerReleasedEventArgs e)
        {

        }
    }
}
