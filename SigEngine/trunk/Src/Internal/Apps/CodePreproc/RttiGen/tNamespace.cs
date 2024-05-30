using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;

namespace CodePreproc.RttiGen
{

    class tNamespace : tParser
    {
        string mName = "";
        public List<tClass> mClasses = new List<tClass>();
        public List<tNamespace> mNamespaces = new List<tNamespace>();

        public tNamespace(string tagAndNameText, string bodyText)
        {
            if(tagAndNameText!=null)
                fParseTagAndName(tagAndNameText);
            fParseBody(bodyText);
        }

        public void fOutput(System.IO.StreamWriter outputFileStream, bool templates)
        {
            if (templates && !fHasTemplates())
                return;
            if (!templates && !fHasNonTemplates())
                return;

            if (mName.Length > 0)
            {
                outputFileStream.WriteLine("namespace " + mName);
                outputFileStream.WriteLine("{");
            }

            for (int i = 0; i < mClasses.Count; ++i)
                if (mClasses[i].fHasReflectedClass())
                    mClasses[i].fOutput(outputFileStream, templates);

            for (int i = 0; i < mNamespaces.Count; ++i)
                if (mNamespaces[i].fHasReflectedClass())
                    mNamespaces[i].fOutput(outputFileStream, templates);

            if (mName.Length > 0)
                outputFileStream.WriteLine("}");
        }

        public bool fHasReflectedClass()
        {
            for (int i = 0; i < mClasses.Count; ++i)
                if (mClasses[i].fHasReflectedClass())
                    return true;
            for (int i = 0; i < mNamespaces.Count; ++i)
                if (mNamespaces[i].fHasReflectedClass())
                    return true;
            return false;
        }

        public bool fHasTemplates()
        {
            for (int i = 0; i < mClasses.Count; ++i)
                if (mClasses[i].fHasTemplates())
                    return true;
            for (int i = 0; i < mNamespaces.Count; ++i)
                if (mNamespaces[i].fHasTemplates())
                    return true;
            return false;
        }

        public bool fHasNonTemplates()
        {
            for (int i = 0; i < mClasses.Count; ++i)
                if (mClasses[i].fHasNonTemplates())
                    return true;
            for (int i = 0; i < mNamespaces.Count; ++i)
                if (mNamespaces[i].fHasNonTemplates())
                    return true;
            return false;
        }

        private void fParseTagAndName(string text)
        {
            Match match = mRegExNamespace.Match(text);
            if (!match.Success)
                throw new System.Exception("invalid namespace declaration");
            string next = text.Substring(match.Index + match.Length);
            match = mRegExName.Match(next);
            if (!match.Success)
                throw new System.Exception("invalid class declaration");
            mName = next.Substring(match.Index, match.Length).Trim();
        }

        private string fAddNamespace(string tagAndName, string body)
        {
            mNamespaces.Add(new tNamespace(tagAndName, body));
            return "";
        }

        private string fAddClass(string tagAndName, string body)
        {
            tClass cl = new tClass(tagAndName, body);
            mClasses.Add(cl);
            return cl.fGetSimpleDeclaration();
        }

        private void fParseBody(string text)
        {
            text = fParseBlock(text, mRegExNamespaceDecl, fAddNamespace);
            text = fParseBlock(text, mRegExClassDecl, fAddClass);
        }
    }

}
