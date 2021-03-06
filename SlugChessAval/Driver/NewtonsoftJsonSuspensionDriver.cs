﻿using System;
using System.IO;
using System.Reactive;
using System.Reactive.Linq;
using Newtonsoft.Json;
using ReactiveUI;
using SlugChessAval.ViewModels;

namespace SlugChessAval.Drivers
{
    public class NewtonsoftJsonSuspensionDriver : ISuspensionDriver
    {
        private readonly string _file;
        private readonly JsonSerializerSettings _settings = new JsonSerializerSettings
        {
            TypeNameHandling = TypeNameHandling.All,
            
        };

        public NewtonsoftJsonSuspensionDriver(string file) => _file = file;

        public IObservable<Unit> InvalidateState()
        {
            if (File.Exists(_file))
                File.Delete(_file);
            return Observable.Return(Unit.Default);
        }

        public IObservable<object> LoadState()
        {
            if (!File.Exists(_file)) {
                Console.WriteLine("appstate.json not found. Creating from default");
                return Observable.Return(new MainWindowViewModel());
            };
            //var open = File.OpenRead(_file);
            var lines = File.ReadAllText(_file);
            var state = JsonConvert.DeserializeObject<MainWindowViewModel>(lines, _settings) ?? new MainWindowViewModel();
            return Observable.Return(state);
        }

        public IObservable<Unit> SaveState(object state)
        {
            MainWindowViewModel? stateTyped = state as MainWindowViewModel;
            var lines = JsonConvert.SerializeObject(stateTyped, Formatting.Indented, _settings);
            File.WriteAllText(_file, lines);
            return Observable.Return(Unit.Default);
        }
    }
}