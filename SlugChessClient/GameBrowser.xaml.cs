using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
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
        private CancellationTokenSource _joinGameTokenSource = new CancellationTokenSource();

        public GameBrowser(ServerConnection connection, ChessCom.UserData userdata)
        {
            _connection = connection;
            _userdata = userdata;
            InitializeComponent();
            Task.Run(()=> {
                var matches = MatchesBind.FromChesscom(_connection.Call.AvailableGames(new ChessCom.Void()));
                Dispatcher.Invoke(() => this.dgMatchesDataGrid.ItemsSource = matches);
             });
            
        }

        private void JoinGame_Click(object sender, RoutedEventArgs e)
        {
            if (dgMatchesDataGrid.SelectedIndex < 0) return;
            int selectedId = ((MatchesBind)dgMatchesDataGrid.Items[dgMatchesDataGrid.SelectedIndex]).GetMatchId();
            Task.Run(() => {
                var token = _joinGameTokenSource.Token;
                try
                {
                    MatchResult = _connection.Call.JoinGame(new ChessCom.JoinGameRequest
                    {
                        Id = selectedId,
                        Joiner = _userdata
                    }, null, null, _joinGameTokenSource.Token);
                    Dispatcher.Invoke(() => Close());
                }
                catch (Grpc.Core.RpcException ex)
                {
                    if (!token.IsCancellationRequested)
                    {
                        throw ex;
                    }
                }


            });
        }

        private void dgMatchesDataGrid_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            var dataGrid = sender as DataGrid;
            if(dataGrid.SelectedIndex == -1)
            {
                btJoinGame.IsEnabled = false;
            }
            else
            {
                btJoinGame.IsEnabled = true;
            }
        }
    }
}
