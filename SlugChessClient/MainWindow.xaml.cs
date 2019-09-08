using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
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

namespace SlugChess
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
        public static Image WhiteBishop { get { return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/WhiteBishop.png", UriKind.Absolute)) }; } }
        public static Image WhiteKing { get { return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/WhiteKing.png", UriKind.Absolute)) }; } }
        public static Image WhiteKnight { get { return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/WhiteKnight.png", UriKind.Absolute)) }; } }
        public static Image WhitePawn { get { return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/WhitePawn.png", UriKind.Absolute)) }; } }
        public static Image WhiteQueen { get { return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/WhiteQueen.png", UriKind.Absolute)) }; } }
        public static Image WhiteRook { get { return new Image { Source = new BitmapImage(new Uri("pack://siteoforigin:,,,/img/WhiteRook.png", UriKind.Absolute)) }; } }

        //
        public static Uri MoveSoundUri { get { return new Uri(".\\sound\\move.mp3", UriKind.Relative); } }


        public static MainWindow Instance { get; private set; }

        private GlobalState _globalState = new GlobalState();
        private List<Pices> _killedPices = new List<Pices>();
        private ClientIsPlayer _clientIsPlayer = ClientIsPlayer.Both;
        private ServerConnection _connection;
        private string _userToken;
        private string _matchToken;
        private Grpc.Core.AsyncDuplexStreamingCall<ChessCom.MovePacket, ChessCom.MoveResult> _matchStream;
        private Task _runnerTask;
        private MediaPlayer _mediaPlayer = new MediaPlayer();
        private string _lastMoveFrom;
        private string _lastMoveTo;

        public MainWindow()
        {
            InitializeComponent();
            DataContext = new MyLittleFuckingDataContext();
            var ver = FileVersionInfo.GetVersionInfo(Assembly.GetExecutingAssembly().Location);
            Title = $"Slug Chess v{ver.FileMajorPart}.{ver.FileMinorPart}.{ver.FileBuildPart}";
#if DEBUG
            Title += " Debug";
#endif
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
            //_globalState = GlobalState.CreateStartState(new VisionRules{ Enabled = false });
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

            Instance = this;
            //ServerConnection connection = new ServerConnection("hive.spaceslug.no", 43326);
            _connection = new ServerConnection("hive.spaceslug.no", 43326);
            try
            {
                _connection.Connect();
                messageBox.AppendText("\nConnected to SlugChessServer\n");
                _mediaPlayer.MediaFailed += (o, args) => {
                    int i = 5;
                };
                _mediaPlayer.Open(MoveSoundUri);

                //var a = _connection.Call.sendRequest(new ChessCom.MathRequest { A = 3, B = 4 });
                //connection.Call.
                //Console.WriteLine(a);
            }
            catch (AggregateException ex)
            {
                _connection = null;
                string popupText = "Slug Chess Connection failed";
                string textBoxText = "Can not connect to Slug Chess server. Please bother admin at admin@spaceslug.no. Continue to singleplayer?";
                MessageBoxButton button = MessageBoxButton.YesNo;
                MessageBoxImage icon = MessageBoxImage.Error;
                var result = MessageBox.Show(textBoxText, popupText, button, icon);
                if(result == MessageBoxResult.No) Application.Current.Shutdown();
            }
        }

        private void Window_Closing(object sender, CancelEventArgs args)
        {
            _matchStream?.RequestStream.WriteAsync(new ChessCom.MovePacket { CheatMatchEvent = ChessCom.MatchEvent.UnexpectedClosing, UserToken = _userToken, DoingMove=false});
            _runnerTask?.Wait();
            _connection?.ShutDownAsync();
        }

        public void Runner()
        {
            ChessCom.LookForMatchResult result = _connection.Call.LookForMatch(new ChessCom.UserIdentity { UserToken = _userToken });
            if (result.Succes)
            {
                Instance._matchToken = result.MatchToken;
                Instance.Dispatcher.Invoke(()=> {
                    StartMatch(result.IsWhitePlayer, result.MatchToken, result.Rules);
                });

                var matchStream = _connection.Call.Match();
                _matchStream = matchStream;
                //Open match stream call
                _matchStream.RequestStream.WriteAsync(new ChessCom.MovePacket { AskingForDraw = false, DoingMove = false, MatchToken = _matchToken, UserToken = _userToken });
                bool matchEnded = false;
                while (!matchEnded)
                {
                    if (!matchStream.ResponseStream.MoveNext().Result)
                    {
                        matchEnded = true;
                        continue;
                    }
                    ChessCom.MoveResult move = matchStream.ResponseStream.Current;

                    //if (move.)
                    //{

                    //}
                    if (move.MoveHappned)
                    {
                        Instance.Dispatcher.Invoke(() => {
                            messageBox.AppendText("Opponent did move!\n");
                            messageBox.CaretPosition = messageBox.CaretPosition.DocumentEnd;
                            messageBox.BringIntoView();
                            messageBox.Focus();
                            _globalState.Selected = move.Move.From;
                            Pices killedPice = _globalState.DoMoveTo(move.Move.To);
                            if (killedPice != Pices.Non) _killedPices.Add(killedPice);
                            _globalState.Selected = null;
                            _lastMoveFrom = _globalState.CanSeeField(_clientIsPlayer, move.Move.From)? move.Move.From:"";
                            _lastMoveTo = _globalState.CanSeeField(_clientIsPlayer, move.Move.To)? move.Move.To:"";
                            ClearBoard();
                            UpdateBoardFromGlobalState();
                            if(move.MatchEvent == ChessCom.MatchEvent.WhiteWin || move.MatchEvent == ChessCom.MatchEvent.WhiteWin)
                            {
                                EndOfMatch(move.MatchEvent);
                                matchEnded = true;
                            }
                            else
                            {
                                _mediaPlayer.Stop();
                                _mediaPlayer.Play();
                            }

                        });

                    }
                    else if(move.OpponentAskingForDraw)
                    {
                        Instance.Dispatcher.Invoke(() =>
                        {
                            string popupText = "Draw?";
                            string textBoxText = "Opponent is asking for draw. Do you accept?";
                            MessageBoxButton button = MessageBoxButton.YesNo;
                            MessageBoxImage icon = MessageBoxImage.Error;
                            var drawResult = MessageBox.Show(textBoxText, popupText, button, icon);
                            if (drawResult == MessageBoxResult.Yes)
                            {
                                _matchStream.RequestStream.WriteAsync(new ChessCom.MovePacket { AskingForDraw = true });
                                matchEnded = true;
                            }
                        });
                    }
                    else if (move.MatchEvent == ChessCom.MatchEvent.UnexpectedClosing)
                    {
                        Instance.Dispatcher.Invoke(() =>
                        {
                            string popupText = "UnexpextedClosing";
                            string textBoxText = "Opponents client unexpectedly closed";
                            messageBox.AppendText(textBoxText + "\n");
                            MessageBoxButton button = MessageBoxButton.OK;
                            MessageBoxImage icon = MessageBoxImage.Exclamation;
                            var drawResult = MessageBox.Show(textBoxText, popupText, button, icon);
                            if (drawResult == MessageBoxResult.Yes)
                            {
                                _matchStream.RequestStream.WriteAsync(new ChessCom.MovePacket { AskingForDraw = true });
                                matchEnded = true;
                            }
                        });
                    }
                }
                _matchStream.RequestStream.CompleteAsync();
                _matchStream = null;
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

        public void StartMatch(bool isWhitePlayer, string matchToken, ChessCom.VisionRules rules)
        {
            messageBox.AppendText("Starting match: " + matchToken +  ", player is " + (isWhitePlayer?"white":"black") + "\n");
            _clientIsPlayer = isWhitePlayer ? ClientIsPlayer.White : ClientIsPlayer.Black;
            ClearBoard();
            var vr = new VisionRules
            {
                Enabled = rules.Enabled,
                ViewMoveFields = rules.ViewMoveFields,
                ViewRange = rules.ViewRange,
                PiceOverwrite = new Dictionary<Pices, VisionRules>()


            };
            foreach(var keyValOR in rules.PiceOverwriter)
            {
                vr.PiceOverwrite.Add((Pices)keyValOR.Key, new VisionRules {Enabled = keyValOR.Value.Enabled,ViewMoveFields = keyValOR.Value.ViewMoveFields,ViewRange = keyValOR.Value.ViewRange});
            }

            _globalState = GlobalState.CreateStartState(vr);
            UpdateBoardFromGlobalState();
        }

        private void EndOfMatch(ChessCom.MatchEvent result)
        {
            if(result == ChessCom.MatchEvent.Draw)
            {

            }
            else if((result == ChessCom.MatchEvent.WhiteWin && _clientIsPlayer == ClientIsPlayer.White) || (result == ChessCom.MatchEvent.BlackWin && _clientIsPlayer == ClientIsPlayer.Black))
            {
                messageBox.AppendText("Congratulations you won!!" + "\n");
            }
            else
            {
                messageBox.AppendText("You lost, stupid idiot." + "\n");
            }
            lookForMatchButton.IsEnabled = true;
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
            string blackKingField = _globalState.GetWhiteKingPos();
            string whiteKingField = _globalState.GetBlackKingPos();

            foreach (Grid child in this.BoardGrid.Children)
            {
                ApplyFieldStateToGrid(child, _globalState.GetFieldAt(child.Name));
                if (_globalState.CanSeeField(_clientIsPlayer, child.Name))
                {
                    child.Opacity = 1;
                    foreach (UIElement underChiled in child.Children)
                    {
                        underChiled.Visibility = Visibility.Visible;
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
                    else if (_lastMoveFrom == child.Name || _lastMoveTo == child.Name)
                    {
                        var border = new Border();
                        border.BorderBrush = Brushes.SeaGreen;
                        border.BorderThickness = new Thickness(3, 3, 3, 3); //You can specify here which borders do you want
                        child.Children.Add(border);
                    }
                     
                    if (_globalState.GetLegalMovesFromField(child.Name).Contains(whiteKingField) && _globalState.CanSeeField(_clientIsPlayer, whiteKingField))
                    {
                        //messageBox.AppendText("YGetLegalMovesFromField" + "\n");
                        var border = new Border();
                        border.BorderBrush = Brushes.Gold;
                        border.BorderThickness = new Thickness(3, 3, 3, 3); //You can specify here which borders do you want
                        child.Children.Add(border);
                        Grid kingGrid = (Grid)BoardGrid.FindName(whiteKingField);
                        border = new Border();
                        border.BorderBrush = Brushes.Gold;
                        border.BorderThickness = new Thickness(3, 3, 3, 3); //You can specify here which borders do you want
                        kingGrid.Children.Add(border);
                    }
                    else if (_globalState.GetLegalMovesFromField(child.Name).Contains(blackKingField) && _globalState.CanSeeField(_clientIsPlayer, blackKingField))
                    {
                        //messageBox.AppendText("YGetLegalMovesFromField" + "\n");
                        var border = new Border();
                        border.BorderBrush = Brushes.Gold;
                        border.BorderThickness = new Thickness(3, 3, 3, 3); //You can specify here which borders do you want
                        child.Children.Add(border);
                        Grid kingGrid = (Grid)BoardGrid.FindName(blackKingField);
                        border = new Border();
                        border.BorderBrush = Brushes.Gold;
                        border.BorderThickness = new Thickness(3, 3, 3, 3); //You can specify here which borders do you want
                        kingGrid.Children.Add(border);
                    }

                }
                else
                {
                    child.Visibility = Visibility.Visible;
                    child.Opacity = 0.2;
                    foreach (UIElement underChiled in child.Children)
                    {
                        underChiled.Visibility = Visibility.Hidden;
                    }
                }
               
            }
            _killedPices.Sort();
            foreach (var pice in _killedPices)
            {
                AddPiceToGrid(KilledPicesGrid, pice);
            }
        }

        private void LoginButtonClick(object sender, RoutedEventArgs args)
        {
            string name = nameTextBox.Text;
            var result = _connection.Call.Login(new ChessCom.LoginForm { Username = name });
            if (result.SuccessfullLogin)
            {
                messageBox.AppendText("Logged in as " + name + "\n");
                _userToken = result.UserToken;
                loginButton.Content = "U logged in";
                loginButton.IsEnabled = false;
                lookForMatchButton.IsEnabled = true;
            }

        }

        private void LookForMatchClick(object sender, RoutedEventArgs args)
        {
            _runnerTask = Task.Run(() => { Runner(); });
            ((Button)sender).IsEnabled = false;
            //ChessCom.LookForMatchResult result = _connection.Call.LookForMatch(new ChessCom.UserIdentity {UserToken =  _userToken});
            //if (result.Succes)
            //{
            //    //_matchToken = result.MatchToken;
            //    var match = _connection.Call.Match();

            //    _matchMoveSteam = match.RequestStream;

            //    StartMatch(result.IsWhitePlayer, result.MatchToken);
            //    HandleMovesLoop(match);
            //}

        }

        //private void HandleMovesLoop(Grpc.Core.AsyncDuplexStreamingCall<ChessCom.MovePacket, ChessCom.MoveResult> match)
        //{
        //    while (match.ResponseStream.MoveNext().Result)
        //    {

        //    }
        //}

        private void Field_PreviewMouseDown(object sender, MouseButtonEventArgs e)
        {
            //sender
            if(e.LeftButton == MouseButtonState.Pressed)
            {
                Grid fieldGrid = (Grid)sender;
                Field clickState = _globalState.GetFieldAt(fieldGrid.Name);
                //Early escape not player pice
                if (_globalState.Selected == null)
                {
                    if (_globalState.WhiteTurn ? _clientIsPlayer == ClientIsPlayer.Black : _clientIsPlayer == ClientIsPlayer.White) return;
                    if ((_globalState.WhiteTurn && Field.HasWhitePice(clickState)) || (!_globalState.WhiteTurn && Field.HasBlackPice(clickState)))
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
                        messageBox.AppendText("I did move!\n");
                        Pices killedPice = _globalState.DoMoveTo(fieldGrid.Name);
                        if (killedPice != Pices.Non) _killedPices.Add(killedPice);
                        ChessCom.MatchEvent matchEvent = ChessCom.MatchEvent.Non;
                        if (killedPice == Pices.WhiteKing) { matchEvent = ChessCom.MatchEvent.BlackWin; EndOfMatch(matchEvent); }
                        if (killedPice == Pices.BlackKing) { matchEvent = ChessCom.MatchEvent.WhiteWin; EndOfMatch(matchEvent); }

                        if (_matchStream?.RequestStream != null)
                        {
                            _matchStream.RequestStream.WriteAsync(new ChessCom.MovePacket
                            {
                                AskingForDraw = false,
                                CheatMatchEvent = matchEvent,
                                DoingMove = true,
                                MatchToken = _matchToken,
                                UserToken = _userToken,
                                Move = new ChessCom.Move { From = _globalState.Selected, To = fieldGrid.Name},
                                
                            });
                        }
                        
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

        private void AddPiceToGrid(UniformGrid grid, Pices pice)
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

    public enum ClientIsPlayer
    {
        Both,
        White,
        Black
    }
}
