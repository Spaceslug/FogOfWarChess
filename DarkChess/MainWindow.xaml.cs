using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace DarkChess
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public static readonly Dictionary<string, int> BoardPosToIndex = new Dictionary<string, int>
        {
            { "a1", 0 },
            { "b1", 1 },
            { "c1", 2 },
            { "d1", 3 },
            { "e1", 4 },
            { "f1", 5 },
            { "g1", 6 },
            { "h1", 7 },
            { "a2", 8 },
            { "b2", 9 },
            { "c2", 10 },
            { "d2", 11 },
            { "e2", 12 },
            { "f2", 13 },
            { "g2", 14 },
            { "h2", 15 },
            { "a3", 16 },
            { "b3", 17 },
            { "c3", 18 },
            { "d3", 19 },
            { "e3", 20 },
            { "f3", 21 },
            { "g3", 22 },
            { "h3", 23 },
            { "a4", 24 },
            { "b4", 25 },
            { "c4", 26 },
            { "d4", 27 },
            { "e4", 28 },
            { "f4", 29 },
            { "g4", 30 },
            { "h4", 31 },
            { "a5", 32 },
            { "b5", 33 },
            { "c5", 34 },
            { "d5", 35 },
            { "e5", 36 },
            { "f5", 37 },
            { "g5", 38 },
            { "h5", 39 },
            { "a6", 40 },
            { "b6", 41 },
            { "c6", 42 },
            { "d6", 43 },
            { "e6", 44 },
            { "f6", 45 },
            { "g6", 46 },
            { "h6", 47 },
            { "a7", 48 },
            { "b7", 49 },
            { "c7", 50 },
            { "d7", 51 },
            { "e7", 52 },
            { "f7", 53 },
            { "g7", 54 },
            { "h7", 55 },
            { "a8", 56 },
            { "b8", 57 },
            { "c8", 58 },
            { "d8", 59 },
            { "e8", 60 },
            { "f8", 61 },
            { "g8", 62 },
            { "h8", 63 }
        };


        private static RoutedUICommand _pressMeCommand = new RoutedUICommand("Press Me", "PressMe", typeof(MainWindow));
        public static RoutedUICommand PressMeCommand
        {
            get { return _pressMeCommand; }
        }

        public static Image BlackBishop { get { return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/BlackBishop.png", UriKind.Absolute)) }; } }
        public static Image BlackKing { get { return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/BlackKing.png", UriKind.Absolute)) }; } }
        public static Image BlackKnight { get { return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/BlackKnight.png", UriKind.Absolute)) }; } }
        public static Image BlackPawn { get { return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/BlackPawn.png", UriKind.Absolute)) }; } }
        public static Image BlackQueen { get { return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/BlackQueen.png", UriKind.Absolute)) }; } }
        public static Image BlackRook { get { return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/BlackRook.png", UriKind.Absolute)) }; } }
        public static Image WhiteBishop { get{ return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/WhiteBishop.png", UriKind.Absolute)) }; } }
        public static Image WhiteKing { get { return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/WhiteKing.png", UriKind.Absolute)) }; } }
        public static Image WhiteKnight
        {
            get
            {
                return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/WhiteKnight.png", UriKind.Absolute)) };
            }
        }
        public static Image WhitePawn
        {
            get
            {
                return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/WhitePawn.png", UriKind.Absolute)) };
            }
        }
        public static Image WhiteQueen { get
            {
                return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/WhiteQueen.png", UriKind.Absolute)) };
            }
        }
        public static Image WhiteRook { get
            {
                return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/WhiteRook.png", UriKind.Absolute)) };
            }
        }


        public static MainWindow Instance { get; private set; }

        private GlobalState _globalState = new GlobalState();
        private List<(string, List<FieldState>)> _legalMoves = new List<(string, List<FieldState>)>();
        private List<Pices> _killedPices = new List<Pices>();

        public MainWindow()
        {
            InitializeComponent();
            DataContext = new MyLittleFuckingDataContext();
            
            //Uri pageUri = new Uri("pack://siteoforigin:,,,/SiteOfOriginFile.xaml", UriKind.Absolute);
            //this.a6.Children.Add(new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/BlackKing.png", UriKind.Absolute)) });
            //this.a8.Children.Add(new Image { Source = new BitmapImage(new Uri("img\\BlackKing.png", UriKind.RelativeOrAbsolute)) });
            //string d = System.AppDomain.CurrentDomain.BaseDirectory;
            //this.a2.Children.Add(new Image { Source = new BitmapImage(new Uri("BlackPawn.png", UriKind.RelativeOrAbsolute)) });
            //Grid b8 = (Grid)this.BoardGrid.FindName("b8");
            //b8.Children.Clear();
            //b8.Children.Add(BlackPawn);
            //BlackOutField("d4");
            ClearBoard();
            _globalState = DarkChess.GlobalState.CreateStartState();
            UpdateBoardFromGlobalState();
            Instance = this;
        }

        public void ClearBoard()
        {
            foreach (Grid child in this.BoardGrid.Children)
            {
                child.Children.Clear();
                child.Visibility = Visibility.Visible;
            }
            KilledPicesGrid.Children.Clear();
        }

        public void UpdateBoardFromGlobalState()
        {
            var vision = GameRules.GetVision(_globalState, _globalState.WhiteTurn, new VisionRules { ViewMoveFields = false, ViewRange = 1 });
            foreach (Grid child in this.BoardGrid.Children)
            {


                
                ApplyFieldStateToGrid(child, _globalState.Board[BoardPosToIndex[child.Name]]);
                if (vision.Contains(child.Name))
                {
                    child.Opacity = 1;
                    foreach (UIElement underChiled in child.Children)
                    {
                        underChiled.Visibility = Visibility.Visible;
                    }
                }
                else
                {
                    child.Opacity = 0.2;
                    foreach (UIElement underChiled in child.Children)
                    {
                        underChiled.Visibility = Visibility.Hidden;
                    }
                }
                if (child.Name == _globalState.Selected)
                {
                    var border = new Border();
                    border.BorderBrush = Brushes.Red;
                    border.BorderThickness = new Thickness(2, 2, 2, 2); //You can specify here which borders do you want
                    child.Children.Add(border);
                }
                else if (_legalMoves.Any((a) => a.Item1 == child.Name))
                {
                    var border = new Border();
                    border.BorderBrush = Brushes.Red;
                    border.BorderThickness = new Thickness(3, 3, 3, 3); //You can specify here which borders do you want
                    child.Children.Add(border);
                }
            }
            _killedPices.Sort();
            foreach (var pice in _killedPices)
            {
                AddPiceToGrid(KilledPicesGrid, pice);
            }
        }

        private void Field_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            //sender
            if(e.LeftButton == MouseButtonState.Pressed)
            {
                Grid fieldGrid = (Grid)sender;
                Field clickState = _globalState.Board[BoardPosToIndex[fieldGrid.Name]];
                if (_globalState.Selected == null)
                {
                    if((_globalState.WhiteTurn && Field.HasWhitePice(clickState)) || (!_globalState.WhiteTurn && Field.HasBlackPice(clickState)))
                    {
                        _globalState.Selected = fieldGrid.Name;
                        _legalMoves = GameRules.GetLegalMoves(_globalState, new FieldState(_globalState.Selected, clickState));
                        ClearBoard();
                        UpdateBoardFromGlobalState();
                    }

                    
                }
                else
                {
                    if (_legalMoves.Any((a) => a.Item1 == fieldGrid.Name))
                    {
                        (var name, var extraFieldList) = _legalMoves.Find((a) => a.Item1 == fieldGrid.Name);
                        //Must clean an passants
                        _globalState.CleanAnPassants();
                        //
                        Field selectState = _globalState.Board[BoardPosToIndex[_globalState.Selected]];
                        if(extraFieldList == null || extraFieldList.Count == 0)
                        {
                            (Field from, Field to) = Move(selectState, clickState);
                            _globalState.Board[BoardPosToIndex[_globalState.Selected]] = from;
                            _globalState.Board[BoardPosToIndex[fieldGrid.Name]] = to;
                        }
                        else if(extraFieldList.Count == 1)
                        {
                            (Field from, Field to, Field anPs) = Move(selectState, clickState, extraFieldList[0].Field);
                            _globalState.Board[BoardPosToIndex[_globalState.Selected]] = from;
                            _globalState.Board[BoardPosToIndex[fieldGrid.Name]] = to;
                            _globalState.Board[BoardPosToIndex[extraFieldList[0].FieldName]] = anPs;
                        }
                        else if(extraFieldList.Count == 2)
                        {
                            (Field fromK, Field toK, Field fromR, Field toR) = Move(selectState, clickState, extraFieldList[0].Field, extraFieldList[1].Field);
                            _globalState.Board[BoardPosToIndex[_globalState.Selected]] = fromK;
                            _globalState.Board[BoardPosToIndex[fieldGrid.Name]] = toK;
                            _globalState.Board[BoardPosToIndex[extraFieldList[0].FieldName]] = fromR;
                            _globalState.Board[BoardPosToIndex[extraFieldList[1].FieldName]] = toR;
                        }
                        else
                        {
                            throw new NotImplementedException("what tha fuck man");
                        }

                        _globalState.WhiteTurn = !_globalState.WhiteTurn;

                        _globalState.Selected = null;
                        _legalMoves.Clear();
                        ClearBoard();
                        UpdateBoardFromGlobalState();
                    }
                    //

                }
                
            }
            else if(e.RightButton == MouseButtonState.Pressed)
            {
                _legalMoves.Clear();
                _globalState.Selected = null;
                ClearBoard();
                UpdateBoardFromGlobalState();
            }
            
        }

        private (Field, Field, Field, Field) Move(Field fromK, Field toK, Field fromR, Field toR)
        {
            if((fromK.Pice == Pices.WhiteKing && fromR.Pice == Pices.WhiteRook) || (fromK.Pice == Pices.BlackKing && fromR.Pice == Pices.BlackRook))
            {
                return (new Field(Pices.Non), new Field(fromK.Pice), new Field(Pices.Non), new Field(fromR.Pice));
            }
            else 
            {
                throw new NotImplementedException("I only know of castling that can use this");
            }
        }
        private (Field, Field, Field) Move(Field from, Field to, Field backup)
        {
            if (to.AnPassan_able)
            {
                _killedPices.Add(backup.Pice);
                return (new Field(Pices.Non), new Field(from.Pice), new Field(Pices.Non));
            }
            else if (from.Pice == Pices.WhitePawn || from.Pice == Pices.BlackPawn)
            {
                return (new Field(Pices.Non), new Field(from.Pice), new Field(Pices.Non, false, true, false, false));
            }
            else
            {
                throw new NotImplementedException("Only an passan here");
            }
        }
        private (Field, Field) Move(Field from, Field to)
        {
            if(to.Pice != Pices.Non)_killedPices.Add(to.Pice);
            return (new Field(Pices.Non), new Field(from.Pice));
        }

        private void ApplyFieldStateToGrid(Grid grid, Field fieldState)
        {
            switch (fieldState.Pice)
            {
                case Pices.Non:
                    
                    break;
                case Pices.BlackKing:
                    grid.Children.Add(BlackKing);
                    break;
                case Pices.BlackQueen:
                    grid.Children.Add(BlackQueen);
                    break;
                case Pices.BlackBishop:
                    grid.Children.Add(BlackBishop);
                    break;
                case Pices.BlackKnight:
                    grid.Children.Add(BlackKnight);
                    break;
                case Pices.BlackRook:
                    grid.Children.Add(BlackRook);
                    break;
                case Pices.BlackPawn:
                    grid.Children.Add(BlackPawn);
                    break;
                case Pices.WhiteKing:
                    grid.Children.Add(WhiteKing);
                    break;
                case Pices.WhiteQueen:
                    grid.Children.Add(WhiteQueen);
                    break;
                case Pices.WhiteBishop:
                    grid.Children.Add(WhiteBishop);
                    break;
                case Pices.WhiteKnight:
                    grid.Children.Add(WhiteKnight);
                    break;
                case Pices.WhiteRook:
                    grid.Children.Add(WhiteRook);
                    break;
                case Pices.WhitePawn:
                    grid.Children.Add(WhitePawn);
                    break;
            }
        }

        private void AddPiceToGrid(System.Windows.Controls.Primitives.UniformGrid grid, Pices pice)
        {
            switch (pice)
            {
                case Pices.Non:

                    break;
                case Pices.BlackKing:
                    grid.Children.Add(BlackKing);
                    break;
                case Pices.BlackQueen:
                    grid.Children.Add(BlackQueen);
                    break;
                case Pices.BlackBishop:
                    grid.Children.Add(BlackBishop);
                    break;
                case Pices.BlackKnight:
                    grid.Children.Add(BlackKnight);
                    break;
                case Pices.BlackRook:
                    grid.Children.Add(BlackRook);
                    break;
                case Pices.BlackPawn:
                    grid.Children.Add(BlackPawn);
                    break;
                case Pices.WhiteKing:
                    grid.Children.Add(WhiteKing);
                    break;
                case Pices.WhiteQueen:
                    grid.Children.Add(WhiteQueen);
                    break;
                case Pices.WhiteBishop:
                    grid.Children.Add(WhiteBishop);
                    break;
                case Pices.WhiteKnight:
                    grid.Children.Add(WhiteKnight);
                    break;
                case Pices.WhiteRook:
                    grid.Children.Add(WhiteRook);
                    break;
                case Pices.WhitePawn:
                    grid.Children.Add(WhitePawn);
                    break;
            }
        }

        public void BlackOutField(string fieldName)
        {
            Grid field = (Grid)this.BoardGrid.FindName(fieldName);
            field.Visibility = Visibility.Hidden;
        }
    }
}
