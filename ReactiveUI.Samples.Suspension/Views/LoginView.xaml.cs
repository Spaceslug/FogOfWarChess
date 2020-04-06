using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using ReactiveUI.Samples.Suspension.ViewModels;

namespace ReactiveUI.Samples.Suspension.Views
{
    public sealed class LoginView : ReactiveUserControl<LoginViewModel>
    {
        public LoginView()
        {
            this.WhenActivated(disposables => { });
            AvaloniaXamlLoader.Load(this);
        }
    }
}
