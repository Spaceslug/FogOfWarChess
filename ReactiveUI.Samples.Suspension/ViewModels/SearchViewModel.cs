using System.Reactive;
using System.Reactive.Linq;
using System.Runtime.Serialization;
using System.Threading.Tasks;
using System.Windows.Input;
using Splat;

namespace ReactiveUI.Samples.Suspension.ViewModels
{
    [DataContract]
    public class SearchViewModel : ReactiveObject, IRoutableViewModel
    {
        private readonly ReactiveCommand<Unit, Unit> _search;
        private string _searchQuery;

        // We inject the IScreen implementation via the constructor.
        // If we receive null, we use Splat.Locator to resolve the
        // default implementation. The parameterless constructor is
        // required for the deserialization feature to work.
        public SearchViewModel(IScreen screen = null) 
        {
            HostScreen = screen ?? Locator.Current.GetService<IScreen>();

            // Each time the search query changes, we check if the search 
            // query is empty. If it is, we disable the command.
            var canSearch = this
                .WhenAnyValue(x => x.SearchQuery)
                .Select(query => !string.IsNullOrWhiteSpace(query));

            // Buttons bound to the command will stay disabled
            // as long as the command stays disabled.
            _search = ReactiveCommand.CreateFromTask(
                () => Task.Delay(1000), // emulate a long-running operation
                canSearch);
        }

        public IScreen HostScreen { get; }

        public string UrlPathSegment => "/search";

        public ICommand Search => _search;

        [DataMember]
        public string SearchQuery 
        {
            get => _searchQuery;
            set => this.RaiseAndSetIfChanged(ref _searchQuery, value);
        }
    }
}