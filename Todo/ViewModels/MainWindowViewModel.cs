using Todo.Services;
using Todo.Models;
using ReactiveUI;
using System;
using System.Reactive.Linq;

namespace Todo.ViewModels
{
    class MainWindowViewModel : ViewModelBase
    {
        public ViewModelBase Content
        {
            get => _content;
            private set => this.RaiseAndSetIfChanged(ref _content, value);
        }
        private ViewModelBase _content;

        public string Greeting => "Welcome to Avalonia!";
        public string UserSignatureLabel => "Enter Signature here";
        public string UserSignature { get; set; }
        public TodoListViewModel List { get; }

        public MainWindowViewModel(Database db)
        {
            Content = List = new TodoListViewModel(db.GetItems());
        }

        public void AddItem()
        {
            var vm = new AddItemViewModel();
            Observable.Merge(
                vm.Ok,
                vm.Cancel.Select(_ => (TodoItem)null)
            ).Take(1).Subscribe(model =>
                {
                    if (model != null)
                    {
                        List.Items.Add(model);
                    }
                    Content = List;
                }
            );
            Content = vm;
        }
    }
}