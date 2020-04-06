using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Text;
using SlugChessAval.Models;

namespace SlugChessAval.ViewModels
{
    public class TodoListViewModel : ViewModelBase
    {
        public TodoListViewModel()
        {
            Items = new ObservableCollection<TodoItem>();
        }

        public ObservableCollection<TodoItem> Items { get; }
    }
}
