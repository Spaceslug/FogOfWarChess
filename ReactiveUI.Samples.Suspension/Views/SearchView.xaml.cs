using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using ReactiveUI.Samples.Suspension.ViewModels;

namespace ReactiveUI.Samples.Suspension.Views
{
    public sealed class SearchView : ReactiveUserControl<SearchViewModel>
    {
        public SearchView()
        {
            // The call to WhenActivated is used to execute a block of code
            // when the corresponding view model is activated and deactivated.
            this.WhenActivated(disposables => { });
            AvaloniaXamlLoader.Load(this);
        }
    }
}
