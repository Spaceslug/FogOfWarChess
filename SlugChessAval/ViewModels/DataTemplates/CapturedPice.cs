using Avalonia.Media;
using Avalonia.Media.Imaging;
using Google.Protobuf.WellKnownTypes;
using SlugChessAval.Models;
using SlugChessAval.Services;
using System;
using System.Collections.Generic;
using System.Text;

namespace SlugChessAval.ViewModels.DataTemplates
{
    public class CapturedPice
    {
        public SolidColorBrush Background { get; }
        public IBitmap? Pice { get; }
        public string Field { get; }
        public CapturedPice(ChessCom.PiceCapture pc)
        {
            Background = (ChessboardModel.FieldColorLight(pc.Location) ? StyleMirror.WhiteField : StyleMirror.BlackField) ?? new SolidColorBrush(Colors.Pink);
            Pice = AssetBank.ImageFromPice(pc.Pice);
            Field = pc.Location;
        }
        private CapturedPice(SolidColorBrush b, IBitmap? p, string f)
        {
            Background = b;
            Pice = p;
            Field = f;
        }

        public static CapturedPice Empty => new CapturedPice(new SolidColorBrush { Opacity=0}, null, "");

        public static IEnumerable<CapturedPice> CapturedPices => new List<CapturedPice> { 
            new CapturedPice(new ChessCom.PiceCapture { Location="a4", Pice=ChessCom.Pices.WhitePawn}),
            new CapturedPice(new ChessCom.PiceCapture { Location="b4", Pice=ChessCom.Pices.BlackBishop}),
            new CapturedPice(new ChessCom.PiceCapture { Location="c4", Pice=ChessCom.Pices.BlackPawn}),
            new CapturedPice(new ChessCom.PiceCapture { Location="e4", Pice=ChessCom.Pices.WhiteQueen}),
            new CapturedPice(new ChessCom.PiceCapture { Location="f4", Pice=ChessCom.Pices.WhiteKnight}),
            new CapturedPice(new ChessCom.PiceCapture { Location="h4", Pice=ChessCom.Pices.BlackRook})
        };
    }
}
