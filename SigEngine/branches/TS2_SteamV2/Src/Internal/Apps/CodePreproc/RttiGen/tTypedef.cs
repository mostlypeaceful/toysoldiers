using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;

namespace CodePreproc.RttiGen
{
    class tTypedef : tParser
    {
        string mTypedef="";

        public tTypedef(string typedefString)
        {
            mTypedef = typedefString;
            // TODO further parse the string if we want to separate out its types?
        }
    }
}
