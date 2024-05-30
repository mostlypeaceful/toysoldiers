using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;

namespace CodePreproc.RttiGen
{
    class tFile : tParser
    {
        string mFullName;
        string mIncludeName;
        tNamespace mGlobalNamespace;

        public tFile(string inputFolderName, string fileName, string text)
        {
            mFullName = fileName;

            mIncludeName = fileName.Replace(inputFolderName, "");
            mIncludeName = mIncludeName.Replace("\\", "/");
            if (mIncludeName[0] == '/')
                mIncludeName = mIncludeName.Substring(1);

            // first off, clean all comments out of the file
            text = mRegExCComment.Replace(text, "");
            text = mRegExCppComment.Replace(text, "");

            // create the global namespace, and parse recursively
            mGlobalNamespace = new tNamespace(null, text);
        }

        public void fOutput(System.IO.StreamWriter outputFileStream, bool templates)
        {
            if (templates && !fHasTemplates())
                return;
            if (!templates && !fHasNonTemplates())
                return;

            outputFileStream.WriteLine();
            outputFileStream.WriteLine("//___________________________________________________________________________________________");
            outputFileStream.WriteLine("// Generated from file [" + mFullName.Substring( mFullName.IndexOf( "src", StringComparison.InvariantCultureIgnoreCase ) ) + "]");
            outputFileStream.WriteLine("//");
            outputFileStream.WriteLine("#include \"" + mIncludeName + "\"");
            mGlobalNamespace.fOutput(outputFileStream, templates);
            outputFileStream.WriteLine();
        }

        public bool fHasReflectedClass()
        {
            return mGlobalNamespace.fHasReflectedClass();
        }
        
        public bool fHasTemplates()
        {
            return mGlobalNamespace.fHasTemplates();
        }

        public bool fHasNonTemplates()
        {
            return mGlobalNamespace.fHasNonTemplates();
        }
    }
}
