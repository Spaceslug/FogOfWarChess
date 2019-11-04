using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SlugChess
{

    public class GlobalState
    {

        public static readonly Dictionary<string, int> BoardPosToIndex = new Dictionary<string, int>
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
        public static readonly List<string> BoardPos = new List<string>
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
        /// <summary>
        /// a1 = 0, a8 = 7, b1 = 8, h8 = 63
        /// </summary>
        private List<Field> Board { get; set; } = new List<Field>(64);
        private bool[] WhiteVision { get; set; } = new bool[64];
        private bool[] BlackVision { get; set; } = new bool[64];
        private string _selected = null;
        public string Selected { get { return _selected; } set { _legalMovesSelected = (value != null?GameRules.GetLegalMoves(this, new FieldState(value, GetFieldAt(value))):new List<(string, List<FieldState>)>()); _selected = value; } } 
        public bool WhiteTurn { get; set; } = true;
        public VisionRules VisionRules { get; set; }
        private List<(string, List<FieldState>)> _legalMovesSelected = new List<(string, List<FieldState>)>();
        private Dictionary<string, List<(string, List<FieldState>)>> _legalMovesAll = new Dictionary<string, List<(string, List<FieldState>)>>();
        public IEnumerable<string> GetLegalMovesFromSelected => _legalMovesSelected.Select(items => items.Item1);
        public IEnumerable<string> GetLegalMovesFromField(string field) => (_legalMovesAll.ContainsKey(field)?_legalMovesAll[field]?.Select(items => items.Item1):new List<string>());

        //public void UpdateState(GlobalState oldState)
        //{
        //    Board = new List<Field>(64);
        //    Board.AddRange(oldState.Board);
        //    Selected = oldState.Selected;

        //}
        public GlobalState ShallowCopy()
        {
            GlobalState newState = new GlobalState();
            newState.Board = this.Board;
            newState.WhiteVision = this.WhiteVision;
            newState.BlackVision = this.BlackVision;
            newState._selected = this._selected;
            newState.WhiteTurn = this.WhiteTurn;
            newState.VisionRules = this.VisionRules;
            newState.WhiteVision = this.WhiteVision;
            newState._legalMovesSelected = this._legalMovesSelected;
            newState._legalMovesAll = this._legalMovesAll;

            return newState;
        } 

        public void CleanAnPassants()
        {
            int i = Board.FindIndex(field => field.AnPassan_able);
            if(i > -1)
            {
                Field f = Board[i];
                f.AnPassan_able = false;
                Board[i]= f;
            }
        }

        public void CleanLastCapturedField()
        {
            int i = Board.FindIndex(field => field.PiceCapturedLastMove);
            if (i > -1)
            {
                Field f = Board[i];
                f.PiceCapturedLastMove = false;
                Board[i] = f;
            }
        }

        public Field GetFieldAt(string pos)
        {
            return Board[BoardPosToIndex[pos]];
        }

        public Field GetField(int index)
        {
            return Board[index];
        }

        public bool IsLegalMove(string pos)
        {
            if (WhiteTurn ? WhiteVision[BoardPosToIndex[pos]]: BlackVision[BoardPosToIndex[pos]])
            {
                return _legalMovesSelected.Any((a) => a.Item1 == pos);
            }
            return false;
        }

        public Pices DoMoveTo(string moveTo)
        {
            Pices killedPice = Pices.Non;
            (var name, var extraFieldList) = _legalMovesSelected.Find((a) => a.Item1 == moveTo);
            //Must clean an passants and last captured field
            CleanAnPassants();
            CleanLastCapturedField();
            //
            Field fromField = GetFieldAt(_selected);
            Field toField = GetFieldAt(moveTo);
            if (extraFieldList == null || extraFieldList.Count == 0)
            {
                (Field from, Field to, Pices killed) = GameRules.Move(fromField, toField);
                if (to.Pice == Pices.WhitePawn && moveTo[1] == '8') to.Pice = Pices.WhiteQueen;
                if (to.Pice == Pices.BlackPawn && moveTo[1] == '1') to.Pice = Pices.BlackQueen;
                if (killed != Pices.Non) to.PiceCapturedLastMove = true;
                Board[BoardPosToIndex[_selected]] = from;
                Board[BoardPosToIndex[moveTo]] = to;
                killedPice = killed;
            }
            else if (extraFieldList.Count == 1)
            {
                (Field from, Field to, Field anPs, Pices killed) = GameRules.Move(fromField, toField, extraFieldList[0].Field);
                Board[BoardPosToIndex[_selected]] = from;
                Board[BoardPosToIndex[moveTo]] = to;
                Board[BoardPosToIndex[extraFieldList[0].FieldName]] = anPs;
            }
            else if (extraFieldList.Count == 2)
            {
                (Field fromK, Field toK, Field fromR, Field toR, Pices killed) = GameRules.Move(fromField, toField, extraFieldList[0].Field, extraFieldList[1].Field);
                Board[BoardPosToIndex[_selected]] = fromK;
                Board[BoardPosToIndex[moveTo]] = toK;
                Board[BoardPosToIndex[extraFieldList[0].FieldName]] = fromR;
                Board[BoardPosToIndex[extraFieldList[1].FieldName]] = toR;
            }
            else
            {
                throw new NotImplementedException("what tha fuck man");
            }

            WhiteTurn = !WhiteTurn;
            CalculateVision();
            _legalMovesAll = GameRules.GetLegalMoves(this);

            //Selected = null;
            return killedPice;
        }

        public string GetWhiteKingPos()
        {
            return BoardPosToIndex.FirstOrDefault(s => Board[s.Value].Pice == Pices.WhiteKing).Key;
        }
        public string GetBlackKingPos()
        {
            return BoardPosToIndex.FirstOrDefault(s => Board[s.Value].Pice ==  Pices.BlackKing).Key;
        }
        public void CalculateVision()
        {
            if (!VisionRules.Enabled)
            {
                WhiteVision = Enumerable.Repeat<bool>(true, 64).ToArray();
                BlackVision = Enumerable.Repeat<bool>(true, 64).ToArray();
                return;
            }
            BlackVision = new bool[64];
            WhiteVision = new bool[64];
            for (int i = 0; i < 64; i++)
            {
                if (Board[i].Pice == Pices.Non) continue;
                GameRules.AddPiceVision(Board, i, VisionRules.PiceOverwrite.ContainsKey(Board[i].Pice)? VisionRules.PiceOverwrite[Board[i].Pice] : VisionRules, Board[i].HasWhitePice()? WhiteVision:BlackVision);
                if (VisionRules.ViewMoveFields)
                {
                    GlobalState otherState = this.ShallowCopy();
                    otherState.VisionRules.Enabled = false;
                    var legalMoves = GameRules.GetLegalMoves(this, new FieldState(BoardPos[i], Board[i]));
                    foreach (var endPosTuple in legalMoves)
                    {
                        var visionBoard = Board[i].HasWhitePice() ? WhiteVision : BlackVision;
                        visionBoard[BoardPosToIndex[endPosTuple.Item1]] = true;
                    }
                }
                if(VisionRules.ViewCaptureField && Board[i].PiceCapturedLastMove)
                {
                    WhiteVision[i] = true;
                    BlackVision[i] = true;
                }
            }
        }

        public bool CanSeeField(ClientIsPlayer visionMode, string fieldName)
        {
            switch (visionMode)
            {
                case ClientIsPlayer.Both:
                    return WhiteTurn ? WhiteVision[GameRules.BPToIndx(fieldName)] : BlackVision[GameRules.BPToIndx(fieldName)];
                case ClientIsPlayer.White:
                    return WhiteVision[GameRules.BPToIndx(fieldName)];
                case ClientIsPlayer.Black:
                    return BlackVision[GameRules.BPToIndx(fieldName)];
                default:
                    throw new Exception("fuck you this cant happen");
            }
        }

        public bool CanWhiteSeeField(string fieldName)
        {
            return WhiteVision[GameRules.BPToIndx(fieldName)];
        }

        public bool CanBlackSeeField(string fieldName)
        {
            return BlackVision[GameRules.BPToIndx(fieldName)];
        }

        public static GlobalState CreateStartState(VisionRules visionRules)
        {
            var globalState = new GlobalState {
                VisionRules = visionRules
            };
            globalState.Board.Add(new Field(Pices.WhiteRook, false, false, true, true, false));
            globalState.Board.Add(new Field(Pices.WhiteKnight, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhiteBishop, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhiteQueen, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhiteKing, false, false, true, true, false));
            globalState.Board.Add(new Field(Pices.WhiteBishop, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhiteKnight, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhiteRook, false, false, true, true, false));
            //
            globalState.Board.Add(new Field(Pices.WhitePawn, true, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhitePawn, true, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhitePawn, true, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhitePawn, true, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhitePawn, true, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhitePawn, true, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhitePawn, true, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhitePawn, true, false, false, true, false));
            //
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            //
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            //
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            //
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            //
            globalState.Board.Add(new Field(Pices.BlackPawn, true, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackPawn, true, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackPawn, true, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackPawn, true, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackPawn, true, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackPawn, true, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackPawn, true, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackPawn, true, false, false, true, false));
            //
            globalState.Board.Add(new Field(Pices.BlackRook, false, false, true, true, false));
            globalState.Board.Add(new Field(Pices.BlackKnight, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackBishop, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackQueen, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackKing, false, false, true, true, false));
            globalState.Board.Add(new Field(Pices.BlackBishop, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackKnight, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackRook, false, false, true, true, false));

            globalState.CalculateVision();
            globalState._legalMovesAll = GameRules.GetLegalMoves(globalState);
            return globalState;
        }

        public static GlobalState CreateStartStateNoPawns()
        {
            var globalState = new GlobalState();
            globalState.Board.Add(new Field(Pices.WhiteRook, false, false, true, true, false));
            globalState.Board.Add(new Field(Pices.WhiteKnight, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhiteBishop, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhiteQueen, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhiteKing, false, false, true, true, false));
            globalState.Board.Add(new Field(Pices.WhiteBishop, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhiteKnight, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.WhiteRook, false, false, true, true, false));
            //
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            //
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            //
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            //
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            //
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            //
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            globalState.Board.Add(new Field(Pices.Non));
            //
            globalState.Board.Add(new Field(Pices.BlackRook, false, false, true, true, false));
            globalState.Board.Add(new Field(Pices.BlackKnight, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackBishop, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackQueen, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackKing, false, false, true, true, false));
            globalState.Board.Add(new Field(Pices.BlackBishop, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackKnight, false, false, false, true, false));
            globalState.Board.Add(new Field(Pices.BlackRook, false, false, true, true, false));

            globalState.CalculateVision();
            globalState._legalMovesAll = GameRules.GetLegalMoves(globalState);
            return globalState;
        }


    }

    public enum Pices
    {
        Non = 0,
        BlackPawn = 1,
        BlackKnight = 2,
        BlackBishop = 3,
        BlackRook = 4,
        BlackQueen = 5,
        BlackKing = 6,
        WhitePawn = 7,
        WhiteKnight = 8,
        WhiteBishop = 9,
        WhiteRook = 10,
        WhiteQueen = 11,
        WhiteKing = 12
    }
    public struct FieldState
    {
        public string FieldName;
        public Field Field;
        public FieldState(string fieldName, Field field)
        {
            Field = field;
            FieldName = fieldName;
        }
    }
    public struct Field
    {
        public bool AnPassan_creating;
        public bool AnPassan_able;
        public bool Rokade_able;
        public bool FirstMove;
        public bool PiceCapturedLastMove;
        public Pices Pice;

        public Field(Pices pice, bool anPassanCreating, bool anPassanAble, bool rokadeAble, bool firstMove, bool piceCapturedLastMove)
        {
            Pice = pice;
            AnPassan_creating = anPassanCreating;
            AnPassan_able = anPassanAble;
            Rokade_able = rokadeAble;
            FirstMove = firstMove;
            PiceCapturedLastMove = piceCapturedLastMove;
        }

        public Field(Pices pice) : this(pice, false, false, false, false, false)
        {

        }
        public bool HasBlackPice()
        {
            return Field.HasBlackPice(this);
        }
        public bool HasWhitePice()
        {
            return Field.HasWhitePice(this);
        }

        public static bool HasBlackPice(Field field)
        {
            switch (field.Pice)
            {
                case Pices.BlackKing:
                case Pices.BlackQueen:
                case Pices.BlackBishop:
                case Pices.BlackKnight:
                case Pices.BlackRook:
                case Pices.BlackPawn:
                    return true;
                default:
                    return false;
            }
        }

        public static bool HasWhitePice(Field field)
        {
            switch (field.Pice)
            {
                case Pices.WhiteKing:
                case Pices.WhiteQueen:
                case Pices.WhiteBishop:
                case Pices.WhiteKnight:
                case Pices.WhiteRook:
                case Pices.WhitePawn:
                    return true;
                default:
                    return false;
            }
        }

    }
}
