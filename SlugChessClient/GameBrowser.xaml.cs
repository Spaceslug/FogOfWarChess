using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace SlugChess
{
    /// <summary>
    /// Interaction logic for GameBrowser.xaml
    /// </summary>
    public partial class GameBrowser : Window
    {

        public ChessCom.LookForMatchResult MatchResult { get; set; } = null;

        private ChessCom.UserData _userdata;
        private ServerConnection _connection;

        public GameBrowser(ServerConnection connection, ChessCom.UserData userdata)
        {
            _connection = connection;
            _userdata = userdata;
            InitializeComponent();
            Task.Run(()=> {
                var matches = MatchesBind.FromChesscom(_connection.Call.AvailableGames(new ChessCom.Void()));
                Dispatcher.Invoke(() => this.matchesDataGrid.ItemsSource = matches);
             });
            
        }
    }
}
