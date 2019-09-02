using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace SlugChess
{
    public class MyLittleFuckingDataContext
    {
        ICommand _clearBoardCommand = new ClearBoardCommand();

        public ICommand ClearBoardCommand
        {
            get { return _clearBoardCommand; }
        }
    }
}
