using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using ReactiveUI;
using SlugChessAval.Models;
using SlugChessAval.Services;
using Splat;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reactive;
using System.Reactive.Linq;
using System.Runtime.Serialization;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SlugChessAval.ViewModels
{

    [DataContract]
    public class ChessboardViewModel : ViewModelBase, IRoutableViewModel
    {
        

        public List<Field> FieldBoard
        {
            get => _fieldBoard;
            private set => this.RaiseAndSetIfChanged(ref _fieldBoard, value);
        }
        private List<Field> _fieldBoard;


        public string UrlPathSegment => "/chessboard";
        public IScreen HostScreen { get; }

        //[DataMember]
        public ChessboardModel CbModel
        {
            get => _cbModel;
            set => this.RaiseAndSetIfChanged(ref _cbModel, value);
        }
        private ChessboardModel _cbModel;

        //[DataMember]
        public string Selected
        {
            get => _selected;
            set => this.RaiseAndSetIfChanged(ref _selected, value);
        }
        private string _selected = "";

        public ReactiveCommand<(string from, string to), (string from, string to)> MoveFromTo;

        public IBrush FogBackground = new ImageBrush(AssetBank.GetImage("fog"));


        public double Width
        {
            get => _width;
            set => this.RaiseAndSetIfChanged(ref _width, value);
        }
        private double _width = 100;

        public Grid? mainChessboardGrid;
        public ChessboardViewModel(IScreen? screen = null)
        {
            _fieldBoard = Enumerable.Repeat(new Field {  }, 64).ToList();
            _cbModel = ChessboardModel.FromDefault();
            MoveFromTo = ReactiveCommand.Create<(string from, string to),(string from, string to)>( t => t);
            //MoveFromTo.Select(t => $"From={t.from}, To={t.to}").Subscribe(s => A = s);
            ////MoveFromTo.Execute(("dad", "aaaa"));
            ////MoveFromTo.Subscribe(t => A = t.from + " whaaaaah");

            //A = "b";
            //var command = ReactiveCommand.Create<string, string>(s => { return s; });
            //command.(s => throw new ArgumentException("ffafafafa"));
            //command.Select(s => s).Subscribe(s => A = s);
            //Task.Run(() =>
            //{
            //    Thread.Sleep(3000);
            //    command.Execute("l").Subscribe();
            //    Thread.Sleep(1000);
            //    command.Execute("m").Subscribe();
            //    Thread.Sleep(1000);
            //    command.Execute("n").Subscribe();
            //    Thread.Sleep(1000);
            //    command.Execute("o").Subscribe();
            //});
            //command.Execute("f").Subscribe();
            //command.Subscribe(s => A = "aaddadwawadadwadwadw");
            //command.Select(s => s).Subscribe(s => A = s);
            //command.Execute(Unit.Default);
            //command.Execute(Unit.Default);
            //command.Execute(Unit.Default);


            //Console.WriteLine(" whaaaaah");

            HostScreen = screen ?? Locator.Current.GetService<IScreen>();
            //MoveFromTo = ReactiveCommand.Create(
            //   () => new TodoItem { Description = Description },
            //   okEnabled);
            //Cancel = ReactiveCommand.Create(() => { });
            this.WhenAnyValue(x => x.CbModel).Subscribe(x => UpdateBoard(x));
            this.WhenAnyValue(x => x.Selected).Subscribe(x => UpdateSelected(x));
            //Task.Run(() => { Thread.Sleep(2000); CbModel = ChessboardModel.FromTestData();  });
            //Task.Run(() => { Thread.Sleep(8000); Students = new List<Student> { new Student { Name = "aperrer" }, new Student { Name = "bitchface" } }; });
            
        }

        public void ChessfieldClicked(Border border)
        {
            if (Selected == "") Selected = border.Name;
            if(CbModel.Moves.ContainsKey(Selected) && CbModel.Moves[Selected].Contains(border.Name))
            {
                var from = Selected;
                Selected = "";
                MoveFromTo.Execute((from, border.Name)).Subscribe();
            }
            else
            {
                Selected = border.Name;
            }
        }

        private void UpdateBoard(ChessboardModel? model)
        {
            if (model == null) return;
            Selected = "";
            for (int i = 0; i < FieldBoard.Count; i++)
            {
                //var oldField = FieldBoard[i];
                var newField = new Field();

                if (model.Vision[i])
                {
                    newField.Image = ImageFromPice(model.FieldPices[i]);
                    newField.Opacity = 1.0d;
                    if (ChessboardModel.BoardPos[i] == model.From || ChessboardModel.BoardPos[i] == model.To)
                    {
                        newField.BorderThickness = Field.LastMove.BorderThickness;
                        newField.BorderBrush = Field.LastMove.BorderBrush;
                        newField.CornerRadius = Field.LastMove.CornerRadius;
                    }
                    else if(model.InACheck(ChessboardModel.BoardPos[i]))
                    {
                        newField.BorderThickness = Field.Check.BorderThickness;
                        newField.BorderBrush = Field.Check.BorderBrush;
                        newField.CornerRadius = Field.Check.CornerRadius;
                        newField.ImageBackgroundBrush = Field.MoveToShadow.ImageBackgroundBrush;
                    }
                    else
                    {
                        newField.BorderThickness = Field.None.BorderThickness;
                        newField.BorderBrush = Field.None.BorderBrush;
                        newField.CornerRadius = Field.None.CornerRadius;
                    }

                }
                else
                {
                    newField.Image = null;
                    newField.Opacity = 0.2d;
                    //newField.TestText = "fog boy";
                }
                FieldBoard[i] = newField;
            }
            FieldBoard = new List<Field>(FieldBoard);
        }

        private void UpdateSelected(string selected)
        {
            if (_cbModel == null) return;
            for (int i = 0; i < FieldBoard.Count; i++)
            {
                var oldField = FieldBoard[i];
                FieldBoard[i] = new Field
                {
                    BorderBrush = oldField.BorderBrush,
                    BorderThickness = oldField.BorderThickness,
                    Image = oldField.Image,
                    CornerRadius = oldField.CornerRadius,
                    Opacity = oldField.Opacity,
                    ImageBackgroundBrush = Field.None.ImageBackgroundBrush
                };
            }
            if (CbModel.Moves.ContainsKey(selected))
            {
                FieldBoard[ChessboardModel.BoardPosToIndex[selected]].ImageBackgroundBrush = Field.Selected.ImageBackgroundBrush;
                foreach(var moveTo in CbModel.Moves[selected])
                {
                    FieldBoard[ChessboardModel.BoardPosToIndex[moveTo]].ImageBackgroundBrush = Field.MoveTo.ImageBackgroundBrush;
                }
            }
            else if(CbModel.ShadowMoves.ContainsKey(selected))
            {
                FieldBoard[ChessboardModel.BoardPosToIndex[selected]].ImageBackgroundBrush = Field.SelectedShadow.ImageBackgroundBrush;
                foreach (var moveTo in CbModel.ShadowMoves[selected])
                {
                    FieldBoard[ChessboardModel.BoardPosToIndex[moveTo]].ImageBackgroundBrush = Field.MoveToShadow.ImageBackgroundBrush;
                }
            }
            FieldBoard = new List<Field>(FieldBoard);
        }


        private IBitmap? ImageFromPice(ChessCom.Pices pice) =>
            pice switch
            {
                ChessCom.Pices.None => null,
                ChessCom.Pices.BlackKing => AssetBank.GetImage("blackKing"),
                ChessCom.Pices.BlackQueen => AssetBank.GetImage("blackQueen"),
                ChessCom.Pices.BlackRook => AssetBank.GetImage("blackRook"),
                ChessCom.Pices.BlackBishop => AssetBank.GetImage("blackBishop"),
                ChessCom.Pices.BlackKnight => AssetBank.GetImage("blackKnight"),
                ChessCom.Pices.BlackPawn => AssetBank.GetImage("blackPawn"),
                ChessCom.Pices.WhiteKing => AssetBank.GetImage("whiteKing"),
                ChessCom.Pices.WhiteQueen => AssetBank.GetImage("whiteQueen"),
                ChessCom.Pices.WhiteRook => AssetBank.GetImage("whiteRook"),
                ChessCom.Pices.WhiteBishop => AssetBank.GetImage("whiteBishop"),
                ChessCom.Pices.WhiteKnight => AssetBank.GetImage("whiteKnight"),
                ChessCom.Pices.WhitePawn => AssetBank.GetImage("whitePawn"),
                _ => throw new ArgumentException(message: "invalid enum value", paramName: nameof(pice)),
            };

        public class Student
        {
            public string Name { get; set; } = "default";
        }

        public class Field
        {
            public IBitmap? Image { get; set; } = null;
            public Thickness BorderThickness { get; set; } = new Thickness(0.0d);
            public IBrush BorderBrush { get; set; } = ColorNull;
            public CornerRadius CornerRadius { get; set; } = new CornerRadius(0.0d);
            public double Opacity { get; set; } = 1.0d;
            public IBrush? ImageBackgroundBrush { get; set; } = Transparent;
            //public string TestText { get; set; } = "default";

            public static IBrush ColorNull = new SolidColorBrush(0xFF000000);
            public static IBrush Transparent = new SolidColorBrush(0x00000000);

            public static readonly Field None = new Field
            {
                Image = null,
                BorderThickness = new Thickness(0.0d),
                BorderBrush = new SolidColorBrush(0xFF000000),
                CornerRadius = new CornerRadius(0.0d),
                Opacity = 1.0d,
                ImageBackgroundBrush = null
            };
            public static readonly Field Check = new Field
            {
                BorderThickness = new Thickness(3.0d),
                BorderBrush = new SolidColorBrush(0xFFFFFF00),
                CornerRadius = new CornerRadius(3.0d),
            };
            public static readonly Field LastMove = new Field
            {
                BorderThickness = new Thickness(3.0d),
                BorderBrush = new SolidColorBrush(0xFF33FF00),
                CornerRadius = new CornerRadius(3.0d),
            };
            public static readonly Field Selected = new Field
            {
                ImageBackgroundBrush = new SolidColorBrush { Color = Color.FromUInt32(0x6633AA22), Opacity = 1.0d }
            };
            public static readonly Field SelectedShadow = new Field
            {
                ImageBackgroundBrush = new SolidColorBrush { Color = Color.FromUInt32(0x66555511), Opacity = 1.0d }
            };
            public static readonly Field MoveTo = new Field
            {
                ImageBackgroundBrush = new RadialGradientBrush { GradientOrigin = RelativePoint.Center, Radius = 0.5d, Opacity = 1.0d, SpreadMethod = GradientSpreadMethod.Pad, 
                    GradientStops = new GradientStops { 
                        new GradientStop { Color=Color.FromUInt32(0xCC33AA22), Offset=0.0d},
                        new GradientStop { Color=Color.FromUInt32(0xCC33AA22), Offset=0.44d},
                        new GradientStop { Color=Color.FromUInt32(0x00000000), Offset=0.5d}
                    } 
                }
            };
            public static readonly Field MoveToShadow = new Field
            {
                ImageBackgroundBrush = new RadialGradientBrush
                {
                    GradientOrigin = RelativePoint.Center, Radius = 0.5d, Opacity = 1.0d, SpreadMethod = GradientSpreadMethod.Pad,
                    GradientStops = new GradientStops {
                        new GradientStop { Color=Color.FromUInt32(0xCC777722), Offset=0.0d},
                        new GradientStop { Color=Color.FromUInt32(0xCC777722), Offset=0.44d},
                        new GradientStop { Color=Color.FromUInt32(0x00000000), Offset=0.5d}
                    }
                }
            };
            
        }
    }
}
