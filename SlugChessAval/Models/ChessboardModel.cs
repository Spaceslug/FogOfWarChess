using System;
using System.Collections.Generic;
using System.Text;

namespace SlugChessAval.Models
{
    public class ChessboardModel
    {
        public ChessCom.GameState ComGameState;
        public Dictionary<string, List<string>> Moves { get; set; } = new Dictionary<string, List<string>>();
        public Dictionary<string, object> Dddddd { get; set; }

        private ChessboardModel()
        {

        }

        public static ChessboardModel FromChesscomGamestate(ChessCom.GameState gameState)
        {
            var chessboard = new ChessboardModel();
            foreach(var keyval in gameState.PlayerMoves)
            {
                chessboard.Moves.Add(keyval.Key, new List<string>(keyval.Value.List));
            }
            //gameState.

            return chessboard;
        }

        public static ChessboardModel FromDefault()
        {
            var chessboard = new ChessboardModel();
            throw new NotImplementedException("aaa");
        }
    }
}
