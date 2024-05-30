using System;
using System.Collections.Generic;
using System.Text;
using CodePreproc.RttiGen;

namespace CodePreproc
{
    class Program
    {
        static void Main(string[] args)
        {
            string opts = "-i <inputFolderName> # [optional] Program takes as input the .h/.hpp/.inl files in the specified folder." +
                                                "  If not specified, uses current directory.\n" +
                          "-o <outputFileName>  # [optional] Creates two output files, <outputFileName>.hpp and <outputFileName>.cpp." +
                                                "  Places output in these files.  If not specified, uses <current directory name>.rtti.hpp/.cpp.\n" +
                          "-r  # [optional] Scans recursively.\n" +
                          "-e  # [optional] Explicit scan only; only use file search expressions specified with -f.\n" +
                          "-f  # [optional] Adds a file search expression to scan for as input (i.e., *.*, or *.h, or file.txt).  You can add multiple.\n" +
                          "-h  # [optional] Add the name of a file to be included in the generated .cpp rtti file.\n";

            //
            // setup defaults
            //

            string inputFolderName = System.IO.Directory.GetCurrentDirectory( );
            string outputFileName = "";
            bool recursive = false;
            bool explicitSearch = false;

            List<string> inputFileExts = new List<string>();
            inputFileExts.Add("*.h");
            inputFileExts.Add("*.hpp");
            inputFileExts.Add("*.inl");
            List<string> moreFileExts = new List<string>();

            List<string> filesToInclude = new List<string>();

            //
            // parse arguments, gathering data and overriding defaults if applicable
            //

            for (int i = 0; i < args.Length; ++i)
            {
                if (args[i].Length < 2)
                    continue;
                if (args[i][0] != '-' && args[i][0] != '/' && args[i][0] != '\\')
                    continue;
                switch (args[i][1])
                {
                    case 'i':
                    case 'I':
                        {
                            if (i + 1 < args.Length)
                                inputFolderName = args[i + 1];
                        }
                        break;
                    case 'o':
                    case 'O':
                        {
                            if (i + 1 < args.Length)
                                outputFileName = args[i + 1];
                        }
                        break;
                    case 'r':
                    case 'R':
                        {
                            recursive = true;
                        }
                        break;
                    case 'e':
                    case 'E':
                        {
                            explicitSearch = true;
                        }
                        break;
                    case 'f':
                    case 'F':
                        {
                            if (i + 1 < args.Length)
                                moreFileExts.Add(args[i + 1]);
                        }
                        break;
                    case 'h':
                    case 'H':
                        {
                            if (i + 1 < args.Length)
                                filesToInclude.Add(args[i + 1]);
                        }
                        break;
                }
            }

            if (outputFileName.Length == 0)
            {
                outputFileName = inputFolderName + '\\' +
                    inputFolderName.Substring(inputFolderName.LastIndexOf('\\') + 1);
            }

            //
            // create output file
            //

            System.IO.StreamWriter outputFileStreamHpp, outputFileStreamCpp;
            String hfile = outputFileName + ".rtti.hpp";
            String cfile = outputFileName + ".rtti.cpp";

            fSpawnProcess("openforedit.cmd", hfile, System.Environment.GetEnvironmentVariable("SigEngine") + "\\bin");
            fSpawnProcess("openforedit.cmd", cfile, System.Environment.GetEnvironmentVariable("SigEngine") + "\\bin");

            try
            {
                outputFileStreamHpp = System.IO.File.CreateText( hfile );
                outputFileStreamCpp = System.IO.File.CreateText( cfile );
            }
            catch (System.Exception)
            {
                Console.Write(opts);
                return;
            }

            string headerFileGuardDefine = "__" + outputFileName.Substring(outputFileName.LastIndexOf('\\') + 1) + "_rtti__";
            outputFileStreamHpp.WriteLine("#ifndef " + headerFileGuardDefine);
            outputFileStreamHpp.WriteLine("#define " + headerFileGuardDefine);

            foreach (string s in filesToInclude)
            {
                outputFileStreamCpp.WriteLine("#include \"" + s + "\"");
            }

            Console.WriteLine();
            Console.WriteLine("Preparing reflection information for output file {0}...", outputFileName);


            //
            // gather all the input files into one list
            //

            List<string> inputFiles = new List<string>();
            if (!explicitSearch)
                fAddInputFiles(inputFiles, inputFileExts, inputFolderName, recursive);
            fAddInputFiles(inputFiles, moreFileExts, inputFolderName, recursive);

            if (inputFiles.Count == 0)
            {
                Console.Write(opts);
                return;
            }

            //
            // ... and go ...
            //

            tReflectionGenerator reflectionGenerator = new tReflectionGenerator();

            for (int i = 0; i < inputFiles.Count; ++i)
            {
                Console.WriteLine("Working on input file {0}...", inputFiles[i]);

                // verbose...
                //Console.WriteLine("\tgenerating reflection data");

                tFile reflectedFile = reflectionGenerator.fGenerateFile(inputFolderName, inputFiles[i]);

                if (reflectedFile == null)
                {
                    Console.WriteLine("\tencountered error while generating data, skipping input file");
                    continue;
                }

                if (reflectedFile.fHasReflectedClass())
                {
                    Console.WriteLine("\twriting reflected classes to output file");
                    reflectedFile.fOutput(outputFileStreamHpp, true);
                    reflectedFile.fOutput(outputFileStreamCpp, false);
                }
                else
                {
                    // verbose...
                    //Console.WriteLine("\tno reflected classes to write to output file");
                }

                outputFileStreamHpp.Flush();
                outputFileStreamCpp.Flush();
            }

            outputFileStreamHpp.WriteLine("#endif//" + headerFileGuardDefine);

            outputFileStreamHpp.Close();
            outputFileStreamCpp.Close();
        }

        static void fAddInputFiles(List<string> inputFiles, List<string> inputFileExts, string dirName, bool recursive)
        {
            for (int i = 0; i < inputFileExts.Count; ++i)
            {
                string[] files = System.IO.Directory.GetFiles(
                    dirName, inputFileExts[i], 
                    recursive ? System.IO.SearchOption.AllDirectories : System.IO.SearchOption.TopDirectoryOnly);
                for (int j = 0; j < files.Length; ++j)
                {
                    if (!files[j].Contains(".rtti."))
                        inputFiles.Add(files[j]);
                }
            }
        }

        static void fSpawnProcess(string cmd, string args, string dir)
        {
            System.Diagnostics.Process spawnedProc = new System.Diagnostics.Process();
            spawnedProc.StartInfo.FileName = cmd;
            spawnedProc.StartInfo.Arguments = args;
            spawnedProc.StartInfo.UseShellExecute = false;
            //spawnedProc.StartInfo.RedirectStandardOutput = true;
            //spawnedProc.StartInfo.RedirectStandardInput = true;
            //spawnedProc.StartInfo.RedirectStandardError = true;
            spawnedProc.StartInfo.CreateNoWindow = true;
            spawnedProc.StartInfo.WorkingDirectory = dir;

            //spawnedProc.OutputDataReceived += OutputReceived;
            //spawnedProc.ErrorDataReceived += OutputReceived;

            spawnedProc.Start();

            while (!spawnedProc.HasExited)
            { }
        }
    }
}
