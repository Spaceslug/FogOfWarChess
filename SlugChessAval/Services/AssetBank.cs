﻿using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
//using Svg.Skia.Avalonia;

namespace SlugChessAval.Services
{
    public static class AssetBank
    {
        private static readonly string _assetFolder = "Asset/";
        private static readonly string _builtInAssetFolder = "avares://SlugChessAval/Assets/";

        public static IBitmap GetImage(string s) => _loadedBitmaps[s];
        private static readonly Dictionary<string, IBitmap> _loadedBitmaps = new Dictionary<string, IBitmap>();

        public static Task<bool> LoadAssets() => Task.Run<bool>(()=>
        {
            var assetLoader = AvaloniaLocator.Current.GetService<IAssetLoader>();
            foreach (var assetUri in assetLoader.GetAssets(new Uri(_builtInAssetFolder), null))
            {
                if (assetUri.AbsolutePath.ToLower().EndsWith(".png"))
                {
                    _loadedBitmaps[Path.GetFileNameWithoutExtension(assetUri.AbsolutePath)] = new Bitmap(assetLoader.Open(assetUri));
                }
            }
            if (Directory.Exists(_assetFolder))
            {
                foreach (var file in Directory.EnumerateFiles(_assetFolder))
                {
                    if (file.ToLower().EndsWith(".svg"))
                    {
                        // var svg = new SvgSkia();
                        //svg.Load(file);
                        // _loadedBitmaps[Path.GetFileNameWithoutExtension(file)] = new SvgImage { Source=svg };
                        // var r = new Image();
                        // r.Source = new SvgImage { Source = svg };
                    }
                    else if (file.ToLower().EndsWith(".png"))
                    {
                        _loadedBitmaps[Path.GetFileNameWithoutExtension(file)] = new Bitmap(file);
                    }
                }
            }
            
            return true;
        });

        public static Image WhitePawnImage => new Image { Source=_loadedBitmaps["whitePawn"] };
        public static IBitmap? ImageFromPice(ChessCom.Pices pice) =>
            pice switch
            {
                ChessCom.Pices.None => null,
                ChessCom.Pices.BlackKing => AssetBank.GetImage("blackKing"),
                ChessCom.Pices.BlackQueen => AssetBank.GetImage("blackQueen"),
                ChessCom.Pices.BlackRook => AssetBank.GetImage("blackRook"),
                ChessCom.Pices.BlackBishop => AssetBank.GetImage("blackBishop"),
                ChessCom.Pices.BlackKnight => AssetBank.GetImage("blackKnight"),
                ChessCom.Pices.BlackPawn => AssetBank.GetImage("blackPawn"),
                ChessCom.Pices.WhiteKing => AssetBank.GetImage("whiteKing"),
                ChessCom.Pices.WhiteQueen => AssetBank.GetImage("whiteQueen"),
                ChessCom.Pices.WhiteRook => AssetBank.GetImage("whiteRook"),
                ChessCom.Pices.WhiteBishop => AssetBank.GetImage("whiteBishop"),
                ChessCom.Pices.WhiteKnight => AssetBank.GetImage("whiteKnight"),
                ChessCom.Pices.WhitePawn => AssetBank.GetImage("whitePawn"),
                _ => throw new ArgumentException(message: "invalid enum value", paramName: nameof(pice)),
            };
    }
}
