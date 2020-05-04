using ChessCom;
using ReactiveUI;
using SlugChessAval.Services;
using Splat;
using System;
using System.Collections.Generic;
using System.Reactive;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Input;

namespace SlugChessAval.ViewModels
{
    public class CreateGameViewModel : ViewModelBase, IRoutableViewModel
    {
        public const int DEFAULT_START_TIME_MIN = 5;
        public const int DEFAULT_EXTRA_TIME_SEC = 6;
        public const int DEFAULT_CHESSTYPE_INDEX = 2;
        public const int DEFAULT_VISION_RULES_INDEX = 0;
        public const int DEFAULT_HOST_COLOR_INDEX = 2;
        public string UrlPathSegment => "/creategame";
        public IScreen HostScreen { get; }

        private CancellationTokenSource _hostGameTokenSource = new CancellationTokenSource();

        public ICommand Cancel => ((MainWindowViewModel)HostScreen).Cancel;

        public ReactiveCommand<Unit, LookForMatchResult> HostGame { get; }

        public string StartTimeMin
        {
            get => _startTimeMin.ToString();
            set => this.RaiseAndSetIfChanged(ref _startTimeMin, int.TryParse(value, out int n)?n:0);
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

        public CreateGameViewModel(IScreen? screen = null)
        {
            HostScreen = screen ?? Locator.Current.GetService<IScreen>();

            HostGame = ReactiveCommand.CreateFromTask(() => HostGameRequest());

            HostGame.Subscribe(result => 
            {
                if (result.Succes)
                {

                }
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
                ChessCom.VisionRules vr = new ChessCom.VisionRules();
               switch (_visionRulesSelectedIndex)
               {
                   case 0: //Standard
                        {
                           vr.Enabled = true;
                           vr.ViewCaptureField = true;
                           vr.ViewMoveFields = false;
                           vr.ViewRange = 2;
                           ChessCom.VisionRules overwrite = new ChessCom.VisionRules
                           {
                               ViewRange = 1,
                               ViewMoveFields = false
                           };
                           vr.PiceOverwriter.Add((int)ChessCom.Pices.WhitePawn, overwrite);
                           vr.PiceOverwriter.Add((int)ChessCom.Pices.BlackPawn, overwrite);
                       }
                       break;
                   case 1: //Sea
                        {
                           vr.Enabled = true;
                           vr.ViewCaptureField = true;
                           vr.ViewMoveFields = true;
                           vr.ViewRange = 0;
                           ChessCom.VisionRules overwrite = new ChessCom.VisionRules
                           {
                               ViewRange = 1,
                               ViewMoveFields = true
                           };
                           vr.PiceOverwriter.Add((int)ChessCom.Pices.WhiteKnight, overwrite);
                           vr.PiceOverwriter.Add((int)ChessCom.Pices.BlackKnight, overwrite);
                           vr.PiceOverwriter.Add((int)ChessCom.Pices.WhitePawn, overwrite);
                           vr.PiceOverwriter.Add((int)ChessCom.Pices.BlackPawn, overwrite);
                       }
                       break;
                   case 2: //No Vision
                        {
                           vr.Enabled = false;
                       }
                       break;
               }

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
                                VisionRules = vr,
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
