using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.Threading;
using ChessCom;
using DynamicData;
using ReactiveUI;
using SlugChessAval.ViewModels.DataTemplates;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive.Disposables;
using System.Text;

namespace SlugChessAval.ViewModels
{
    public class CapturedPicesViewModel : ViewModelBase, IActivatableViewModel
    {
        public ViewModelActivator Activator { get; }

        public ObservableCollection<CapturedPice> Items
        {
            get => _items;
            set => this.RaiseAndSetIfChanged(ref _items, value);
        }
        private ObservableCollection<CapturedPice> _items = new ObservableCollection<CapturedPice>();

        public double PanelWidth
        {
            get => _panelWidth;
            set => this.RaiseAndSetIfChanged(ref _panelWidth, value);
        }
        private double _panelWidth;



        public CapturedPicesViewModel(IObservable<MoveResult>? observableResult = null)
        {
            Activator = new ViewModelActivator();

            if (observableResult != null)
            {
                observableResult.Subscribe(x => {
                    Items = new ObservableCollection<CapturedPice>( x.GameState.CapturedPices.Select(x => new CapturedPice(x))); 
                });
            }
            else
            {
                Items.Add(CapturedPice.Empty); //To give the layout some space
            }

            this.WhenActivated(disposables =>
            {
                //DispatcherTimer _timer = new DispatcherTimer();
                //_timer.Interval = new TimeSpan(0,0,5);
                //_timer.Tick += (o, e) =>
                //{
                //    Items.Add(CapturedPice.CapturedPices);
                //    //OtherItems.Add(new Item
                //    //{
                //    //    Text = $"ewrewrerwer"
                //    //});
                //    //_timer.Stop();
                //};
                //_timer.Start();

                Disposable.Create(() => 
                {  
                    
                }).DisposeWith(disposables);
            });
        }
    }
}
