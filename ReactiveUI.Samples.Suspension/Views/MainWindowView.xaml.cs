using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using ReactiveUI.Samples.Suspension.ViewModels;

namespace ReactiveUI.Samples.Suspension.Views
{
public sealed class MainWindowView : ReactiveWindow<MainWindowViewModel>
{
    public MainWindowView()
    {
        this.WhenActivated(disposables => { });
        AvaloniaXamlLoader.Load(this);
    }
}
}