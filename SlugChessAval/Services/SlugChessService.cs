using Avalonia.Collections;
using Avalonia.Threading;
using ChessCom;
using Google.Protobuf.Collections;
using Grpc.Core;
using ReactiveUI;
using SlugChessAval.ViewModels;
using SlugChessAval.Views;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Reactive.Linq;
using System.Reactive.Subjects;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Security;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SlugChessAval.Services
{
    public class SlugChessService : INotifyPropertyChanged
    {
        #region init stuff
        public static SlugChessService Client => _client ?? throw new Exception("Asking for client when client is null");
        private static SlugChessService? _client = null;
        public static string Usertoken => Client?.UserData?.Usertoken ?? "";
        public static void Instanciate(string adress, int port)
        {
            if (_client != null) {
                Console.WriteLine("Allready initialized ChessCom Client. Starting new");
                _client._channel?.ShutdownAsync().Wait();
                _client.ConnectionAlive = false;
            }
            
            _client = new SlugChessService(adress, port);
            ////Task.Run(ChannelStateListner);
            _client._channel.ConnectAsync(DateTime.UtcNow + TimeSpan.FromSeconds(3)).ContinueWith(x =>
            {
                if (x.IsCompletedSuccessfully)
                {
                    _client.ConnectionAlive = true;
                }
                else
                {

                }
            });
        }
        public static void DeInstansiate()
        {
            throw new NotImplementedException();

        }
        //public object temp;
        private SlugChessService(string adress, int port)
        {
            _channel = new Channel(adress, port, ChannelCredentials.Insecure);
            Call = new ChessCom.ChessCom.ChessComClient(_channel);
            _heartbeatTimer.AutoReset = true;
            _heartbeatTimer.Elapsed += (o, e) => 
            {
                try
                {
                    if (!Call.Alive(new Heartbeat { Alive = true, Usertoken = UserData?.Usertoken ?? "" }).Alive) 
                    { 
                        UserLoggedIn.OnNext(false); 
                        UserData = new UserData(); 
                    }

                }
                catch(Grpc.Core.RpcException ex)
                {
                    // EX here means Heartbeat failed for some reason.
                    throw ex;
                }
            };
            UserLoggedIn.Subscribe(loggedIn => { 
                if (loggedIn) { 
                    _heartbeatTimer.Start();
                    Task.Run(() =>
                    {
                        var stream = Call.ChatMessageListener(UserData);
                        while (stream.ResponseStream.MoveNext().Result) // Gets inner exeption with status code 'Unavailable' when server closes with players logged in
                        {
                            //Send the message to UI thread to avoid weird race conditions. Rx should be single threaded.
                            Dispatcher.UIThread.InvokeAsync(() =>
                            {
                                MessageToLocal(stream.ResponseStream.Current.Message, stream.ResponseStream.Current.SenderUsername);
                            }).Wait(); //Wait to process one before starting to process the next
                        }
                        stream.Dispose();
                    });
                } else {
                    _heartbeatTimer.Stop();
                } 
            });

        }
        #endregion

        private Channel _channel;
        public ChessCom.ChessCom.ChessComClient Call { get; private set;} 

        public event PropertyChangedEventHandler? PropertyChanged;

        // This method is called by the Set accessor of each property.  
        // The CallerMemberName attribute that is applied to the optional propertyName  
        // parameter causes the property name of the caller to be substituted as an argument.  
        private void NotifyPropertyChanged([CallerMemberName] String propertyName = "")
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        public bool ConnectionAlive
        {
            get {return _connectionAlive; }
            set {if (value != _connectionAlive){_connectionAlive = value; NotifyPropertyChanged();} }
        }
        private bool _connectionAlive = false;

        //public AvaloniaList<KeyValuePair<string, VisionRules>> ServerVisionRuleset { get; } = new AvaloniaList<KeyValuePair<string, VisionRules>>();
        public AvaloniaList<string> ServerNamedVariants { get; } = new AvaloniaList<string>();
        //private AvaloniaList<KeyValuePair<string, VisionRules>> _serverVisionRulesetTemp = new AvaloniaList<KeyValuePair<string, VisionRules>>();

        public Subject<bool> UserLoggedIn { get; set; } = new Subject<bool>();

        public void MessageToLocal(string message, string sender) => _messages.OnNext(
            DateTime.Now.ToString("HH:mm:ss") + " " + sender + ": " + message);
        public IObservable<string> Messages => _messages;
        private ReplaySubject<string> _messages = new ReplaySubject<string>();
        public UserData UserData
        {
            get { return _userData; }
            set { if (value != _userData) { _userData = value; NotifyPropertyChanged(); } }
        }
        private UserData _userData = new UserData();

        private System.Timers.Timer _heartbeatTimer = new System.Timers.Timer(60*1000);

        public void GetNewUserdata()
        {
            if(_client == null)return;
            var userId = new UserIdentification { Usertoken = UserData.Usertoken, Secret = "????" };
            var req = new UserDataRequest { UserIdent = userId, Username = UserData.Username };
            var ud = _client.Call.GetPublicUserdata(req);
            //WAAAAA. Make sure not to overwrite the usertoken
            UserData = new UserData { Username=UserData.Username, Usertoken = UserData.Usertoken, Elo = ud.Elo };
        }

        public Task<LoginResult> LoginInUserAsync(string username, string password) => Task.Run<LoginResult>(() => 
            {
                if(_client == null) return new LoginResult{ SuccessfullLogin=false, LoginMessage="Could not login as not connected to SlugChess server"};

                var ver = FileVersionInfo.GetVersionInfo(Assembly.GetExecutingAssembly().Location);
                try
                {
                    var result = _client.Call.Login(new LoginForm { Username = username, Password = password, MajorVersion = ver.FileMajorPart.ToString(), MinorVersion = ver.FileMinorPart.ToString(), BuildVersion = ver.FileBuildPart.ToString() });
                    if (result.SuccessfullLogin)
                    {
                        UserData = new UserData
                        {
                            Username = result.UserData.Username,
                            Usertoken = result.UserData.Usertoken,
                            Elo = result.UserData.Elo
                        };
                        UserLoggedIn.OnNext(true);
                        //Call.ServerVisionRulesetsAsync(new ChessCom.Void()).ResponseAsync.ContinueWith(y =>
                        //{
                        //    ServerVisionRuleset.Clear();
                        //    var a = y.Result.VisionRulesets.ToList();
                            //a.Insert(0, new KeyValuePair<string, VisionRules>("No Vision Rules", new VisionRules { Enabled = false }));
                        //    ServerVisionRuleset.AddRange(a);
                        //});
                        Call.GetNamedVariantsAsync(new ChessCom.Void()).ResponseAsync.ContinueWith(y =>
                        {
                            ServerNamedVariants.Clear();
                            ServerNamedVariants.AddRange(y.Result.Variants);
                        });

                        return result;
                    }
                    else
                    {
                        //TODO: make Serilog work -> Serilog.Log.Information("Login failed. " + result.LoginMessage);
                        MainWindowViewModel.SendNotification("Login attempt rejected from server");
                        return result;
                    }
                }
                catch(RpcException ex)
                {
                    if(ex.StatusCode == StatusCode.Unavailable)
                    {
                        Console.WriteLine("SlugChess Server Unavailable. " + ex.Message);
                        MainWindowViewModel.SendNotification("SlugChessServer Unavailable");
                        return new LoginResult
                        {
                            SuccessfullLogin = false,
                            LoginMessage = "SlugChess Server unavailable. The server might be down or your internet connection interupted\n" +
                            "Press login button to try again or complain to the SlugChess Server admin at admin@spaceslug.no"
                        };
                    }   
                    else 
                    {
                        throw ex;
                    }
                }
                
            });
        public Task Logout() => Task.Run(() =>
        {
            if (ConnectionAlive)
            {
                var result = Call.Logout(new UserIdentification{ Usertoken = UserData?.Usertoken ?? ""});
                if (!result.SuccessfullLogout)
                {
                    MainWindowViewModel.SendNotification("Malformed logout: " + result.LogoutMessage);
                }
                //TODO: Write in logfile the message if there is one -> if(result.LogoutMessage != "")

                UserLoggedIn.OnNext(false);
                UserData = new UserData();
            }
        });

        public IObservable<MoveResult> GetMatchListener(string matchId)
        {
            var subject = new Subject<MoveResult>();
            //TODO add exeption handleling to OnError the subject and allow subscribers to do well formed closing. OnError also if stream ends before end of match event
            Task.Run(() =>
            {
                var stream = Call.MatchEventListener(new MatchObserver { MatchId = matchId, Usertoken = UserData?.Usertoken ?? ""});
                while (stream.ResponseStream.MoveNext().Result)
                {
                    subject.OnNext(stream.ResponseStream.Current);
                    //Close stream if end of match event. Nooo don't. Server  
                    if (stream.ResponseStream.Current.MatchEvent switch
                    {
                        MatchEvent.Draw => true,
                        MatchEvent.UnexpectedClosing => true,
                        MatchEvent.ExpectedClosing => true,
                        _ => false,
                    }) break;
                }
                stream.Dispose();
                subject.OnCompleted();

            });
            return subject;
        }

    }
        
}
