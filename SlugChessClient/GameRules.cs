using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SlugChess
{
    public static class GameRules
    {
        public delegate string MoveFunction(string pos);
        public delegate bool ValidFieldFunction(Field field);

        private static bool LegalPos(string pos)
        {
            int pos1Int = (int)pos[0];
            int pos2Int = (int)pos[1];
            return pos1Int >= 97 && pos1Int <= 104 && pos2Int >= 49 && pos2Int <= 56;
        }
        private static bool LegalPos(int pos)
        {
            return pos >= 0 && pos < 64;
        }
        private static bool LegalColumn(int column)
        {
            return column >= 0 && column < 8;
        }
        private static bool LegalRow(int pos)
        {
            return LegalColumn(pos);
        }
        private static int IndexFromColRow(int column, int row)
        {
            return column + (8 * row);
        }
        private static string UpOne(string startPos)
        {
            return startPos[0] + char.ConvertFromUtf32(((int)startPos[1]) + 1);
        }
        private static int UpOne(int startPos)
        {
            return startPos + 8;
        }
        private static string DownOne(string startPos)
        {
            return startPos[0] + char.ConvertFromUtf32(((int)startPos[1]) - 1);
        }
        private static int DownOne(int startPos)
        {
            return startPos - 8;
        }
        private static string LeftOne(string startPos)
        {
            return char.ConvertFromUtf32(((int)startPos[0]) - 1) + startPos[1];
        }
        private static int LeftOne(int startPos)
        {
            if (startPos % 8 == 0) return -1;
            return startPos - 1;
        }
        private static string RightOne(string startPos)
        {
            return char.ConvertFromUtf32(((int)startPos[0]) + 1) + startPos[1];
        }
        private static int RightOne(int startPos)
        {
            if (startPos % 8 == 7) return -1;
            return startPos + 1;
        }

        public static void AddMove(GlobalState state, FieldState fromField, List<(string, List<FieldState>)> moveList, string endPos)
        {
            bool moveIsWhite = Field.HasWhitePice(fromField.Field);
            if (!LegalPos(endPos)) return;
            Field currentField = state.GetFieldAt(endPos);
            if (!(moveIsWhite ? Field.HasWhitePice(currentField) : Field.HasBlackPice(currentField)))
            {
                moveList.Add((endPos, null));
            }
        }

        public static void AddPwnAttack(GlobalState state, FieldState fromField, List<(string, List<FieldState>)> moveList, string endPos)
        {
            bool moveIsWhite = Field.HasWhitePice(fromField.Field);
            if (!LegalPos(endPos)) return;
            Field currentField = state.GetFieldAt(endPos);
            if ((moveIsWhite ? Field.HasBlackPice(currentField) : Field.HasWhitePice(currentField)))
            {
                moveList.Add((endPos, null));
            }
            else if (currentField.AnPassan_able)
            {
                string anPs = (moveIsWhite ? DownOne(endPos) : UpOne(endPos));
                Field anPsField = BPToFi(state, anPs);
                if ((moveIsWhite ? anPsField.HasBlackPice() : anPsField.HasWhitePice()))
                {
                    moveList.Add((endPos, new List<FieldState> { new FieldState(anPs, anPsField) }));
                }
            }
        }

        public static void AddMovesTillEnd(GlobalState state, FieldState fromField, List<(string, List<FieldState>)> moveList, MoveFunction moveFunc)
        {
            bool validMove = true;
            string currentFieldName = fromField.FieldName;
            bool moveIsWhite = Field.HasWhitePice(fromField.Field);
            while (true)
            {
                currentFieldName = moveFunc(currentFieldName);
                if (!LegalPos(currentFieldName)) break;
                Field currentField = state.GetFieldAt(currentFieldName);
                validMove = !(moveIsWhite ? Field.HasWhitePice(currentField) : Field.HasBlackPice(currentField));

                if (validMove)
                {
                    bool canSeeField = true;
                    if (state.VisionRules.Enabled)
                    {
                        canSeeField = moveIsWhite ? state.CanWhiteSeeField(currentFieldName) : state.CanBlackSeeField(currentFieldName);
                    }
                    if (!canSeeField) break;
                    moveList.Add((currentFieldName, null));
                    if ((moveIsWhite ? Field.HasBlackPice(currentField) : Field.HasWhitePice(currentField)))
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }

        public static void AddCastelling(GlobalState state, FieldState fromField, List<(string, List<FieldState>)> moveList, MoveFunction moveFunc)
        {
            if (!fromField.Field.FirstMove) return;
            bool validMove = true;
            string currentFieldName = fromField.FieldName;
            Field currentField = new Field();
            (string, Field) lastField = (null, new Field());
            (string, Field) lastLastField = (null, new Field());
            bool moveIsWhite = Field.HasWhitePice(fromField.Field);
            while (true)
            {
                lastLastField = lastField;
                lastField = (currentFieldName, currentField);
                currentFieldName = moveFunc(currentFieldName);
                if (!LegalPos(currentFieldName)) break;
                currentField = state.GetFieldAt(currentFieldName);
                validMove = !(moveIsWhite ? Field.HasWhitePice(currentField) : Field.HasBlackPice(currentField));
                if (validMove)
                {
                    if ((moveIsWhite ? Field.HasBlackPice(currentField) : Field.HasWhitePice(currentField)))
                    {
                        break;
                    }
                }
                else if (currentField.Pice == (moveIsWhite ? Pices.WhiteRook : Pices.BlackRook) && currentField.Rokade_able)
                {
                    moveList.Add((lastField.Item1, new List<FieldState> { new FieldState(currentFieldName, currentField), new FieldState(lastLastField.Item1, lastLastField.Item2) }));
                    break;
                }
                else
                {
                    break;
                }
            }
        }

        public static Field BPToFi(GlobalState state, string pos)
        {
            return state.GetFieldAt(pos);
        }
        public static int BPToIndx(string pos)
        {
            return GlobalState.BoardPosToIndex[pos];
        }

        public static Dictionary<string, List<(string, List<FieldState>)>> GetLegalMoves(GlobalState state)
        {
            Dictionary<string, List<(string, List<FieldState>)>> legalMoves = new Dictionary<string, List<(string, List<FieldState>)>>();
            //IsColorFunction func;
            //if (state.WhiteTurn)
            //    func = Field.HasWhitePice;
            //else
            //    func = Field.HasBlackPice;

            foreach (var pos in GlobalState.BoardPosToIndex.Keys)
            {
                FieldState fs = new FieldState(pos, state.GetFieldAt(pos));
                //if (func(fs.Field))
                if (fs.Field.Pice != Pices.Non)
                {
                    legalMoves[pos] = GetLegalMoves(state, fs);
                }
            }
            return legalMoves;
        }

        public static List<(string, List<FieldState>)> GetLegalMoves(GlobalState state, FieldState fromField)
        {
            List<(string, List<FieldState>)> legalMoves = new List<(string, List<FieldState>)>();
            if (fromField.FieldName == null) return legalMoves;
            switch (fromField.Field.Pice)
            {
                case Pices.Non:
                    return legalMoves;
                case Pices.BlackPawn:
                    AddPwnAttack(state, fromField, legalMoves, LeftOne(DownOne(fromField.FieldName)));
                    AddPwnAttack(state, fromField, legalMoves, RightOne(DownOne(fromField.FieldName)));

                    string bpDown = DownOne(fromField.FieldName);
                    bool bpDownLegal = LegalPos(bpDown) && BPToFi(state, bpDown).Pice == Pices.Non;
                    if (bpDownLegal)
                    {
                        legalMoves.Add((bpDown, null));
                        if (!fromField.Field.FirstMove) break;

                        string bpDown2 = DownOne(bpDown);
                        Field bpDown2Field = BPToFi(state, bpDown2);
                        bool bpDown2Legal = LegalPos(bpDown2) && bpDown2Field.Pice == Pices.Non;
                        if (bpDown2Legal) legalMoves.Add((bpDown2, new List<FieldState> { new FieldState(bpDown, BPToFi(state, bpDown)) }));
                    }
                    break;
                case Pices.BlackKing:
                case Pices.WhiteKing:
                    AddMove(state, fromField, legalMoves, UpOne(fromField.FieldName));
                    AddMove(state, fromField, legalMoves, DownOne(fromField.FieldName));
                    AddMove(state, fromField, legalMoves, LeftOne(fromField.FieldName));
                    AddMove(state, fromField, legalMoves, RightOne(fromField.FieldName));

                    AddMove(state, fromField, legalMoves, UpOne(LeftOne(fromField.FieldName)));
                    AddMove(state, fromField, legalMoves, LeftOne(DownOne(fromField.FieldName)));
                    AddMove(state, fromField, legalMoves, DownOne(RightOne(fromField.FieldName)));
                    AddMove(state, fromField, legalMoves, RightOne(UpOne(fromField.FieldName)));

                    AddCastelling(state, fromField, legalMoves, RightOne);
                    AddCastelling(state, fromField, legalMoves, LeftOne);
                    break;
                case Pices.BlackQueen:
                case Pices.WhiteQueen:
                    AddMovesTillEnd(state, fromField, legalMoves, pos => UpOne(LeftOne(pos)));
                    AddMovesTillEnd(state, fromField, legalMoves, pos => UpOne(RightOne(pos)));
                    AddMovesTillEnd(state, fromField, legalMoves, pos => DownOne(LeftOne(pos)));
                    AddMovesTillEnd(state, fromField, legalMoves, pos => DownOne(RightOne(pos)));
                    AddMovesTillEnd(state, fromField, legalMoves, UpOne);
                    AddMovesTillEnd(state, fromField, legalMoves, DownOne);
                    AddMovesTillEnd(state, fromField, legalMoves, LeftOne);
                    AddMovesTillEnd(state, fromField, legalMoves, RightOne);
                    break;
                case Pices.BlackBishop:
                case Pices.WhiteBishop:
                    AddMovesTillEnd(state, fromField, legalMoves, pos => UpOne(LeftOne(pos)));
                    AddMovesTillEnd(state, fromField, legalMoves, pos => UpOne(RightOne(pos)));
                    AddMovesTillEnd(state, fromField, legalMoves, pos => DownOne(LeftOne(pos)));
                    AddMovesTillEnd(state, fromField, legalMoves, pos => DownOne(RightOne(pos)));
                    break;
                case Pices.BlackKnight:
                case Pices.WhiteKnight:
                    AddMove(state, fromField, legalMoves, UpOne(UpOne(LeftOne(fromField.FieldName))));
                    AddMove(state, fromField, legalMoves, UpOne(UpOne(RightOne(fromField.FieldName))));
                    AddMove(state, fromField, legalMoves, LeftOne(LeftOne(UpOne(fromField.FieldName))));
                    AddMove(state, fromField, legalMoves, LeftOne(LeftOne(DownOne(fromField.FieldName))));
                    AddMove(state, fromField, legalMoves, RightOne(RightOne(UpOne(fromField.FieldName))));
                    AddMove(state, fromField, legalMoves, RightOne(RightOne(DownOne(fromField.FieldName))));
                    AddMove(state, fromField, legalMoves, DownOne(DownOne(LeftOne(fromField.FieldName))));
                    AddMove(state, fromField, legalMoves, DownOne(DownOne(RightOne(fromField.FieldName))));
                    break;
                case Pices.BlackRook:
                case Pices.WhiteRook:
                    AddMovesTillEnd(state, fromField, legalMoves, UpOne);
                    AddMovesTillEnd(state, fromField, legalMoves, DownOne);
                    AddMovesTillEnd(state, fromField, legalMoves, LeftOne);
                    AddMovesTillEnd(state, fromField, legalMoves, RightOne);
                    break;
                case Pices.WhitePawn:
                    AddPwnAttack(state, fromField, legalMoves, LeftOne(UpOne(fromField.FieldName)));
                    AddPwnAttack(state, fromField, legalMoves, RightOne(UpOne(fromField.FieldName)));

                    string wpUp = UpOne(fromField.FieldName);
                    bool wpUpLegal = LegalPos(wpUp) && BPToFi(state, wpUp).Pice == Pices.Non;
                    if (wpUpLegal)
                    {
                        legalMoves.Add((wpUp, null));
                        if (!fromField.Field.FirstMove) break;

                        string wpUp2 = UpOne(wpUp);
                        Field wpUp2Field = BPToFi(state, wpUp2);
                        bool wpUp2Legal = LegalPos(wpUp2) && wpUp2Field.Pice == Pices.Non;
                        if (wpUp2Legal) legalMoves.Add((wpUp2, new List<FieldState> { new FieldState(wpUp, BPToFi(state, wpUp)) }));
                    }
                    break;
                default:
                    return legalMoves;
            }
            return legalMoves;
        }

        [Obsolete]
        public static SortedSet<string> GetVision(GlobalState state, bool whiteVision, VisionRules rules)
        {
            SortedSet<string> visionSet = new SortedSet<string>();
            if (rules.ViewMoveFields)
            {
                throw new NotImplementedException("addgagg");
            }
            if(rules.ViewRange > 0)
            {
                List<string> posesWithVision = GlobalState.BoardPosToIndex.Where((keyVal) => whiteVision ? state.GetField(keyVal.Value).HasWhitePice() : state.GetField(keyVal.Value).HasBlackPice()).Select(keyVal => keyVal.Key).ToList<string>();
                posesWithVision.ForEach(s => AddFromWithRange(visionSet, s, rules.ViewRange));
            }
            return visionSet;
        }

        

        public static void AddPiceVision(List<Field> board, int index, VisionRules vision, bool[] visionBoard)
        {
            int column = index % 8 - vision.ViewRange;
            int row = index / 8 - vision.ViewRange;
            //int start = index - visionRules.ViewRange - visionRules.ViewRange* 8;
            
            int iterate = vision.ViewRange * 2 + 1;
            for (int i = 0; i < iterate; i++)
            {
                if (!LegalColumn(column + i)) continue;
                for (int j = 0; j < iterate; j++)
                {
                    if (!LegalRow(row+j)) continue;
                    visionBoard[IndexFromColRow(column + i, row + j)] = true;
                }
            }
        }

        private static void AddFromWithRange(SortedSet<string> vision, string from, int range)
        {
            string start = from;
            for (int i = 0; i < range; i++){
                start = DownOne(LeftOne(start));
            }
            int iterate = range * 2 + 1;
            string current = start;
            for (int i = 0; i < iterate; i++)
            {
                for(int j = 0; j < iterate; j++)
                {
                    if(LegalPos(current))vision.Add(current);
                    current = UpOne(current);
                }
                start = RightOne(start);
                current = start;
            }
        }

        public static(Field, Field, Field, Field, Pices) Move(Field fromK, Field toK, Field fromR, Field toR)
        {
            if ((fromK.Pice == Pices.WhiteKing && fromR.Pice == Pices.WhiteRook) || (fromK.Pice == Pices.BlackKing && fromR.Pice == Pices.BlackRook))
            {
                return (new Field(Pices.Non), new Field(fromK.Pice), new Field(Pices.Non), new Field(fromR.Pice), Pices.Non);
            }
            else
            {
                throw new NotImplementedException("I only know of castling that can use this");
            }
        }
        public static (Field, Field, Field, Pices) Move(Field from, Field to, Field backup)
        {
            if (to.AnPassan_able)
            {
                //_killedPices.Add(backup.Pice);
                return (new Field(Pices.Non), new Field(from.Pice), new Field(Pices.Non), backup.Pice);
            }
            else if (from.Pice == Pices.WhitePawn || from.Pice == Pices.BlackPawn)
            {
                return (new Field(Pices.Non), new Field(from.Pice), new Field(Pices.Non, false, true, false, false, false), Pices.Non);
            }
            else
            {
                throw new NotImplementedException("Only an passan here");
            }
        }
        public static (Field, Field, Pices) Move(Field from, Field to)
        {
            return (new Field(Pices.Non), new Field(from.Pice), to.Pice);
        }
    }

    public class VisionRules
    {
        public bool Enabled { get; set; } = true;
        public bool ViewMoveFields { get; set; } = false;
        public bool ViewCaptureField { get; set; } = false;
        //public bool View
        public int ViewRange { get; set; } = 2;
        public Dictionary<Pices, VisionRules> PiceOverwrite { get; set; }
    }
}
