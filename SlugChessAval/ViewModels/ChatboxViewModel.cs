using ReactiveUI;
using System;
using System.Collections.Generic;
using System.Reactive;
using System.Text;

namespace SlugChessAval.ViewModels
{
    public class ChatboxViewModel : ViewModelBase
    {
        public string MessagesText;
        public string PlayerText;

        public ReactiveCommand<Unit, Unit> SendCommand { get; }

        public ChatboxViewModel()
        {
            SendCommand = ReactiveCommand.Create(() => { });
        }
    }
}
