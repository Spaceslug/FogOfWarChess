using System;
using Avalonia.Controls;
using Avalonia.Controls.Templates;
using SlugChessAval.ViewModels;

namespace SlugChessAval
{
    public class ViewLocator : IDataTemplate
    {
        public bool SupportsRecycling => false;

        public IControl Build(object data)
        {
#pragma warning disable CS8602 // Dereference of a possibly null reference.
            var name = data.GetType().FullName.Replace("ViewModel", "View");
#pragma warning restore CS8602 // Dereference of a possibly null reference.
            var type = Type.GetType(name);

            if (type != null)
            {
#pragma warning disable CS8600 // Converting null literal or possible null value to non-nullable type.
#pragma warning disable CS8603 // Possible null reference return.
                return (Control)Activator.CreateInstance(type);
#pragma warning restore CS8603 // Possible null reference return.
#pragma warning restore CS8600 // Converting null literal or possible null value to non-nullable type.
            }
            else
            {
                return new TextBlock { Text = "Not Found: " + name };
            }
        }

        public bool Match(object data)
        {
            return data is ViewModelBase;
        }
    }
}