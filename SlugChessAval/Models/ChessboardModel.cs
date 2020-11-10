using Avalonia.Logging;
using SlugChessAval.ViewModels;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Runtime.Serialization;
using System.Text;

namespace SlugChessAval.Models
{
    [DataContract]
    public class ChessboardModel
    {
        public enum VisionTypes
        {
            White,
            Black,
            Observer
        }

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

        public static bool FieldColorLight(string pos)
        {
            return (pos[1] % 2) == (pos[0] % 2);
        }

        public static VisionTypes ToVisionType(PlayerIs player) => player switch
        {
            PlayerIs.White => VisionTypes.White,
            PlayerIs.Black => VisionTypes.Black,
            PlayerIs.Observer => VisionTypes.Observer,
            PlayerIs.Both => VisionTypes.Observer,
            PlayerIs.Non => throw new NotImplementedException(),
            _ => throw new NotImplementedException()
        };


        [DataMember]
        public IDictionary<string, List<string>> Moves { get; set; } = new Dictionary<string, List<string>>();
        [DataMember]
        public Dictionary<string, List<string>> ShadowMoves { get; set; } = new Dictionary<string, List<string>>();
        [DataMember]
        public List<ViewModels.DataTemplates.CapturedPice> CapturedPices { get; set; } = new List<ViewModels.DataTemplates.CapturedPice>(new[] { ViewModels.DataTemplates.CapturedPice.Empty });

        public ChessCom.Pices GetFieldPice(string fieldname) => _fieldPices[BoardPosToIndex[fieldname]];
        public IList<ChessCom.Pices> FieldPices => _fieldPices;
        [DataMember]
        private IList<ChessCom.Pices> _fieldPices;

        public bool InACheck(string fieldname) => _checks.Contains(fieldname);
        [DataMember]
        private IList<string> _checks;

        [DataMember]
        public string From { get; private set; } = "";
        [DataMember]
        public string To { get; private set; } = "";

        public bool InVision(string fieldname) => Visions[PlayerVisionType][BoardPosToIndex[fieldname]];

        public VisionTypes PlayerVisionType { get; private set; }
        public IDictionary<VisionTypes, List<bool>> Visions { get; private set; } = new Dictionary<VisionTypes, List<bool>>();

        private ChessboardModel(IList<string> checks, IList<ChessCom.Pices> fieldPices)
        {
            _checks = checks;
            _fieldPices = fieldPices;
        }

        public static ChessboardModel FromChesscomGamestate(ChessCom.GameState gameState, VisionTypes visionType)
        {
            
            
            //     public string PlayingAs => (MatchModel?.PlayerIs??PlayerIs.Non) switch 
            //{ PlayerIs.White => "Playing as White", PlayerIs.Black => "Playing as Black", PlayerIs.Both => "Playing yourself", PlayerIs.Oberserver => "Watching as Observer", _ => "No game active" };
            var chessboard = new ChessboardModel(gameState.Check, gameState.Pices);
            foreach(var keyval in gameState.PlayerMoves)
            {
                chessboard.Moves.Add(keyval.Key, new List<string>(keyval.Value.List));
            }
            foreach (var keyval in gameState.ShadowMoves)
            {
                chessboard.ShadowMoves.Add(keyval.Key, new List<string>(keyval.Value.List));
            }
            chessboard.From = gameState.From;
            chessboard.To = gameState.To;
            chessboard.CapturedPices = new List<ViewModels.DataTemplates.CapturedPice>(gameState.CapturedPices.Select(x => new ViewModels.DataTemplates.CapturedPice(x)));
            chessboard.PlayerVisionType = visionType;
            if(gameState.VisionState.WhiteVision.Count == 64)
            {
                chessboard.Visions.Add(VisionTypes.White, gameState.VisionState.WhiteVision.ToList());
            }
            if (gameState.VisionState.BlackVision.Count == 64)
            {
                chessboard.Visions.Add(VisionTypes.Black, gameState.VisionState.BlackVision.ToList());
            }
            if (visionType == VisionTypes.Observer)
            {
                chessboard.Visions.Add(VisionTypes.Observer, Enumerable.Repeat(true, 64).ToList());
            }
            if (!chessboard.Visions.ContainsKey(visionType))
            { 
                MainWindowViewModel.SendNotification("VisionBoard from server invalid");
                throw new ArgumentException("VisionBoard is ficked");
            }
            return chessboard;
        }

        public static ChessboardModel FromDefault()
        {
            var chessboard = new ChessboardModel(new List<string>(), Enumerable.Repeat(ChessCom.Pices.None, 64).ToList());
            chessboard.From = "";
            chessboard.To = "";
            chessboard.PlayerVisionType = VisionTypes.Observer;
            chessboard.Visions.Add(VisionTypes.Observer, Enumerable.Repeat(true, 64).ToList());
            return chessboard;
        }

        public static ChessboardModel FromTestData()
        {
            var chessboard = new ChessboardModel(new List<string> { "d8", "h8" }, Enumerable.Repeat(ChessCom.Pices.None, 64).ToList());
            chessboard.PlayerVisionType = VisionTypes.Observer;
            chessboard.Visions.Add(VisionTypes.Observer, Enumerable.Repeat(true, 64).ToList());

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

            chessboard.From = "d2";
            chessboard.To = "d8";

            chessboard.Visions[VisionTypes.Observer][56] = false;
            chessboard.Visions[VisionTypes.Observer][48] = false;
            chessboard.Visions[VisionTypes.Observer][40] = false;
            chessboard.Visions[VisionTypes.Observer][32] = false;
            chessboard.Visions[VisionTypes.Observer][57] = false;
            chessboard.Visions[VisionTypes.Observer][58] = false;
            chessboard.Visions[VisionTypes.Observer][49] = false;
            chessboard.Visions[VisionTypes.Observer][41] = false;
            chessboard.Visions[VisionTypes.Observer][33] = false;
            chessboard.Visions[VisionTypes.Observer][26] = false;

            chessboard.Moves["e7"] = new List<string> { "e8", "e6", "e5", "e4", "e3" };
            chessboard.Moves["g7"] = new List<string> { "g6" };
            chessboard.Moves["h7"] = new List<string> { "h6" };
            chessboard.ShadowMoves["g2"] = new List<string> { "g3", "g4" };

            return chessboard;
        }
    }
}
