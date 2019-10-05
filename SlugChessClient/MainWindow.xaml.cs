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
        public static Uri MoveSoundUri { get { return new Uri(".\\sound\\move.wav", UriKind.Relative); } }
        public static Uri MatchStartSoundUri { get { return new Uri(".\\sound\\match_start.wav", UriKind.Relative); } }


        public static MainWindow Instance { get; private set; }

        private GlobalState _globalState = new GlobalState();
        private List<Pices> _killedPices = new List<Pices>();
        private ClientIsPlayer _clientIsPlayer = ClientIsPlayer.Both;
        private ServerConnection _connection;
        private string _username;
        private string _userToken;
        private string _matchToken;
        private string _opponentUsername;
        private Grpc.Core.AsyncDuplexStreamingCall<ChessCom.MovePacket, ChessCom.MoveResult> _matchStream;
        private Grpc.Core.AsyncDuplexStreamingCall<ChessCom.ChatMessage, ChessCom.ChatMessage> _matchMessageStream;
        private Task _runnerTask;
        private MediaPlayer _mediaPlayer = new MediaPlayer();
        private string _lastMoveFrom;
        private string _lastMoveTo;
        private bool _isSingelplayer = true;
        private ChessCom.Move _myLastMove;

        public MainWindow()
        {
            InitializeComponent();
            DataContext = new MyLittleFuckingDataContext();
            var ver = FileVersionInfo.GetVersionInfo(Assembly.GetExecutingAssembly().Location);
            Title = $"Slug Chess v{ver.FileMajorPart}.{ver.FileMinorPart}.{ver.FileBuildPart}";
#if DEBUG
            Title += $" Debug";
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
                    [Pices.BlackKing] = new VisionRules { ViewRange = 2 },
                    [Pices.WhiteKing] = new VisionRules { ViewRange = 2 },
                }
            });
            UpdateBoardFromGlobalState();
            //loginButton.Visibility = Visibility.Hidden;
            var rand = new Random();
            nameTextBox.Text = "default" + rand.Next(0, 99999).ToString();

            Instance = this;
            //ServerConnection connection = new ServerConnection("hive.spaceslug.no", 43326);
#if DEBUG
            int port = 43326;
#else
            int port = 43327;
#endif
            messageBox.Document.Blocks.Clear();
            _connection = new ServerConnection("hive.spaceslug.no", port);
            try
            {
                _connection.Connect();
                WriteTextNonInvoke("Connected to SlugChessServer\n");
                _mediaPlayer.MediaFailed += (o, args) => {
                    int i = 5;
                };
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
            _matchStream?.RequestStream.WriteAsync(new ChessCom.MovePacket { CheatMatchEvent = ChessCom.MatchEvent.UnexpectedClosing, UserToken = _userToken, DoingMove=false}).Wait();
            _matchStream?.RequestStream.CompleteAsync();
            // -- Might need an abort message to avoid exeptions on the server
            //_matchMessageStream?.RequestStream.WriteAsync(new ChessCom.ChatMessage
            //{
            //    UserToken = _userToken,
            //    Reciver = "system",
            //    Sender = _username,
            //    Message = "exit"
            //}).Wait();
            _matchMessageStream?.RequestStream.CompleteAsync();
        }

        public void Runner()
        {
            ChessCom.LookForMatchResult result = _connection.Call.LookForMatch(new ChessCom.UserIdentity { UserToken = _userToken });
            if (result.Succes)
            {
                _isSingelplayer = false;
                Instance._matchToken = result.MatchToken;
                Instance._opponentUsername = result.OpponentUsername;
                Instance.Dispatcher.Invoke(()=> {
                    _mediaPlayer.Stop();
                    _mediaPlayer.Open(MatchStartSoundUri);
                    _mediaPlayer.Play();
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
                    }
                    if (matchEnded) continue;
                    ChessCom.MoveResult move = matchStream.ResponseStream.Current;

                    //if (move.)
                    //{

                    //}
                    if (move.MoveHappned)
                    {
                        if (move.Move.From != _myLastMove.From || move.Move.To != _myLastMove.To)
                        {
                            Instance.Dispatcher.Invoke(() => {
                                WriteTextNonInvoke("Opponent did move!");
                                //messageBox.AppendText("Opponent did move!\n");
                                //messageBox.CaretPosition = messageBox.CaretPosition.DocumentEnd;
                                //messageBox.BringIntoView();
                                //messageBox.Focus();
                                _globalState.Selected = move.Move.From;
                                Pices killedPice = _globalState.DoMoveTo(move.Move.To);
                                if (killedPice != Pices.Non) _killedPices.Add(killedPice);
                                _globalState.Selected = null;
                                _lastMoveFrom = _globalState.CanSeeField(_clientIsPlayer, move.Move.From) ? move.Move.From : "";
                                _lastMoveTo = _globalState.CanSeeField(_clientIsPlayer, move.Move.To) ? move.Move.To : "";
                                ClearBoard();
                                UpdateBoardFromGlobalState();

                                _mediaPlayer.Stop();
                                _mediaPlayer.Open(MoveSoundUri);
                                _mediaPlayer.Play();


                            });
                        } 
                    }
                    if(move.OpponentAskingForDraw)
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
                            WriteTextNonInvoke(textBoxText);
                            MessageBoxButton button = MessageBoxButton.OK;
                            MessageBoxImage icon = MessageBoxImage.Exclamation;
                            MessageBox.Show(textBoxText, popupText, button, icon);
                            EndOfMatch(_clientIsPlayer==ClientIsPlayer.White?ChessCom.MatchEvent.WhiteWin: ChessCom.MatchEvent.BlackWin);
                            matchEnded = true;
                        });
                    }
                    else if (move.MatchEvent == ChessCom.MatchEvent.WhiteWin || move.MatchEvent == ChessCom.MatchEvent.BlackWin)
                    {
                        Instance.Dispatcher.Invoke(() =>
                        {
                            EndOfMatch(move.MatchEvent);
                            matchEnded = true;
                        });
                        _matchStream.RequestStream.WriteAsync(new ChessCom.MovePacket { CheatMatchEvent = ChessCom.MatchEvent.ExpectedClosing, AskingForDraw = false, DoingMove = false, MatchToken = _matchToken, UserToken = _userToken }).Wait();
                        _matchStream.RequestStream.CompleteAsync();

                    }
                }
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
            WriteTextNonInvoke("Starting match: " + matchToken +  ", you are " + (isWhitePlayer?"white":"black"));
            WriteTextNonInvoke("Opponent username: " + _opponentUsername);
            _clientIsPlayer = isWhitePlayer ? ClientIsPlayer.White : ClientIsPlayer.Black;
            _myLastMove = new ChessCom.Move { From = "", To = "" };
            _killedPices.Clear();
            ClearBoard();
            var vr = new VisionRules
            {
                Enabled = rules.Enabled,
                ViewMoveFields = rules.ViewMoveFields,
                ViewRange = rules.ViewRange,
                ViewCaptureField = rules.ViewCaptureField,
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
                WriteTextNonInvoke("Congratulations you won!!" + "\n");
            }
            else
            {
                WriteTextNonInvoke("You lost, stupid idiot." + "\n");
            }
            _clientIsPlayer = ClientIsPlayer.Both;
            _globalState.VisionRules.Enabled = false;
            _globalState.CalculateVision();
            ClearBoard();
            UpdateBoardFromGlobalState();
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

        public void BlackOutField(string fieldName)
        {
            Grid field = (Grid)this.BoardGrid.FindName(fieldName);
            field.Visibility = Visibility.Hidden;
        }

        public void WriteTextInvoke(string text, string sender = null)
        {
            Instance.Dispatcher.Invoke(() =>
            {
                WriteTextNonInvoke(text, sender);
            });
        }

        private void LoginButtonClick(object sender, RoutedEventArgs args)
        {
            _username = nameTextBox.Text;
            var ver = FileVersionInfo.GetVersionInfo(Assembly.GetExecutingAssembly().Location);
            Title = $"Slug Chess v{ver.FileMajorPart}.{ver.FileMinorPart}.{ver.FileBuildPart}";
            var result = _connection.Call.Login(new ChessCom.LoginForm { Username = _username, MajorVersion = ver.FileMajorPart.ToString(), MinorVersion = ver.FileMinorPart.ToString(), BuildVersion = ver.FileBuildPart.ToString() });
            if (result.SuccessfullLogin)
            {
                WriteTextNonInvoke("Logged in as " + _username);
                if(result.LoginMessage != "") WriteTextNonInvoke(result.LoginMessage);
                _userToken = result.UserToken;
                //loginButton.Content = "U logged in";
                loginButton.IsEnabled = false;
                lookForMatchButton.IsEnabled = true;
                tbLoginStatus.Text = $"{_username}";

                _matchMessageStream = _connection.Call.ChatMessageStream();
                _matchMessageStream.RequestStream.WriteAsync(new ChessCom.ChatMessage {
                    UserToken = _userToken,
                    Reciver = "system",
                    Sender = _username,
                    Message = "init"
                });
                Task.Run(() => MessageCallRunner());
                //TODO recive message callback
                //TODO handle shutdown of message
            }
            else
            {
                WriteTextNonInvoke("Login failed. " + result.LoginMessage);
            }

        }

        private void MessageCallRunner()
        {
            while (true)
            {
                if (!_matchMessageStream.ResponseStream.MoveNext().Result)
                {
                    break;
                }
                ChessCom.ChatMessage chatMessage = _matchMessageStream.ResponseStream.Current;

                WriteTextInvoke(chatMessage.Message, chatMessage.Sender);
            }
        }

        private void WriteTextNonInvoke(string text, string sender = null)
        {
            messageBox.AppendText($"{DateTime.Now.ToString("HH:mm:ss")} {(sender!=null?"- "+sender:"")}: {text}\r\n");
            messageBox.CaretPosition = messageBox.CaretPosition.DocumentEnd;
            messageBox.BringIntoView();
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

        private void SendMessageClick(object sender, RoutedEventArgs args)
        {
            if (_opponentUsername == null) return;
            _matchMessageStream?.RequestStream.WriteAsync(new ChessCom.ChatMessage { Sender=_username, UserToken=_userToken, Reciver=_opponentUsername, Message= sendTextBox.Text});
            WriteTextNonInvoke(sendTextBox.Text, _username);
            sendTextBox.Text = "";

        }

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
                        WriteTextNonInvoke("I did move!");
                        Pices killedPice = _globalState.DoMoveTo(fieldGrid.Name);
                        if (killedPice != Pices.Non) _killedPices.Add(killedPice);
                        ChessCom.MatchEvent matchEvent = ChessCom.MatchEvent.Non;
                        if (killedPice == Pices.WhiteKing) { matchEvent = ChessCom.MatchEvent.BlackWin;  }
                        if (killedPice == Pices.BlackKing) { matchEvent = ChessCom.MatchEvent.WhiteWin;  }

                        if (_matchStream?.RequestStream != null)
                        {
                            _myLastMove = new ChessCom.Move { From = _globalState.Selected, To = fieldGrid.Name };
                            _matchStream.RequestStream.WriteAsync(new ChessCom.MovePacket
                            {
                                AskingForDraw = false,
                                CheatMatchEvent = matchEvent,
                                DoingMove = true,
                                MatchToken = _matchToken,
                                UserToken = _userToken,
                                Move = _myLastMove,
                                
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

        private void SendTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if(e.Key == Key.Enter)
            {
                SendMessageClick(sender, null);
            }
        }
    }

    public enum ClientIsPlayer
    {
        Both,
        White,
        Black
    }
}
