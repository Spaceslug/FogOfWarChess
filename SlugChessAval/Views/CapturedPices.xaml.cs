using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Presenters;
using Avalonia.Markup.Xaml;
using Avalonia.ReactiveUI;
using ReactiveUI;
using SlugChessAval.ViewModels;

namespace SlugChessAval.Views
{
    public class CapturedPices : ReactiveUserControl<CapturedPicesViewModel>
    {
        private ItemsRepeater _repeater;
        private ScrollViewer _scroller;
        public CapturedPices()
        {
            AvaloniaXamlLoader.Load(this);
            this.WhenActivated(disposables =>
            {
                ((CapturedPicesViewModel)DataContext).PanelWidth = Width;
                //if(((ScrollContentPresenter)_scroller.Presenter) != null)((ScrollContentPresenter)_scroller.Presenter).CanHorizontallyScroll = false;
            });
            _repeater = this.FindControl<ItemsRepeater>("repeater");
            _scroller = this.FindControl<ScrollViewer>("scroller");
            //DataContext = new CapturedPicesViewModel();
        }

    }
}
