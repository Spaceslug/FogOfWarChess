using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
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
        private List<Pices> _killedPices = new List<Pices>();
        private VisionMode _visionMode = VisionMode.CurrentMove;
        private ServerConnection _connection;
        private string _userToken;

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
            _globalState = GlobalState.CreateStartState(new VisionRules{ Enabled = false });
            UpdateBoardFromGlobalState();

            Instance = this;
            //ServerConnection connection = new ServerConnection("hive.spaceslug.no", 43326);
            _connection = new ServerConnection("hive.spaceslug.no", 43326);
            try
            {
                _connection.Connect();
                var a = _connection.Call.sendRequest(new ChessCom.MathRequest { A = 3, B = 4 });
                //connection.Call.
                Console.WriteLine(a);
            }
            catch (AggregateException ex)
            {
                string popupText = "Slug Chess Connection failed";
                string textBoxText = "Can not connect to Slug Chess server. Please bother admin at support@spaceslug.no. Continue to singleplayer?";
                MessageBoxButton button = MessageBoxButton.YesNo;
                MessageBoxImage icon = MessageBoxImage.Error;
                var result = MessageBox.Show(textBoxText, popupText, button, icon);
                if(result == MessageBoxResult.No) Application.Current.Shutdown();
            }
        }

        public void Runner()
        {
            ChessCom.LookForMatchResult result = _connection.Call.LookForMatch(new ChessCom.UserIdentity { UserToken = _userToken });
            if (result.Succes)
            {
                StartMatch(result.IsWhitePlayer, result.MatchToken);
                var matchStream = _connection.Call.Match();
                bool matchEnded = false;
                while (!matchEnded)
                {
                    ChessCom.MoveResult move = matchStream.ResponseStream.Current;
                    if (move.MoveHappned)
                    {
                        _globalState.Selected = move.From;
                        _globalState.DoMoveTo(move.To);
                    }
                    //move.OpponentAskingForDraw;
                }
            }
            else
            {
                string popupText = "Failed";
                string textBoxText = "Failed to find match";
                MessageBoxButton button = MessageBoxButton.OK;
                MessageBoxImage icon = MessageBoxImage.Error;
                MessageBox.Show(textBoxText, popupText, button, icon);
            }
        }

        public void StartMatch(bool isWhitePlayer, string matchToken)
        {
            _visionMode = isWhitePlayer ? VisionMode.White : VisionMode.Black;
            ClearBoard();
            _globalState = GlobalState.CreateStartState(new VisionRules
            {
                Enabled = true,
                ViewMoveFields = false,
                ViewRange = 2,
                PiceOverwrite = new Dictionary<Pices, VisionRules>()
                {
                    [Pices.BlackPawn] = new VisionRules { ViewRange = 1 },
                    [Pices.WhitePawn] = new VisionRules { ViewRange = 1 },
                    [Pices.BlackKing] = new VisionRules { ViewRange = 1 },
                    [Pices.WhiteKing] = new VisionRules { ViewRange = 1 },
                }
            });
            UpdateBoardFromGlobalState();
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
            //var vision = GameRules.GetVision(_globalState, _globalState.WhiteTurn, new VisionRules { ViewMoveFields = false, ViewRange = 1 });
            

            foreach (Grid child in this.BoardGrid.Children)
            {
                ApplyFieldStateToGrid(child, _globalState.GetFieldAt(child.Name));
                if (_globalState.CanSeeField(_visionMode, child.Name))
                {
                    child.Opacity = 1;
                    foreach (UIElement underChiled in child.Children)
                    {
                        underChiled.Visibility = Visibility.Visible;
                    }
                }
                else
                {
                    child.Visibility = Visibility.Hidden;
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
                else if (_globalState.IsLegalMove(child.Name))
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
                Field clickState = _globalState.GetFieldAt(fieldGrid.Name);
                if (_globalState.Selected == null)
                {
                    if((_globalState.WhiteTurn && Field.HasWhitePice(clickState)) || (!_globalState.WhiteTurn && Field.HasBlackPice(clickState)))
                    {
                        _globalState.Selected = fieldGrid.Name;
                        //_legalMoves = GameRules.GetLegalMoves(_globalState, new FieldState(_globalState.Selected, clickState));
                        ClearBoard();
                        UpdateBoardFromGlobalState();
                    }

                    
                }
                else
                {
                    if (_globalState.IsLegalMove(fieldGrid.Name))
                    {
                        //(var name, var extraFieldList) = _legalMoves.Find((a) => a.Item1 == fieldGrid.Name);
                        Pices killedPice = _globalState.DoMoveTo(fieldGrid.Name);
                        if (killedPice != Pices.Non) _killedPices.Add(killedPice);
                        //_legalMoves.Clear();
                        _globalState.Selected = null;
                        ClearBoard();
                        UpdateBoardFromGlobalState();
                    }
                    //

                }
                
            }
            else if(e.RightButton == MouseButtonState.Pressed)
            {
                //_legalMoves.Clear();
                _globalState.Selected = null;
                ClearBoard();
                UpdateBoardFromGlobalState();
            }
            
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

    public enum VisionMode
    {
        CurrentMove,
        White,
        Black
    }
}
