using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace SlugChess
{
    public class ClearBoardCommand : ICommand
    {
        public void Execute(object parameter)
        {
            MainWindow.Instance.ClearBoard();
        }

        public bool CanExecute(object parameter)
        {
            return true;
        }

        public event EventHandler CanExecuteChanged;
    }
}
