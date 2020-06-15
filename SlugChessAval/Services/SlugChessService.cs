using ChessCom;
using Grpc.Core;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Reactive.Subjects;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace SlugChessAval.Services
{
    public class SlugChessService : INotifyPropertyChanged
    {
        #region init stuff
        public static SlugChessService Client { get; private set; } = new SlugChessService("localhost", 9999);
        public static string Usertoken => Client.UserData?.Usertoken ?? "";
        public static void Instanciate(string adress, int port)
        {
            //if (Client != null) throw new Exception("Allready initialized");
            Client._channel.ShutdownAsync().Wait();
            Client.ConnectionAlive = false;
            Client = new SlugChessService(adress, port);
            ////Task.Run(ChannelStateListner);
            Client._channel.ConnectAsync(DateTime.UtcNow + TimeSpan.FromSeconds(3)).ContinueWith(x =>
            {
                if (x.IsCompletedSuccessfully)
                {
                    Client.ConnectionAlive = true;
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
        private SlugChessService(string adress, int port)
        {
            _channel = new Channel(adress, port, ChannelCredentials.Insecure);
            Call = new ChessCom.ChessCom.ChessComClient(_channel);
            _heartbeatTimer.AutoReset = true;
            _heartbeatTimer.Elapsed += (o, e) => { if (!Call.Alive(new Heartbeat { Alive = true, Usertoken = UserData?.Usertoken ?? "" }).Alive) { UserLoggedIn.OnNext(false); UserData = null; } };
            UserLoggedIn.Subscribe(loggedIn => { 
                if (loggedIn) { 
                    _heartbeatTimer.Start();
                    Task.Run(() =>
                    {
                        var stream = Call.ChatMessageListener(UserData);
                        while (stream.ResponseStream.MoveNext().Result)
                        {
                            //while (_messages.HasObservers == false) Thread.Sleep(500);
                            MessageToLocal(stream.ResponseStream.Current.Message, stream.ResponseStream.Current.SenderUsername);
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

        public Subject<bool> UserLoggedIn { get; set; } = new Subject<bool>();

        public void MessageToLocal(string message, string sender) => _messages.OnNext(
            DateTime.Now.ToString("HH:mm:ss") + " " + sender + ": " + message);
        public IObservable<string> Messages => _messages;
        private ReplaySubject<string> _messages = new ReplaySubject<string>();
        public UserData? UserData
        {
            get { return _userData; }
            set { if (value != _userData) { _userData = value; NotifyPropertyChanged(); } }
        }
        private UserData? _userData = null;

        private System.Timers.Timer _heartbeatTimer = new System.Timers.Timer(60*1000);


        public Task<LoginResult> LoginInUserAsync(string username, string password) => Task.Run<LoginResult>(() => 
            {
                var ver = FileVersionInfo.GetVersionInfo(Assembly.GetExecutingAssembly().Location);
                var result = Client.Call.Login(new LoginForm { Username = username, MajorVersion = ver.FileMajorPart.ToString(), MinorVersion = ver.FileMinorPart.ToString(), BuildVersion = ver.FileBuildPart.ToString() });
                if (result.SuccessfullLogin)
                {
                    UserData = new UserData
                    {
                        Username = result.UserData.Username,
                        Usertoken = result.UserData.Usertoken,
                        Elo = result.UserData.Elo
                    };
                    UserLoggedIn.OnNext(true);


                    //foreach (var item in ((MenuItem)TopMenu.Items[0]).Items)
                    //{
                    //    if (item is MenuItem menuitem)
                    //    {
                    //        if ((string)menuitem.Header != "_Host" || (string)menuitem.Header != "_Browse Games") menuitem.IsEnabled = true;
                    //    }
                    //}
                    //tbLoginStatus.Text = $"{_userdata.Username}";

                    //_matchMessageStream = _connection.Call.ChatMessageStream();
                    //_matchMessageStream.RequestStream.WriteAsync(new ChessCom.ChatMessage
                    //{
                    //    SenderUsertoken = _userdata.Usertoken,
                    //    ReciverUsertoken = "system",
                    //    SenderUsername = _userdata.Username,
                    //    Message = "init"
                    //});
                    //Task.Run(() => MessageCallRunner());
                    //_heartbeatTimer.Elapsed += (obj, e) =>
                    //{
                    //    _connection?.Call.Alive(new ChessCom.Heartbeat { Alive = true });
                    //};
                    ////TODO recive message callback
                    ////TODO handle shutdown of message
                    return result;
                }
                else
                {
                    Serilog.Log.Information("Login failed. " + result.LoginMessage);
                    return result;
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
