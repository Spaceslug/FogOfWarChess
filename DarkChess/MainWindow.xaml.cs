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

        public static Image BlackBishop
        {
            get
            {
                return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/BlackBishop.png", UriKind.Absolute)) };
            }
        }
        public static Image BlackKing
        {
            get
            {
                return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/BlackKing.png", UriKind.Absolute)) };
            }
        }
        public static Image BlackKnight
        {
            get
            {
                return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/BlackKnight.png", UriKind.Absolute)) };
            }
        }
        public static Image BlackPawn
        {
            get
            {
                return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/BlackPawn.png", UriKind.Absolute)) };
            }
        }
        public static Image BlackQueen
        {
            get
            {
                return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/BlackQueen.png", UriKind.Absolute)) };
            }
        }
        public static Image BlackRook
        {
            get
            {
                return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/BlackRook.png", UriKind.Absolute)) };
            }
        }
        public static Image WhiteBishop
        {
            get
            {
                return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/WhiteBishop.png", UriKind.Absolute)) };
            }
        }
        public static Image WhiteKing
        {
            get
            {
                return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/WhiteKing.png", UriKind.Absolute)) };
            }
        }
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

        public GlobalState GlobalState = new GlobalState();

        public MainWindow()
        {
            InitializeComponent();
            DataContext = new MyLittleFuckingDataContext();
            
            Uri pageUri = new Uri("pack://siteoforigin:,,,/SiteOfOriginFile.xaml", UriKind.Absolute);
            this.a6.Children.Add(new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/BlackKing.png", UriKind.Absolute)) });
            this.a8.Children.Add(new Image { Source = new BitmapImage(new Uri("img\\BlackKing.png", UriKind.RelativeOrAbsolute)) });
            string d = System.AppDomain.CurrentDomain.BaseDirectory;
            this.a2.Children.Add(new Image { Source = new BitmapImage(new Uri("BlackPawn.png", UriKind.RelativeOrAbsolute)) });
            Grid b8 = (Grid)this.BoardGrid.FindName("b8");
            b8.Children.Clear();
            b8.Children.Add(BlackPawn);
            BlackOutField("d4");
            ClearBoard();
            GlobalState.UpdateState(DarkChess.GlobalState.CreateStartState());
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
        }

        public void UpdateBoardFromGlobalState()
        {
            foreach (Grid child in this.BoardGrid.Children)
            {
                ApplyFieldStateToGrid(child, GlobalState.Board[BoardPosToIndex[child.Name]]);
            }
        }

        private void ApplyFieldStateToGrid(Grid grid, FieldState fieldState)
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

        public void BlackOutField(string fieldName)
        {
            Grid field = (Grid)this.BoardGrid.FindName(fieldName);
            field.Visibility = Visibility.Hidden;
        }
    }
}
