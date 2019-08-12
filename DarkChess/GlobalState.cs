using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DarkChess
{

    public class GlobalState
    {
        /// <summary>
        /// a1 = 0, a8 = 7, b1 = 8, h8 = 63
        /// </summary>
        public List<Field> Board { get; set; } = new List<Field>(64);
        public string Selected { get; set; } = null;
        public bool WhiteTurn { get; set; } = true;


        //public void UpdateState(GlobalState oldState)
        //{
        //    Board = new List<Field>(64);
        //    Board.AddRange(oldState.Board);
        //    Selected = oldState.Selected;

        //}

        public static GlobalState CreateStartState()
        {
            var globalState = new GlobalState();
            globalState.Board.Add(new Field(Pices.WhiteRook, false, false, true, true));
            globalState.Board.Add(new Field(Pices.WhiteKnight, false, false, false, true));
            globalState.Board.Add(new Field(Pices.WhiteBishop, false, false, false, true));
            globalState.Board.Add(new Field(Pices.WhiteQueen, false, false, false, true));
            globalState.Board.Add(new Field(Pices.WhiteKing, false, false, true, true));
            globalState.Board.Add(new Field(Pices.WhiteBishop, false, false, false, true));
            globalState.Board.Add(new Field(Pices.WhiteKnight, false, false, false, true));
            globalState.Board.Add(new Field(Pices.WhiteRook, false, false, true, true));
            //
            globalState.Board.Add(new Field(Pices.WhitePawn, true, false, false, true));
            globalState.Board.Add(new Field(Pices.WhitePawn, true, false, false, true));
            globalState.Board.Add(new Field(Pices.WhitePawn, true, false, false, true));
            globalState.Board.Add(new Field(Pices.WhitePawn, true, false, false, true));
            globalState.Board.Add(new Field(Pices.WhitePawn, true, false, false, true));
            globalState.Board.Add(new Field(Pices.WhitePawn, true, false, false, true));
            globalState.Board.Add(new Field(Pices.WhitePawn, true, false, false, true));
            globalState.Board.Add(new Field(Pices.WhitePawn, true, false, false, true));
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
            globalState.Board.Add(new Field(Pices.BlackPawn, true, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackPawn, true, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackPawn, true, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackPawn, true, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackPawn, true, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackPawn, true, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackPawn, true, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackPawn, true, false, false, true));
            //
            globalState.Board.Add(new Field(Pices.BlackRook, false, false, true, true));
            globalState.Board.Add(new Field(Pices.BlackKnight, false, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackBishop, false, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackQueen, false, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackKing, false, false, true, true));
            globalState.Board.Add(new Field(Pices.BlackBishop, false, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackKnight, false, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackRook, false, false, true, true));
            return globalState;
        }

        public static GlobalState CreateStartStateNoPawns()
        {
            var globalState = new GlobalState();
            globalState.Board.Add(new Field(Pices.WhiteRook, false, false, true, true));
            globalState.Board.Add(new Field(Pices.WhiteKnight, false, false, false, true));
            globalState.Board.Add(new Field(Pices.WhiteBishop, false, false, false, true));
            globalState.Board.Add(new Field(Pices.WhiteQueen, false, false, false, true));
            globalState.Board.Add(new Field(Pices.WhiteKing, false, false, true, true));
            globalState.Board.Add(new Field(Pices.WhiteBishop, false, false, false, true));
            globalState.Board.Add(new Field(Pices.WhiteKnight, false, false, false, true));
            globalState.Board.Add(new Field(Pices.WhiteRook, false, false, true, true));
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
            globalState.Board.Add(new Field(Pices.BlackRook, false, false, true, true));
            globalState.Board.Add(new Field(Pices.BlackKnight, false, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackBishop, false, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackQueen, false, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackKing, false, false, true, true));
            globalState.Board.Add(new Field(Pices.BlackBishop, false, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackKnight, false, false, false, true));
            globalState.Board.Add(new Field(Pices.BlackRook, false, false, true, true));
            return globalState;
        }
    }

    public enum Pices
    {
        Non,
        BlackPawn,
        BlackKnight,
        BlackBishop,
        BlackRook,
        BlackQueen,
        BlackKing,
        WhitePawn,
        WhiteKnight,
        WhiteBishop,
        WhiteRook,
        WhiteQueen,
        WhiteKing,
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
        public Pices Pice;

        public Field(Pices pice, bool anPassanCreating, bool anPassanAble, bool rokadeAble, bool firstMove)
        {
            Pice = pice;
            AnPassan_creating = anPassanCreating;
            AnPassan_able = anPassanAble;
            Rokade_able = rokadeAble;
            FirstMove = firstMove;
        }

        public Field(Pices pice) : this(pice, false, false, false, false)
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
