using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ChessCom;

namespace SlugChess
{
    public class MatchesBind
    {

        public string Host { get; set; }
        public int HostElo { get; set; }
        public string ChessType { get; set; }
        public string SideType { get; set; }
        public string Time { get; set; }
        public string VisionRules { get; set; }
        private int _matchId;
        public int GetMatchId() => _matchId;
        private string _hostUsertoken;
        public string GetHostUsertoken() => _hostUsertoken;

        public static List<MatchesBind> FromChesscom(ChessCom.HostedGamesMap hostedGamesMap)
        {
            var matches = new List<MatchesBind>();
            foreach(var keyVal in hostedGamesMap.HostedGames)
            {
                var match = new MatchesBind
                {
                    _matchId = keyVal.Value.Id,
                    Host = keyVal.Value.Host.Username,
                    _hostUsertoken = keyVal.Value.Host.Usertoken,
                    HostElo = keyVal.Value.Host.Elo,
                    ChessType = keyVal.Value.GameRules.ChessType.ToString(),
                    SideType = keyVal.Value.GameRules.SideType.ToString(),
                    Time = TimeRulesToString(keyVal.Value.GameRules.TimeRules),
                    VisionRules = "SlugChess"
                };
                //matches.Add();
            }

            return matches;
        }

        public static string TimeRulesToString(TimeRules timeRules)
        {
            if (timeRules == null) return "<null>";
            return $"{timeRules.PlayerTime}min + {timeRules.SecondsPerMove}s";
        }
    }
}
