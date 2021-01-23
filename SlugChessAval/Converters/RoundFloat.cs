using System;
using System.Collections.Generic;
using System.Globalization;
using System.Text;

namespace SlugChessAval.Converters
{
    public class RoundFloat : Avalonia.Data.Converters.IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            var f = (float)value;
            return Math.Round(f).ToString();
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
