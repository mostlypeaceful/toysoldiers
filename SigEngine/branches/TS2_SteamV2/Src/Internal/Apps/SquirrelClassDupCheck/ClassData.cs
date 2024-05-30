using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace SquirrelClassDupCheck
{
    class ClassData
    {
        private string name = string.Empty;
        public string Name
        {
            get
            {
                return name;
            }
            set
            {
                name = value;
            }
        }

        public List<string> files = new List<string>();
    }
}
