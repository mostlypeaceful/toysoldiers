using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace P4Delta
{
    class Program
    {
        static List<string> fileLines = new List<string>();

        static void Main(string[] args)
        {
            if (args.Length < 1)
            {
                Console.WriteLine("\nP4Delta [-sacml] [-p=<Path>] -r1=<Revision|Time|Label #1> -r2=[Revision|Time|Label #2]");
                Console.WriteLine("\nExamples:");
                Console.WriteLine("* p4delta -r1=2011/02/09:00:00 -r2=2011/02/09:12:00");
                Console.WriteLine("* p4delta -r1=1705 -r2=1902");
                Console.WriteLine("* p4delta -r1=alpha_label -r2=rc_label");
                Console.WriteLine("* p4delta -p=//SigEngine/trunk/src/... -r1=1705");
                Console.WriteLine("\n\nCopyright 2011 Signal Studios.");

                return;
            }

            bool sacmlOut = false;
            string lhs = "";
            string rhs = "";

            List<string> paths = new List<string>();
            foreach (string arg in args)
            {
                if (arg.Length < 3)
                    continue;

                if (arg.Substring(0, 3) == "-p=")
                    paths.Add(arg.Substring(3));
                else if (arg.Substring(0, 4) == "-r1=")
                    lhs = arg.Substring(4);
                else if (arg.Substring(0, 4) == "-r2=")
                    rhs = arg.Substring(4);
                else if (arg == "-sacml")
                    sacmlOut = true;
            }
            if (paths.Count == 0)
                paths.Add("//...");

            if (rhs == "")
            {
                P4Process process = new P4Process(Environment.ExpandEnvironmentVariables("changes -s submitted -m 1"));
                process.Start();
                string latestChange = "";
                while (!process.StandardOutput.EndOfStream || !process.HasExited)
                {
                    string data = P4Process.GetTagData(process.StandardOutput.ReadLine(), "change");
                    if (data != "")
                    {
                        latestChange = data;
                        break;
                    }
                }
                if (latestChange != "")
                    rhs = latestChange;
            }

            if (lhs == "" || rhs == "")
            {
                fileLines.Add("Missing tag.");

                return;
            }

            // Get a list of files from each label
            List<P4FileMetadata> lhsList = GetFileList(paths, lhs);
            List<P4FileMetadata> rhsList = GetFileList(paths, rhs);

            List<P4FileMetadata> added = new List<P4FileMetadata>();
            List<P4FileMetadata> deleted = new List<P4FileMetadata>();
            List<P4FileMetadata> modified = new List<P4FileMetadata>();
            for (int i = 0; i < rhsList.Count; i++)
            {
                P4FileMetadata rFile = rhsList[i];

                for (int j = 0; j < lhsList.Count; j++)
                {
                    P4FileMetadata lFile = lhsList[j];

                    if (rFile.depotFile == lFile.depotFile)
                    {
                        // If the change number is the same in both revisions, nothing's changed
                        if (rFile.change != lFile.change)
                        {
                            // If the change number is not the same, it's either been edited or
                            // deleted, because anything ADDED wouldn't even show up in the LHS list
                            if (rFile.action == "delete")
                                deleted.Add(rFile);
                            else
                                modified.Add(rFile);
                        }

                        rhsList.RemoveAt(i--);
                        lhsList.RemoveAt(j--);

                        break;
                    }
                }
            }

            added = rhsList;

            if (sacmlOut)
            {
                fileLines.Add("<Sacml>");
                fileLines.Add("\t<DoFolder>0</DoFolder>");
                fileLines.Add("\t<Recurse>0</Recurse>");
                fileLines.Add("\t<Compress>1</Compress>");
                fileLines.Add("\t<IgnoreDevFiles>0</IgnoreDevFiles>");
                fileLines.Add("\t<CompressionThreshold>0</CompressionThreshold>");
                fileLines.Add("\t<FilesToAdd>");
            }

            foreach (P4FileMetadata file in added)
            {
                if (GetFileExtension(file.depotFile) == "animl")
                    ExpandANIMLFile(file.depotFile, file.action, sacmlOut);

                if (IsExcluded(file))
                    continue;

                file.depotFile = MapSourceToBinaryFile(file.depotFile);

                switch (GetFileExtension(file.depotFile))
                {
                    case "mshb":
                        {
                            ExpandSpecialFile("geob", file.depotFile, file.action, sacmlOut);
                        }
                        break;
                    case "sigb":
                        {
                            ExpandSpecialFile("geob", file.depotFile, file.action, sacmlOut);
                            ExpandSpecialFile("texb", file.depotFile, file.action, sacmlOut);
                        }
                        break;
                }

                AddFileLine(file.depotFile, file.action, sacmlOut);
            }

            foreach (P4FileMetadata file in modified)
            {
                if (GetFileExtension(file.depotFile) == "animl")
                    ExpandANIMLFile(file.depotFile, file.action, sacmlOut);

                if (IsExcluded(file))
                    continue;

                file.depotFile = MapSourceToBinaryFile(file.depotFile);

                switch (GetFileExtension(file.depotFile))
                {
                    case "mshb":
                        {
                            ExpandSpecialFile("geob", file.depotFile, file.action, sacmlOut);
                        }
                        break;
                    case "sigb":
                        {
                            ExpandSpecialFile("geob", file.depotFile, file.action, sacmlOut);
                            ExpandSpecialFile("texb", file.depotFile, file.action, sacmlOut);
                        }
                        break;
                }

                AddFileLine(file.depotFile, file.action, sacmlOut);
            }

            if (sacmlOut)
            {
                fileLines.Add("\t</FilesToAdd>");
                fileLines.Add("</Sacml>");
            }

            foreach (string line in fileLines)
                Console.WriteLine(line);
        }

        static List<P4FileMetadata> GetFileList(List<string> paths, string label)
        {
            List<P4FileMetadata> p4Files = new List<P4FileMetadata>();
            P4FileMetadata currentFile = new P4FileMetadata();

            foreach (string path in paths)
            {
                string args = Environment.ExpandEnvironmentVariables("files " + path + "@" + label);
                P4Process process = new P4Process(args);
                process.Start();
                while (!process.HasExited)
                {
                    string stdout = process.StandardOutput.ReadLine();
                    if (stdout == "")
                    {
                        p4Files.Add(currentFile);
                        currentFile = new P4FileMetadata();
                    }

                    string tag = P4Process.GetTag(stdout);
                    if (tag != null && tag != "")
                    {
                        string data = P4Process.GetTagData(stdout, tag);

                        switch (tag)
                        {
                            case "depotFile":
                                currentFile.depotFile = data;
                                break;
                            case "rev":
                                currentFile.rev = data;
                                break;
                            case "change":
                                currentFile.change = data;
                                break;
                            case "action":
                                currentFile.action = data;
                                break;
                            case "type":
                                currentFile.type = data;
                                break;
                            case "time":
                                currentFile.time = data;
                                break;
                        }
                    }
                }

                string errors = process.StandardError.ReadToEnd();
                if (errors != "")
                    fileLines.Add(errors);
            }

            return p4Files;
        }

        static string MapDepotFileToLocalFile(string depotFile)
        {
            string localFile = "";

            P4Process process = new P4Process("where \"" + depotFile + "\"");
            process.Start();
            while (!process.StandardOutput.EndOfStream || !process.HasExited)
            {
                string line = process.StandardOutput.ReadLine();
                string tagData = P4Process.GetTagData(line, "path");
                if (tagData != "")
                {
                    localFile = tagData;
                    break;
                }
            }

            // Return the same thing back if we couldn't map it
            if (localFile == "")
                localFile = depotFile;

            return localFile;
        }

        static string FormatFileLine(string file)
        {
            string resDir = Environment.ExpandEnvironmentVariables("%SigCurrentProject%\\res\\");

            // Map the depot file to the local file
            string filePath = MapDepotFileToLocalFile(file);

            // Replace the path to the res dir with nothing
            filePath = Replace(filePath, resDir, "", false);

            return filePath;
        }

        static string Replace(string sourceString, string searchString, string replaceString, bool caseSensitive)
        {
            if (sourceString == null)
                throw new ArgumentNullException("sourceString");

            if (searchString == null)
                throw new ArgumentNullException("searchString");

            if (String.IsNullOrEmpty(searchString))
                throw new ArgumentException("searchString cannot be an empty string.", "searchString");

            if (replaceString == null)
                throw new ArgumentNullException("replaceString");

            StringBuilder retVal = new StringBuilder(sourceString.Length);

            int ptr = 0;
            int lastPtr = 0;

            while (ptr >= 0)
            {
                ptr = sourceString.IndexOf(searchString, ptr, caseSensitive ? StringComparison.InvariantCulture : StringComparison.OrdinalIgnoreCase);

                int strLength = ptr - lastPtr;
                if (strLength > 0 || ptr == 0)
                {
                    if (ptr > 0)
                        retVal.Append(sourceString.Substring(lastPtr, strLength));

                    retVal.Append(replaceString);
                    ptr += searchString.Length;
                }
                else
                    break;

                lastPtr = ptr;
            }

            if (lastPtr >= 0) // Append the piece of the string left after the last occurrence of searchString, if any
                retVal.Append(sourceString.Substring(lastPtr));

            return retVal.ToString();
        }

        static string MapSourceToBinaryFile(string file)
        {
            string ext = GetFileExtension(file).ToLower();

            if (ext == "goaml")
                return file.Substring(0, file.Length - 5) + "nutb";
            else if (ext == "anipk")
                return file.Substring(0, file.Length - 2) + "b";
            else if (ext == "tga" || ext == "nut" || ext == "png" || ext == "tab")
                return file += "b";
            else if (ext.Substring(ext.Length - 2, 2) == "ml")
                return file.Substring(0, file.Length - 2) + "b";

            // There is no special mapping, return the same name
            return file;
        }

        static bool IsUnderDevFolder(P4FileMetadata file)
        {
            if (file.depotFile.Contains(@"/~") || file.depotFile.Contains(@"/."))
                return true;

            return false;
        }

        static bool IsUnderUserFolder(P4FileMetadata file)
        {
            if (file.depotFile.Contains(@"/_"))
                return true;

            return false;
        }

        static bool IsExcluded(P4FileMetadata file)
        {
            if (IsUnderDevFolder(file) || IsUnderUserFolder(file) ||
                file.depotFile.ToLower().Substring(file.depotFile.Length - 5, 5) == "animl")
                return true;

            return false;
        }

        static string GetFileExtension(string file)
        {
            int index = file.LastIndexOf(".");
            if (index < 0)
                return "";

            return file.Substring(index + 1);
        }

        static string GetFileName(string file)
        {
            int index = file.LastIndexOf("/");
            if (index < 0)
            {
                index = file.LastIndexOf("\\");
                if (index < 0)
                    return "";
            }

            int extIndex = file.LastIndexOf(".");
            if (extIndex >= 0)
                return file.Substring(index + 1, extIndex - index - 1);

            // No extension?
            return file.Substring(index + 1);
        }

        static string GetFilePath(string file)
        {
            int index = file.LastIndexOf("/");
            if (index < 0)
            {
                index = file.LastIndexOf("\\");
                if (index < 0)
                    return "";
            }

            return file.Substring(0, index);
        }

        static void AddFileLine(string file, string action, bool sacmlOut)
        {
            // Standardize slashes
            file = file.Replace("\\", "/");

            string line;
            if (sacmlOut)
                line = "\t\t<i>" + FormatFileLine(file) + "</i>";
            else
                line = action + "\t" + file;

            // No duplication of lines
            foreach (string fileLine in fileLines)
            {
                if (line == fileLine)
                    return;
            }

            fileLines.Add(line);
        }

        static void ExpandSpecialFile(string expandedExtension, string file, string action, bool sacmlOut)
        {
            // Take the depot file and map it to where the binary version of the file should be located
            string localFile = MapDepotFileToLocalFile(file);
            string assetDir = GetFilePath(localFile).Replace(@"\res\", @"\game\xbox360\");

            // Scan the directory for special files created by the original files
            string[] expandedFiles = Directory.GetFiles(assetDir, GetFileName(file) + ".*");
            foreach (string expandedFile in expandedFiles)
            {
                if (GetFileExtension(expandedFile) == expandedExtension)
                    AddFileLine(expandedFile.Replace(@"\game\xbox360\", @"\res\"), action, sacmlOut);
            }
        }

        static void ExpandANIMLFile(string file, string action, bool sacmlOut)
        {
            string localFile = MapDepotFileToLocalFile(file);
            string localDir = GetFilePath(localFile);

            // Scan the directory for special files created by the original files
            string[] expandedFiles = Directory.GetFiles(localDir, "*.anipk");
            foreach (string expandedFile in expandedFiles)
                ExpandSpecialFile("anib", expandedFile, action, sacmlOut);
        }
    }
}
