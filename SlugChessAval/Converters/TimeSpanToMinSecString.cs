using System;
using System.Collections.Generic;
using System.Globalization;
using System.Text;

namespace SlugChessAval.Converters
{
    public class TimeSpanToMinSecString : Avalonia.Data.Converters.IValueConverter
    {

        public static TimeSpanToMinSecString instance = new TimeSpanToMinSecString();

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            TimeSpan timeSpan = (TimeSpan)value;
            return timeSpan.ToString(@"mm\:ss");
        }

        public object? ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return null;
        }
    }
}
