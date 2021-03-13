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
    public class ChessboardViewModel : ViewModelBase
    {
        public enum ViewTypes
        {
            Default,
            White,
            Black
        }

        public ViewTypes VisionTypeToViewType(ChessboardModel.VisionTypes x) => x switch {
            ChessboardModel.VisionTypes.Black => ViewTypes.Black,
            ChessboardModel.VisionTypes.White => ViewTypes.White,
            ChessboardModel.VisionTypes.Observer => ViewTypes.Default,
            _ => throw new ArgumentException("This does not exists")
        };

        public List<Field> FieldBoard
        {
            get => _fieldBoard;
            private set => this.RaiseAndSetIfChanged(ref _fieldBoard, value);
        }
        private List<Field> _fieldBoard;

        private List<Field> _baseFieldBoard;

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
        public string Hover
        {
            get => _hover;
            set => this.RaiseAndSetIfChanged(ref _hover, value);
        }
        private string _hover = "";

        public bool AllowedToSelect
        {
            get => _allowedToSelect;
            set => this.RaiseAndSetIfChanged(ref _allowedToSelect, value);
        }
        private bool _allowedToSelect;

        public ViewTypes ViewType
        {
            get => _viewType;
            set => this.RaiseAndSetIfChanged(ref _viewType, value);
        }
        private ViewTypes _viewType = ViewTypes.Default;

        public List<ViewTypes> AwailableViewTypes
        {
            get => _awailableViewTypes;
            set => this.RaiseAndSetIfChanged(ref _awailableViewTypes, value);
        }
        private List<ViewTypes> _awailableViewTypes = new List<ViewTypes>{ViewTypes.Default};

        public ReactiveCommand<(string from, string to), (string from, string to)> MoveFromTo;

        public IBrush FogBackground = new ImageBrush(AssetBank.GetImage("fog"));


        public double Width
        {
            get => _width;
            set => this.RaiseAndSetIfChanged(ref _width, value);
        }
        private double _width = 100;

        public Grid? mainChessboardGrid;
        public ChessboardViewModel()
        {
            _fieldBoard = Enumerable.Repeat(new Field {  }, 64).ToList();
            _baseFieldBoard = Enumerable.Repeat(new Field { }, 64).ToList();
            _cbModel = ChessboardModel.FromDefault();
            MoveFromTo = ReactiveCommand.Create<(string from, string to),(string from, string to)>( t => t);

            //Console.WriteLine(" whaaaaah");
            this.WhenAnyValue(x => x.CbModel).Subscribe(x => this.AwailableViewTypes = new List<ViewTypes>( x.Visions.Keys.Select(x => VisionTypeToViewType(x))));

            this.WhenAnyValue(x => x.AllowedToSelect).Where(x => !x).Subscribe(x => Selected = "");
            Observable.CombineLatest(
                this.WhenAnyValue(x => x.CbModel),
                this.WhenAnyValue(y => y.ViewType),
                (x, y) => (x, y)
            ).Subscribe(t => UpdateBoard(t.x));
            
            Observable.Merge(
                this.WhenAnyValue(x => x.Selected),
                this.WhenAnyValue(x => x.Hover)
            ).Subscribe(x => UpdateSelected());

        }

        public void ChessfieldClicked(Border border)
        {
            if (!AllowedToSelect ) return;
            if (border.Name == null) return;
            if (Selected == "") 
            {
                Selected = border.Name;
            }
            else if (Selected == border.Name)
            {
                Selected = "";
            }
            else if(CbModel.Moves.ContainsKey(Selected) && CbModel.Moves[Selected].Contains(border.Name))
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

        public void ChessfieldEnter(Border border)
        {
            if (!AllowedToSelect) return;
            if(border.Name==null) return;
            if (Selected == "")
            {
                Hover = border.Name;
            }
        }

        public void ChessfieldLeave(Border border)
        {
            if (!AllowedToSelect) return;
            if (Hover == border.Name)
            {
                Hover = "";
            }
        }

        private bool FieldVisible(int i, ChessboardModel model)
        {
            return ViewType switch
            {
                ViewTypes.Default => model.Visions[model.PlayerVisionType][i],
                ViewTypes.White => model.Visions[ChessboardModel.VisionTypes.White][i],
                ViewTypes.Black => model.Visions[ChessboardModel.VisionTypes.Black][i],
                _ => throw new ArgumentException("Inconsivable")
            };
        }

        private void UpdateBoard(ChessboardModel? model)
        {
            if (model == null) return;
            Selected = "";
            for (int i = 0; i < FieldBoard.Count; i++)
            {
                //var oldField = FieldBoard[i];
                var newField = new Field();
                
                if (FieldVisible(i, model))
                {
                    newField.Image = AssetBank.ImageFromPice(model.FieldPices[i]);
                    newField.Opacity = 1.0d;
                    if (ChessboardModel.BoardPos[i] == model.From || ChessboardModel.BoardPos[i] == model.To)
                    {
                        newField.ImageBackgroundBrush = Field.LastMove.ImageBackgroundBrush;
                    }
                    if(model.InACheck(ChessboardModel.BoardPos[i]))
                    {
                        newField.BorderThickness = Field.Check.BorderThickness;
                        newField.BorderBrush = Field.Check.BorderBrush;
                        newField.CornerRadius = Field.Check.CornerRadius;
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
            _baseFieldBoard.Clear();
            _baseFieldBoard.AddRange(FieldBoard);
        }

        private void UpdateSelected()
        {
            if (_cbModel == null) return;
            var selected = Selected;
            bool hoverMode = false;
            if (Selected == "")
            {
                selected = Hover;
                hoverMode = true;
            }
            for (int i = 0; i < FieldBoard.Count; i++)
            {
                FieldBoard[i] = new Field
                {
                    BorderBrush = _baseFieldBoard[i].BorderBrush,
                    BorderThickness = _baseFieldBoard[i].BorderThickness,
                    Image = _baseFieldBoard[i].Image,
                    CornerRadius = _baseFieldBoard[i].CornerRadius,
                    Opacity = _baseFieldBoard[i].Opacity,
                    ImageBackgroundBrush = _baseFieldBoard[i].ImageBackgroundBrush
                };
            }
            if (CbModel.Moves.ContainsKey(selected))
            {
                if(hoverMode == false)
                {
                    FieldBoard[ChessboardModel.BoardPosToIndex[selected]].BorderThickness = Field.Selected.BorderThickness;
                    FieldBoard[ChessboardModel.BoardPosToIndex[selected]].BorderBrush = Field.Selected.BorderBrush;
                    FieldBoard[ChessboardModel.BoardPosToIndex[selected]].CornerRadius = Field.Selected.CornerRadius;
                    FieldBoard[ChessboardModel.BoardPosToIndex[selected]].ImageBackgroundBrush = Field.Selected.ImageBackgroundBrush;
                }
                foreach (var moveTo in CbModel.Moves[selected])
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

        public class Field
        {
            public static uint ColorSolidRedHex = 0xAADD2222;
            public static uint ColorSolidGreenHex = 0xAA22DD22;
            public static uint ColorSoftYellowHex = 0xAADDDD66;
            public static uint ColorSoftGreenHex = 0xAA66DD66;

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
                BorderBrush = new SolidColorBrush(ColorSolidRedHex),
                CornerRadius = new CornerRadius(3.0d),
            };
            public static readonly Field LastMove = new Field
            {
                ImageBackgroundBrush = new SolidColorBrush { Color = Color.FromUInt32(ColorSoftYellowHex), Opacity = 1.0d }
            };

            public static readonly Field Selected = new Field
            {
                BorderThickness = new Thickness(3.0d),
                BorderBrush = new SolidColorBrush(ColorSolidGreenHex),
                CornerRadius = new CornerRadius(3.0d),
            };
            public static readonly Field SelectedShadow = new Field
            {
                BorderThickness = new Thickness(3.0d),
                BorderBrush = new SolidColorBrush(ColorSolidGreenHex),
                CornerRadius = new CornerRadius(3.0d),
            };
            public static readonly Field MoveTo = new Field
            {
                ImageBackgroundBrush = new SolidColorBrush { Color = Color.FromUInt32(ColorSoftGreenHex), Opacity = 1.0d }
            };
            public static readonly Field MoveToShadow = new Field
            {
                ImageBackgroundBrush = new SolidColorBrush { Color = Color.FromUInt32(ColorSoftGreenHex), Opacity = 1.0d }
            };
            public static readonly Field OldSelected = new Field
            {
                ImageBackgroundBrush = new SolidColorBrush { Color = Color.FromUInt32(0x6633AA22), Opacity = 1.0d }
            };
            public static readonly Field OldMoveTo = new Field
            {
                ImageBackgroundBrush = new RadialGradientBrush { GradientOrigin = RelativePoint.Center, Radius = 0.5d, Opacity = 1.0d, SpreadMethod = GradientSpreadMethod.Pad, 
                    GradientStops = new GradientStops { 
                        new GradientStop { Color=Color.FromUInt32(0xCC33AA22), Offset=0.0d},
                        new GradientStop { Color=Color.FromUInt32(0xCC33AA22), Offset=0.44d},
                        new GradientStop { Color=Color.FromUInt32(0x00000000), Offset=0.5d}
                    } 
                }
            };
            public static readonly Field OldMoveToShadow = new Field
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
