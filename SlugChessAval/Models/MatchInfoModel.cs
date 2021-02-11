using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using ChessCom;

namespace SlugChessAval.Models
{
    public enum PlayerIs
    {
        Non,
        White,
        Black,
        Both,
        Observer
    }

    public class MatchInfoModel
    {

        public string Host { get; set; }
        public int HostElo { get; set; }
        public string Variant { get; set; }
        //public string ChessType { get; set; }
        public string SideType { get; set; }
        public string Time { get; set; }
        //public string VisionRules { get; set; }
        private int _matchId;
        public int GetMatchId() => _matchId;
        private string _hostUsertoken;

        private MatchInfoModel(string host, int hostElo, string variant, string sideType, string time, int matchId, string hostUsertoken)
        {
            Host = host;
            HostElo = hostElo;
            Variant = variant;
            //ChessType = chessType;
            SideType = sideType;
            Time = time;
            //VisionRules = visionRules;
            _matchId = matchId;
            _hostUsertoken = hostUsertoken;
        }

        public string GetHostUsertoken() => _hostUsertoken;

        public static List<MatchInfoModel> FromChesscom(HostedGamesMap hostedGamesMap)
        {
            var matches = new List<MatchInfoModel>();
            foreach (var keyVal in hostedGamesMap.HostedGames)
            {
                var gameRules = keyVal.Value.GameRules;
                var match = new MatchInfoModel(
                    keyVal.Value.Host.Username,
                    (int)Math.Round(keyVal.Value.Host.Elo),
                    (keyVal.Value.GameRules.VariantCase == GameRules.VariantOneofCase.Named? keyVal.Value.GameRules.Named:"Custom"),
                    //keyVal.Value.GameRules.ChessType.ToString(),
                    keyVal.Value.GameRules.SideType.ToString(),
                    TimeRulesToString(keyVal.Value.GameRules.TimeRules),
                    //gameRules.VisionRulesCase == GameRules.VisionRulesOneofCase.TypeRules?"SlugChess."+ gameRules.TypeRules:throw new NotImplementedException("aaaaaaaaaahahahaaaaaahaaaa"),
                    keyVal.Value.Id,
                    keyVal.Value.Host.Usertoken
                    );
                matches.Add(match);
            }

            return matches;
        }

        public static List<MatchInfoModel> FromTestData()
        {
            return new List<MatchInfoModel> { new MatchInfoModel("frank", 9999, "TourchWip", ChessCom.SideType.Random.ToString(), "gucvk all", 153135, "tickenthing")
                , new MatchInfoModel("dannnr", 8888, "Sigith", ChessCom.SideType.Random.ToString(), "gucvk all", 153136, "tickenthing") };
        }

        public static string TimeRulesToString(TimeRules timeRules)
        {
            if (timeRules == null) return "<null>";
            return $"{timeRules.PlayerTime.Minutes}min + {timeRules.SecondsPerMove}s";
        }

    }
}
