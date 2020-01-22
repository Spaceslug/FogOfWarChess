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
    /// Interaction logic for CreateGame.xaml
    /// </summary>
    public partial class CreateGame : Window
    {
        public ChessCom.LookForMatchResult MatchResult { get; set; } = null;

        private ServerConnection _connection;
        private ChessCom.UserData _userdata;

        public CreateGame(ServerConnection connection, ChessCom.UserData userdata)
        {
            _connection = connection;
            _userdata = userdata;
            InitializeComponent();
        }

        private void BtHostGame_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                var starttime = Convert.ToInt32(tbStarttime.Text);
                var movetime = Convert.ToInt32(tbMovetime.Text);
                ChessCom.ChessType chessType = cbChesType.SelectedIndex == 0 ? ChessCom.ChessType.Classic :
                                                cbChesType.SelectedIndex == 1 ? ChessCom.ChessType.FisherRandom :
                                                ChessCom.ChessType.SlugRandom;
                ChessCom.SideType sideType = cbHostColor.SelectedIndex == 0 ? ChessCom.SideType.HostIsWhite :
                                                cbHostColor.SelectedIndex == 1 ? ChessCom.SideType.HostIsBlack :
                                                ChessCom.SideType.Random;
                ChessCom.VisionRules vr = new ChessCom.VisionRules();
                switch (cbVisionRules.SelectedIndex)
                {
                    case 0: //Standard
                        {
                            vr.Enabled = true;
                            vr.ViewCaptureField = true;
                            vr.ViewMoveFields = false;
                            vr.ViewRange = 2;
                            ChessCom.VisionRules overwrite = new ChessCom.VisionRules
                            {
                                ViewRange = 1,
                                ViewMoveFields = false
                            };
                            vr.PiceOverwriter.Add((int)ChessCom.Pices.WhitePawn, overwrite);
                            vr.PiceOverwriter.Add((int)ChessCom.Pices.BlackPawn, overwrite);
                        }
                        break;
                    case 1: //Sea
                        {
                            vr.Enabled = true;
                            vr.ViewCaptureField = true;
                            vr.ViewMoveFields = true;
                            vr.ViewRange = 0;
                            ChessCom.VisionRules overwrite = new ChessCom.VisionRules
                            {
                                ViewRange = 1,
                                ViewMoveFields = true
                            };
                            vr.PiceOverwriter.Add((int)ChessCom.Pices.WhiteKnight, overwrite);
                            vr.PiceOverwriter.Add((int)ChessCom.Pices.BlackKnight, overwrite);
                            vr.PiceOverwriter.Add((int)ChessCom.Pices.WhitePawn, overwrite);
                            vr.PiceOverwriter.Add((int)ChessCom.Pices.BlackPawn, overwrite);
                        }
                        break;
                    case 2: //No Vision
                        {
                            vr.Enabled = false;
                        }
                        break;
                }
                btHostGame.IsEnabled = false;
                cbChesType.IsEnabled = false;
                cbHostColor.IsEnabled = false;
                cbVisionRules.IsEnabled = false;
                tbStarttime.IsEnabled = false;
                tbMovetime.IsEnabled = false;
                tbHostingMatchText.Visibility = Visibility.Visible;

                Task.Run(() => {
                    _connection.Call.HostGame(new ChessCom.HostedGame
                    {
                        HostUsername = _userdata.Username,
                        HostUsertoken = _userdata.Usertoken,
                        GameRules = new ChessCom.GameRules
                    {
                        ChessType = chessType,
                        SideType = sideType,
                        VisionRules = vr
                    }
                    });

                });

            }
            catch(Exception ex)
            {
                throw ex;
            }
           
        }
    }
}
