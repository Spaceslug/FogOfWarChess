using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DarkChess
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
        private static string UpOne(string startPos)
        {
            return startPos[0] + char.ConvertFromUtf32(((int)startPos[1]) + 1);
        }
        private static string DownOne(string startPos)
        {
            return startPos[0] + char.ConvertFromUtf32(((int)startPos[1]) - 1);
        }
        private static string LeftOne(string startPos)
        {
            return char.ConvertFromUtf32(((int)startPos[0]) - 1) + startPos[1];
        }
        private static string RightOne(string startPos)
        {
            return char.ConvertFromUtf32(((int)startPos[0]) + 1) + startPos[1];
        }

        public static void AddMove(GlobalState state, FieldState fromField, List<(string, List<FieldState>)> moveList, string endPos)
        {
            bool moveIsWhite = Field.HasWhitePice(fromField.Field);
            if (!LegalPos(endPos)) return;
            Field currentField = state.Board[MainWindow.BoardPosToIndex[endPos]];
            if (!(moveIsWhite ? Field.HasWhitePice(currentField) : Field.HasBlackPice(currentField)))
            {
                moveList.Add((endPos, null));
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
                Field currentField = state.Board[MainWindow.BoardPosToIndex[currentFieldName]];
                validMove = !(moveIsWhite?Field.HasWhitePice(currentField):Field.HasBlackPice(currentField));
                if (validMove)
                {
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
                currentField = state.Board[MainWindow.BoardPosToIndex[currentFieldName]];
                validMove = !(moveIsWhite ? Field.HasWhitePice(currentField) : Field.HasBlackPice(currentField));
                if (validMove)
                {
                    if ((moveIsWhite ? Field.HasBlackPice(currentField) : Field.HasWhitePice(currentField)))
                    {
                        break;
                    }
                }
                else if(currentField.Pice == (moveIsWhite ? Pices.WhiteRook:Pices.BlackRook) && currentField.Rokade_able)
                {
                    moveList.Add((lastField.Item1, new List<FieldState> { new FieldState(currentFieldName, currentField), new FieldState(lastLastField.Item1, lastLastField.Item2)}));
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
            return state.Board[MainWindow.BoardPosToIndex[pos]];
        }

        public static List<(string, List<FieldState>)> GetLegalMoves(GlobalState state, FieldState fromField)
        {
            List<(string, List<FieldState>)> legalMoves = new List<(string, List<FieldState>)>();
            switch (fromField.Field.Pice)
            {
                case Pices.Non:
                    return legalMoves;
                case Pices.BlackPawn:
                    string bpDown = DownOne(fromField.FieldName);
                    bool bpDownLegal = LegalPos(bpDown) && !Field.HasBlackPice(state.Board[MainWindow.BoardPosToIndex[bpDown]]);
                    if (bpDownLegal)
                    {
                        legalMoves.Add((bpDown, null));

                        string bpDown2 = UpOne(bpDown);
                        bool bpDown2Legal = fromField.Field.FirstMove && LegalPos(bpDown2) && !Field.HasBlackPice(state.Board[MainWindow.BoardPosToIndex[bpDown2]]);
                        if (bpDown2Legal) legalMoves.Add((bpDown2, null));
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
                    string wpUp = UpOne(fromField.FieldName);
                    bool wpUpLegal = LegalPos(wpUp) && !Field.HasWhitePice(state.Board[MainWindow.BoardPosToIndex[wpUp]]);
                    if (wpUpLegal)
                    {
                        legalMoves.Add((wpUp, null));

                        string wpUp2 = UpOne(wpUp);
                        bool wpUp2Legal = fromField.Field.FirstMove && LegalPos(wpUp2) && !Field.HasWhitePice(state.Board[MainWindow.BoardPosToIndex[wpUp2]]);
                        if (wpUp2Legal) legalMoves.Add((wpUp2, null));
                    }
                    break;
                default:
                    return legalMoves;
            }
            return legalMoves;
        }
    }
}
