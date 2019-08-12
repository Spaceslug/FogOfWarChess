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

        public static List<string> FindMoveTillEnd(GlobalState state, FieldState fromField, MoveFunction moveFunc)
        {
            List<string> moves = new List<string>();
            bool validMove = true;
            string currentFieldName = fromField.FieldName;
            bool moveIsWhite = Field.HasWhitePice(fromField.Field);
            while (true)
            {
                currentFieldName = moveFunc(currentFieldName);
                Field currentField = state.Board[MainWindow.BoardPosToIndex[currentFieldName]];
                validMove = LegalPos(currentFieldName) && !(moveIsWhite?Field.HasWhitePice(currentField):Field.HasBlackPice(currentField));
                if (validMove)
                {
                    moves.Add(currentFieldName);
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
            return moves;
        }

        public static Field BPToFi(GlobalState state, string pos)
        {
            return state.Board[MainWindow.BoardPosToIndex[pos]];
        }

        public static List<string> GetLegalMoves(GlobalState state, FieldState fromField)
        {
            List<string> legalMoves = new List<string>();
            switch (fromField.Field.Pice)
            {
                case Pices.Non:
                    return new List<string>();
                case Pices.BlackKing:
                    //if(LegalPos(UpOne(fromField.)))
                    break;
                case Pices.BlackQueen:
                    //grid.Children.Add(BlackQueen);
                    break;
                case Pices.BlackBishop:
                    //grid.Children.Add(BlackBishop);
                    break;
                case Pices.BlackKnight:
                    //grid.Children.Add(BlackKnight);
                    break;
                case Pices.BlackRook:
                    //grid.Children.Add(BlackRook);
                    break;
                case Pices.BlackPawn:
                    string bpDown = DownOne(fromField.FieldName);
                    bool bpDownLegal = LegalPos(bpDown) && !Field.HasBlackPice(state.Board[MainWindow.BoardPosToIndex[bpDown]]);
                    if (bpDownLegal)
                    {
                        legalMoves.Add(bpDown);

                        string bpDown2 = UpOne(bpDown);
                        bool bpDown2Legal = fromField.Field.FirstMove && LegalPos(bpDown2) && !Field.HasBlackPice(state.Board[MainWindow.BoardPosToIndex[bpDown2]]);
                        if (bpDown2Legal) legalMoves.Add(bpDown2);
                    }
                    break;
                case Pices.WhiteKing:
                    string up = UpOne(fromField.FieldName);
                    if (LegalPos(up) && !BPToFi(state, up).HasWhitePice()) legalMoves.Add(up);
                    string down = DownOne(fromField.FieldName);
                    if (LegalPos(down) && !BPToFi(state, down).HasWhitePice()) legalMoves.Add(down);
                    string left = LeftOne(fromField.FieldName);
                    if (LegalPos(left) && !BPToFi(state, left).HasWhitePice()) legalMoves.Add(left);
                    string right = RightOne(fromField.FieldName);
                    if (LegalPos(right) && !BPToFi(state, right).HasWhitePice()) legalMoves.Add(right);

                    string upright = RightOne(UpOne(fromField.FieldName));
                    if (LegalPos(upright) && !BPToFi(state, upright).HasWhitePice()) legalMoves.Add(upright);
                    string downLeft = LeftOne(DownOne(fromField.FieldName));
                    if (LegalPos(downLeft) && !BPToFi(state, downLeft).HasWhitePice()) legalMoves.Add(downLeft);
                    string leftUp = UpOne(LeftOne(fromField.FieldName));
                    if (LegalPos(leftUp) && !BPToFi(state, leftUp).HasWhitePice()) legalMoves.Add(leftUp);
                    string rightDown = DownOne(RightOne(fromField.FieldName));
                    if (LegalPos(rightDown) && !BPToFi(state, rightDown).HasWhitePice()) legalMoves.Add(rightDown);
                    break;
                case Pices.WhiteQueen:
                    //grid.Children.Add(WhiteQueen);
                    break;
                case Pices.WhiteBishop:
                    //grid.Children.Add(WhiteBishop);
                    break;
                case Pices.WhiteKnight:
                    //grid.Children.Add(WhiteKnight);
                    break;
                case Pices.WhiteRook:
                    //grid.Children.Add(WhiteRook);
                    break;
                case Pices.WhitePawn:
                    string wpUp = UpOne(fromField.FieldName);
                    bool wpUpLegal = LegalPos(wpUp) && !Field.HasWhitePice(state.Board[MainWindow.BoardPosToIndex[wpUp]]);
                    if (wpUpLegal)
                    {
                        legalMoves.Add(wpUp);

                        string wpUp2 = UpOne(wpUp);
                        bool wpUp2Legal = fromField.Field.FirstMove && LegalPos(wpUp2) && !Field.HasWhitePice(state.Board[MainWindow.BoardPosToIndex[wpUp2]]);
                        if (wpUp2Legal) legalMoves.Add(wpUp2);
                    }
                    break;
                default:
                    return new List<string>();
            }
            return legalMoves;
        }
    }
}
