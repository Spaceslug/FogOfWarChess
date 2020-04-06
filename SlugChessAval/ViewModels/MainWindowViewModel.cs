using System;
using System.Collections.Generic;
using System.Text;
using SlugChessAval.Services;
using ReactiveUI;

namespace SlugChessAval.ViewModels
{
    public class MainWindowViewModel : ViewModelBase
    {
        public string Greeting => "Welcome to Avalonia!";
        public string UserSignatureLabel => "Enter Signature here";
        public string UserSignature { get; set; }
        public TodoListViewModel List { get; }

        public ViewModelBase Content
        {
            get => _content;
            private set => this.RaiseAndSetIfChanged(ref _content, value);
        }
        private ViewModelBase _content;

        public MainWindowViewModel()
        {

            
            Content = List = new TodoListViewModel();
        }
    }
}
