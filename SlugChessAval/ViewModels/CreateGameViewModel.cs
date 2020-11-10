using Avalonia.Collections;
using ChessCom;
using DynamicData.Binding;
using ReactiveUI;
using SlugChessAval.Services;
using Splat;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.Linq;
using System.Reactive;
using System.Reactive.Disposables;
using System.Reactive.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Input;

namespace SlugChessAval.ViewModels
{
    public class CreateGameViewModel : ViewModelBase, IRoutableViewModel, IActivatableViewModel
    {
        public const int DEFAULT_START_TIME_MIN = 5;
        public const int DEFAULT_EXTRA_TIME_SEC = 6;
        public const int DEFAULT_CHESSTYPE_INDEX = 2;
        public const int DEFAULT_VISION_RULES_INDEX = 1;
        public const int DEFAULT_HOST_COLOR_INDEX = 2;

        public ViewModelActivator Activator { get; } = new ViewModelActivator();
        public string UrlPathSegment => "/creategame";
        public IScreen HostScreen { get; }

        private CancellationTokenSource _hostGameTokenSource = new CancellationTokenSource();

        public ReactiveCommand<Unit, Unit> Cancel => ((MainWindowViewModel)HostScreen).Cancel;

        public ReactiveCommand<Unit, LookForMatchResult> HostGame { get; }

        public string StartTimeMin
        {
            get => _startTimeMin.ToString();
            set => this.RaiseAndSetIfChanged(ref _startTimeMin, int.TryParse(value, out int n) ? n : 0);
        }
        private int _startTimeMin = DEFAULT_START_TIME_MIN;

        public string ExtraTimeSec
        {
            get => _extraTimeSec.ToString();
            set => this.RaiseAndSetIfChanged(ref _extraTimeSec, int.TryParse(value, out int n) ? n : 0);
        }
        private int _extraTimeSec = DEFAULT_EXTRA_TIME_SEC;
        public int ChessTypeSelectedIndex
        {
            get => _chessTypeSelectedIndex;
            set => this.RaiseAndSetIfChanged(ref _chessTypeSelectedIndex, value);
        }
        private int _chessTypeSelectedIndex = DEFAULT_CHESSTYPE_INDEX;

        public int HostColorSelectedIndex
        {
            get => _hostColorSelectedIndex;
            set => this.RaiseAndSetIfChanged(ref _hostColorSelectedIndex, value);
        }
        private int _hostColorSelectedIndex = DEFAULT_HOST_COLOR_INDEX;

        public int VisionRulesSelectedIndex
        {
            get => _visionRulesSelectedIndex;
            set => this.RaiseAndSetIfChanged(ref _visionRulesSelectedIndex, value);
        }
        private int _visionRulesSelectedIndex = DEFAULT_VISION_RULES_INDEX;

        public IList<KeyValuePair<string, VisionRules>> VisionRuleItems => SlugChessService.Client.ServerVisionRuleset.OrderByDescending(x => x.Key).ToList();

        public CreateGameViewModel(IScreen? screen = null)
        {
            HostScreen = screen ?? Locator.Current.GetService<IScreen>();
            
            HostGame = ReactiveCommand.CreateFromTask(() => HostGameRequest());


            this.WhenActivated(disposables =>
            {
                _hostGameTokenSource = new CancellationTokenSource();
                Disposable.Create(() =>
                {
                    _hostGameTokenSource.Cancel();
                }).DisposeWith(disposables);
            });


        }

        private Task<LookForMatchResult> HostGameRequest() => Task.Run(() =>
       {
           try
           {
               var starttime = _startTimeMin;
               var movetime = _extraTimeSec;
               ChessCom.ChessType chessType = _chessTypeSelectedIndex == 0 ? ChessCom.ChessType.Classic :
                                               _chessTypeSelectedIndex == 1 ? ChessCom.ChessType.FisherRandom :
                                               ChessCom.ChessType.SlugRandom;
                ChessCom.SideType sideType = _hostColorSelectedIndex == 0 ? ChessCom.SideType.HostIsWhite :
                                                _hostColorSelectedIndex == 1 ? ChessCom.SideType.HostIsBlack :
                                                ChessCom.SideType.Random;
               string vrType = VisionRuleItems[_visionRulesSelectedIndex].Key;

                var token = _hostGameTokenSource.Token;
               ((MainWindowViewModel)HostScreen).Notification = "Hosting game";
               try
                   {
                       var matchResult = SlugChessService.Client.Call.HostGame(new ChessCom.HostedGame
                       {
                           Host = SlugChessService.Client.UserData,
                           GameRules = new ChessCom.GameRules
                           {
                                ChessType = chessType,
                                SideType = sideType,
                                TypeRules = vrType,
                                TimeRules = new ChessCom.TimeRules { PlayerTime = new ChessCom.Time { Minutes = starttime }, SecondsPerMove = movetime }
                           }
                       }, null, null, _hostGameTokenSource.Token);
                       return matchResult;

                   }
                   catch (Grpc.Core.RpcException ex)
                   {
                       if (!token.IsCancellationRequested)
                       {
                           throw ex;
                       }
                       return new LookForMatchResult { Succes = false };
                   }
           }
           catch (Exception ex)
           {
               throw ex;
           }
       });
    }
}
