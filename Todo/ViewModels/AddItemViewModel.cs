using System.Reactive;
using ReactiveUI;
using Todo.Models;

namespace Todo.ViewModels
{
    class AddItemViewModel : ViewModelBase
    {
        public string Description
        {
            get => _description;
            set => this.RaiseAndSetIfChanged(ref _description, value);
        }
        private string _description;

        public ReactiveCommand<Unit, TodoItem> Ok { get; }
        public ReactiveCommand<Unit, Unit> Cancel { get; }

        public AddItemViewModel()
        {
            var okEnabled = this.WhenAnyValue(
                x => x.Description,
                x => !string.IsNullOrWhiteSpace(x));

            Ok = ReactiveCommand.Create(
                () => new TodoItem { Description = Description },
                okEnabled);
            Cancel = ReactiveCommand.Create(() => { });
        }
    }
}