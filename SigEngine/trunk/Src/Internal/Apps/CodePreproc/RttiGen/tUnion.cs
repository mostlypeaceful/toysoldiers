using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;

namespace CodePreproc.RttiGen
{
    class tUnion : tParser
    {
        string mUnionName = "";

        public tUnion(string tagAndNameText, string bodyText)
        {
            fParseTagAndName(tagAndNameText);
            fParseBody(bodyText);
        }

        public string fGetSimpleDeclaration()
        {
            return "union " + mUnionName;
        }

        private void fParseTagAndName(string text)
        {
            // store the name for which "type" of class (i.e., struct or class)
            Match match = mRegExUnionType.Match(text);
            if (!match.Success)
                throw new System.Exception("invalid union declaration");

            // store the actual name of the class (i.e., this is the c++ type)
            string next = text.Substring(match.Index + match.Length);
            match = mRegExName.Match(next);
            if (!match.Success)
                mUnionName = "Anonymous";
            else
                mUnionName = next.Substring(match.Index, match.Length).Trim();
        }

        private void fParseBody(string text)
        {
            // TODO
        }
    }
}
