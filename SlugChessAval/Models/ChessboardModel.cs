using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;

namespace SlugChessAval.Models
{
    [DataContract]
    public class ChessboardModel
    {
        public static readonly IDictionary<string, int> BoardPosToIndex = new Dictionary<string, int>
        {
            { "a1", 0 }, { "b1", 1 }, { "c1", 2 }, { "d1", 3 }, { "e1", 4 }, { "f1", 5 }, { "g1", 6 }, { "h1", 7 },
            { "a2", 8 }, { "b2", 9 }, { "c2", 10 },{ "d2", 11 },{ "e2", 12 },{ "f2", 13 },{ "g2", 14 },{ "h2", 15 },
            { "a3", 16 },{ "b3", 17 },{ "c3", 18 },{ "d3", 19 },{ "e3", 20 },{ "f3", 21 },{ "g3", 22 },{ "h3", 23 },
            { "a4", 24 },{ "b4", 25 },{ "c4", 26 },{ "d4", 27 },{ "e4", 28 },{ "f4", 29 },{ "g4", 30 },{ "h4", 31 },
            { "a5", 32 },{ "b5", 33 },{ "c5", 34 },{ "d5", 35 },{ "e5", 36 },{ "f5", 37 },{ "g5", 38 },{ "h5", 39 },
            { "a6", 40 },{ "b6", 41 },{ "c6", 42 },{ "d6", 43 },{ "e6", 44 },{ "f6", 45 },{ "g6", 46 },{ "h6", 47 },
            { "a7", 48 },{ "b7", 49 },{ "c7", 50 },{ "d7", 51 },{ "e7", 52 },{ "f7", 53 },{ "g7", 54 },{ "h7", 55 },
            { "a8", 56 },{ "b8", 57 },{ "c8", 58 },{ "d8", 59 },{ "e8", 60 },{ "f8", 61 },{ "g8", 62 },{ "h8", 63 }
        };
        public static readonly IList<string> BoardPos = new List<string>
        {
            "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
            "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
            "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
            "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
            "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
            "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
            "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
            "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
        };


        public ChessCom.GameState ComGameState;
        [DataMember]
        public IDictionary<string, List<string>> Moves { get; set; } = new Dictionary<string, List<string>>();
        [DataMember]
        public Dictionary<string, List<string>> ShadowMoves { get; set; } = new Dictionary<string, List<string>>();

        public ChessCom.Pices GetFieldPice(string fieldname) => _fieldPices[BoardPosToIndex[fieldname]];
        //public IDictionary<string, ChessCom.Pices> FieldPices
        //{
        //    get
        //    {
        //        var d = new Dictionary<string, ChessCom.Pices>();
        //        for (int i = 0; i < _fieldPices.Count; i++) d[BoardPos[i]] = _fieldPices[i];
        //        return d;
        //    }
        //}
        public IList<ChessCom.Pices> FieldPices => _fieldPices;
        [DataMember]
        private IList<ChessCom.Pices> _fieldPices;

        public bool InACheck(string fieldname) => _checks.Contains(fieldname);
        [DataMember]
        private IList<string> _checks;

        [DataMember]
        public string From { get; private set; }
        [DataMember]
        public string To { get; private set; }

        public bool InVision(string fieldname) => _vision[BoardPosToIndex[fieldname]];
        public IList<bool> Vision => _vision;
        [DataMember]
        private IList<bool> _vision;

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
            foreach (var keyval in gameState.ShadowMoves)
            {
                chessboard.ShadowMoves.Add(keyval.Key, new List<string>(keyval.Value.List));
            }
            chessboard._fieldPices = gameState.Pices;
            chessboard._checks = gameState.Check;
            chessboard.From = gameState.From;
            chessboard.To = gameState.To;
            chessboard._vision = gameState.PlayerVision;

            return chessboard;
        }

        public static ChessboardModel FromDefault()
        {
            var chessboard = new ChessboardModel();
            chessboard._fieldPices = Enumerable.Repeat(ChessCom.Pices.None, 64).ToList();
            chessboard._checks = new List<string>();
            chessboard.From = "";
            chessboard.To = "";
            chessboard._vision = Enumerable.Repeat(true, 64).ToList();
            return chessboard;
        }

        public static ChessboardModel FromTestData()
        {
            var chessboard = new ChessboardModel();
            chessboard._fieldPices = Enumerable.Repeat(ChessCom.Pices.None, 64).ToList();
            chessboard._fieldPices[59] = ChessCom.Pices.WhiteRook;
            chessboard._fieldPices[63] = ChessCom.Pices.BlackKing;
            chessboard._fieldPices[48] = ChessCom.Pices.BlackPawn;
            chessboard._fieldPices[49] = ChessCom.Pices.BlackPawn;
            chessboard._fieldPices[52] = ChessCom.Pices.BlackRook;
            chessboard._fieldPices[54] = ChessCom.Pices.BlackPawn;
            chessboard._fieldPices[55] = ChessCom.Pices.BlackPawn;

            chessboard._fieldPices[23] = ChessCom.Pices.WhitePawn;
            chessboard._fieldPices[13] = ChessCom.Pices.WhitePawn;
            chessboard._fieldPices[14] = ChessCom.Pices.WhitePawn;
            chessboard._fieldPices[6] = ChessCom.Pices.WhiteKing;

            chessboard._checks = new List<string> {"d8","h8"};
            chessboard.From = "d2";
            chessboard.To = "d8";
            chessboard._vision = Enumerable.Repeat(true, 64).ToList();
            chessboard._vision[56] = false;
            chessboard._vision[48] = false;
            chessboard._vision[40] = false;
            chessboard._vision[32] = false;
            chessboard._vision[57] = false;
            chessboard._vision[58] = false;
            chessboard._vision[49] = false;
            chessboard._vision[41] = false;
            chessboard._vision[33] = false;
            chessboard._vision[26] = false;

            chessboard.Moves["e7"] = new List<string> { "e8", "e6", "e5", "e4", "e3" };
            chessboard.Moves["g7"] = new List<string> { "g6" };
            chessboard.Moves["h7"] = new List<string> { "h6" };
            chessboard.ShadowMoves["g2"] = new List<string> { "g3", "g4" };

            return chessboard;
        }
    }
}
