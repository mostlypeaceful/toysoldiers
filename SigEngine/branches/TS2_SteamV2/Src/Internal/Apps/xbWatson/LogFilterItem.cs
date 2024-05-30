using System.Drawing;

namespace Atg.Samples.xbWatson
{
    class LogFilterItem
    {
        private string name;
        private Color textColor;

        public LogFilterItem(string _name, Color _textColor)
        {
            name = _name;
            textColor = _textColor;
        }

        public LogFilterItem(string _name)
        {
            name = _name;
            textColor = Color.Black;
        }

        public string Name
        {
            get { return name; }
            set { name = value;  }
        }

        public Color Color
        {
            get { return textColor; }
            set { textColor = value; }
        }

        public override string ToString()
        {
            return name;
        }
    }
}
