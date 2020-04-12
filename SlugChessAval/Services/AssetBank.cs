using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using Avalonia.Controls;
using Svg.Skia.Avalonia;

namespace SlugChessAval.Services
{
    public class AssetBank
    {
        private static string _assetFolder = "Asset\\";

        private static Dictionary<string, SvgImage> _loadedBitmaps = new Dictionary<string, SvgImage>();

        public static Task<bool> LoadAssets() => Task.Run<bool>(()=>
       {
           foreach(var file in Directory.EnumerateFiles(_assetFolder))
           {
               if (file.ToLower().EndsWith(".svg"))
               {
                   var svg = new SvgSkia();
                  svg.Load(file);
                   _loadedBitmaps[Path.GetFileNameWithoutExtension(file)] = new SvgImage { Source=svg };
                   var r = new Image();
                   r.Source = new SvgImage { Source = svg };
               }
           }
           return true;
       });

        public static Image WhitePawnImage => new Image { Source=_loadedBitmaps["whitePawn"] };
    }
}
