using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;

namespace CodePreproc.RttiGen
{
    /// <summary>
    /// Known issues:
    /// * Doesn't support anonymous classes/unions/enums; this.
    /// * Doesn't properly handle inline function pointers (but is fine if you use a typedef first).
    /// </summary>
    class tReflectionGenerator : tParser
    {
        public tFile fGenerateFile(string inputFolderName, string inputFileName)
        {
            if (inputFileName == null || inputFileName.Length == 0)
            {
                Console.WriteLine("Invalid (null) input file name supplied to class_desc_gen module");
                return null;
            }
            if (!System.IO.File.Exists(inputFileName))
            {
                Console.WriteLine("The input file specified to tReflectionGenerator module does not exist");
                return null;
            }

            string inputFileContents;

            try
            {
                // read in whole input file to string
                inputFileContents = System.IO.File.ReadAllText(inputFileName);

                // decompose the file into namespaces and classes
                return new tFile(inputFolderName, inputFileName, inputFileContents);
            }
            catch (System.Exception)
            {
                // some kind of general badness, usually file in use, or else
                // some kind of input format violations or something
            }

            return null;
        }

    }
}
