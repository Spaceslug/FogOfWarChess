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
        public List<FieldState> Board { get; private set; } = new List<FieldState>(64);
        
        public void UpdateState(GlobalState oldState)
        {
            Board = new List<FieldState>(64);
            Board.AddRange(oldState.Board);

        }

        public static GlobalState CreateStartState()
        {
            var globalState = new GlobalState();
            globalState.Board.Add(new FieldState(Pices.WhiteRook, false, false, true));
            globalState.Board.Add(new FieldState(Pices.WhiteKnight));
            globalState.Board.Add(new FieldState(Pices.WhiteBishop));
            globalState.Board.Add(new FieldState(Pices.WhiteQueen));
            globalState.Board.Add(new FieldState(Pices.WhiteKing, false, false, true));
            globalState.Board.Add(new FieldState(Pices.WhiteBishop));
            globalState.Board.Add(new FieldState(Pices.WhiteKnight));
            globalState.Board.Add(new FieldState(Pices.WhiteRook, false, false, true));
            //
            globalState.Board.Add(new FieldState(Pices.WhitePawn, true, false, false));
            globalState.Board.Add(new FieldState(Pices.WhitePawn, true, false, false));
            globalState.Board.Add(new FieldState(Pices.WhitePawn, true, false, false));
            globalState.Board.Add(new FieldState(Pices.WhitePawn, true, false, false));
            globalState.Board.Add(new FieldState(Pices.WhitePawn, true, false, false));
            globalState.Board.Add(new FieldState(Pices.WhitePawn, true, false, false));
            globalState.Board.Add(new FieldState(Pices.WhitePawn, true, false, false));
            globalState.Board.Add(new FieldState(Pices.WhitePawn, true, false, false));
            //
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            //
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            //
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            //
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            globalState.Board.Add(new FieldState(Pices.Non));
            //
            globalState.Board.Add(new FieldState(Pices.BlackPawn, true, false, false));
            globalState.Board.Add(new FieldState(Pices.BlackPawn, true, false, false));
            globalState.Board.Add(new FieldState(Pices.BlackPawn, true, false, false));
            globalState.Board.Add(new FieldState(Pices.BlackPawn, true, false, false));
            globalState.Board.Add(new FieldState(Pices.BlackPawn, true, false, false));
            globalState.Board.Add(new FieldState(Pices.BlackPawn, true, false, false));
            globalState.Board.Add(new FieldState(Pices.BlackPawn, true, false, false));
            globalState.Board.Add(new FieldState(Pices.BlackPawn, true, false, false));
            //
            globalState.Board.Add(new FieldState(Pices.BlackRook, false, false, true));
            globalState.Board.Add(new FieldState(Pices.BlackKnight));
            globalState.Board.Add(new FieldState(Pices.BlackBishop));
            globalState.Board.Add(new FieldState(Pices.BlackQueen));
            globalState.Board.Add(new FieldState(Pices.BlackKing, false, false, true));
            globalState.Board.Add(new FieldState(Pices.BlackBishop));
            globalState.Board.Add(new FieldState(Pices.BlackKnight));
            globalState.Board.Add(new FieldState(Pices.BlackRook, false, false, true));
            return globalState;
        }
    }

    public enum Pices
    {
        Non,
        BlackKing,
        BlackQueen,
        BlackRook,
        BlackBishop,
        BlackKnight,
        BlackPawn,
        WhiteKing,
        WhiteQueen,
        WhiteRook,
        WhiteBishop,
        WhiteKnight,
        WhitePawn

    }
    public struct FieldState
    {
        public bool AnPassan_creating;
        public bool AnPassan_able;
        public bool Rokade_able;
        public Pices Pice;

        public FieldState(Pices pice, bool anPassanCreating, bool anPassanAble, bool rokadeAble)
        {
            Pice = pice;
            AnPassan_creating = anPassanCreating;
            AnPassan_able = anPassanAble;
            Rokade_able = rokadeAble;
        }

        public FieldState(Pices pice) : this(pice, false, false, false)
        {

        }

    }
}
