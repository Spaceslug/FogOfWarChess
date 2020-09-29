using ReactiveUI;
using SlugChessAval.Services;
using System;
using System.Collections.Generic;
using System.Reactive;
using System.Reactive.Disposables;
using System.Text;

namespace SlugChessAval.ViewModels
{
    public class ChatboxViewModel : ViewModelBase, IActivatableViewModel
    {
        public ViewModelActivator Activator { get; } = new ViewModelActivator();

        public ReactiveCommand<Unit, Unit> SendTextCommand { get; }

        public string? OpponentUsertoken { get; set; } = null;

        public string ChatroomName
        {
            get => _chatroomName;
            set => this.RaiseAndSetIfChanged(ref _chatroomName, value);
        }
        private string _chatroomName = "";

        public string MessageText
        {
            get => _messageText;
            set => this.RaiseAndSetIfChanged(ref _messageText, value);
        }
        private string _messageText = "";
        public string RecivedMessagesText
        {
            get => _recivedMessagesText;
            set => this.RaiseAndSetIfChanged(ref _recivedMessagesText, value);
        }
        private string _recivedMessagesText = "";
        public int CaretIndex
        {
            get => _caretIndex;
            set => this.RaiseAndSetIfChanged(ref _caretIndex, value);
        }
        private int _caretIndex = 0;

        public ChatboxViewModel()
        {
            ChatroomName = "system/CurrentMatch";
            SendTextCommand = ReactiveCommand.Create(() =>  
            {
                SlugChessService.Client.Call.SendChatMessage(new ChessCom.ChatMessage
                {
                    Message = MessageText,
                    SenderUsername = SlugChessService.Client.UserData.Username,
                    SenderUsertoken = SlugChessService.Client.UserData.Usertoken,
                    ReciverUsertoken = OpponentUsertoken ?? "system"
                });
                MessageText = "";
            }, this.WhenAnyValue((x)=> x.MessageText, (messageText) => !string.IsNullOrWhiteSpace(messageText)));
            this.WhenActivated(disposables =>
            {
                SlugChessService.Client.Messages.Subscribe(x => { 
                    RecivedMessagesText = RecivedMessagesText + "\n" + x; 
                    CaretIndex = RecivedMessagesText.Length;
                }).DisposeWith(disposables);
                
                Disposable.Create(() =>
                {
                    RecivedMessagesText = ""; //Clear text as next subscribe to messages will replay all
                }).DisposeWith(disposables);
            });
        }
    }
}
