﻿using Avalonia.Controls;
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

        public List<CapturedPice> Items
        {
            get => _items;
            // If list becomes empty it stops working for some fucked up reasons
            set => this.RaiseAndSetIfChanged(ref _items, (value.Count > 0?value:new List<CapturedPice>{ CapturedPice.Empty }));
        }
        private List<CapturedPice> _items = new List<CapturedPice>();

        public double PanelWidth
        {
            get => _panelWidth;
            set => this.RaiseAndSetIfChanged(ref _panelWidth, value);
        }
        private double _panelWidth;



        public CapturedPicesViewModel()
        {
            Activator = new ViewModelActivator();
            //if (observableResult != null)
            //{
            //    observableResult.Subscribe(x => {
            //        Items = new ObservableCollection<CapturedPice>(x.GameState?.CapturedPices?.Select(x => new CapturedPice(x)).OrderBy(x => (int)x.PiceType).ToList() ?? Items.ToList()); 
            //        //I hacked this shit by casting everyting to List. Looks bad and should be fixed
            //    });
            //}
            //else
            //{
            //    Items.Add(CapturedPice.Empty); //To give the layout some space
            //}

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
