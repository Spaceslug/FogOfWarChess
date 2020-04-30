﻿using ChessCom;
using Grpc.Core;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Reactive.Subjects;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace SlugChessAval.Services
{
    public class SlugChessService : INotifyPropertyChanged
    {
        #region init stuff
        public static SlugChessService Client { get; private set; } = new SlugChessService("localhost", 9999);
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
            _heartbeatTimer.Elapsed += (o, e) => { if (!Call.Alive(new Heartbeat { Alive = true, Usertoken = UserData.Usertoken }).Alive) { UserLoggedIn.OnNext(false); UserData = null; } };
            UserLoggedIn.Subscribe(loggedIn => { if (loggedIn) { _heartbeatTimer.Start(); } else { _heartbeatTimer.Stop(); } } );
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
        public UserData? UserData
        {
            get { return _userData; }
            set { if (value != _userData) { _userData = value; NotifyPropertyChanged(); } }
        }
        private UserData? _userData = null;

        private System.Timers.Timer _heartbeatTimer = new System.Timers.Timer(60*1000);


        public Task<bool> LoginInUserAsync(string username, string password) => Task.Run<bool>(() => 
            {
                var ver = FileVersionInfo.GetVersionInfo(Assembly.GetExecutingAssembly().Location);
                var result = Client.Call.Login(new LoginForm { Username = username, MajorVersion = ver.FileMajorPart.ToString(), MinorVersion = ver.FileMinorPart.ToString(), BuildVersion = ver.FileBuildPart.ToString() });
                if (result.SuccessfullLogin)
                {
                    UserLoggedIn.OnNext(true);
                    UserData = new UserData
                    {
                        Username = result.UserData.Username,
                        Usertoken = result.UserData.Usertoken,
                        Elo = result.UserData.Elo
                    };
                    Serilog.Log.Information("Logged in as " + result.UserData.Username);
                    if (result.LoginMessage != "") Serilog.Log.Information("Login Message: " + result.LoginMessage);
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
                    return true;
                }
                else
                {
                    Serilog.Log.Information("Login failed. " + result.LoginMessage);
                    return false;
                }
            });
        
    }
        
}
