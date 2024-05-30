using System;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;

namespace SquirrelClassDupCheck
{
    class Program
    {
        static void Main(string[] args)
        {
            string startPath;
            if (args.Length > 0)
                startPath = args[0];
            else
            {
                startPath = Environment.GetEnvironmentVariable("SigCurrentProject");
                if (startPath == "")
                {
                    Console.WriteLine("No path was given and no current project is defined.");
                    return;
                }
                startPath += "\\res";
            }

            List<ClassData> classData = new List<ClassData>();

            List<string> files = GetFilesRecursive(startPath, "*.nut");
            foreach (string file in files)
            {
                List<string> classNames = GetClassNamesFromFile(file);
                foreach (string className in classNames)
                {
                    bool found = false;
                    foreach (ClassData data in classData)
                    {
                        if (className == data.Name)
                        {
                            data.files.Add(file);

                            found = true;

                            break;
                        }
                    }
                    if (!found)
                    {
                        ClassData newClass = new ClassData();
                        newClass.Name = className;
                        newClass.files.Add(file);

                        classData.Add(newClass);
                    }
                }
            }

            foreach (ClassData data in classData)
            {
                if (data.files.Count > 1)
                {
                    string output = "!WARNING! >> Duplicate class: " + data.Name;
                    foreach (string path in data.files)
                        output += "\n\t" + path;

                    Console.WriteLine("\n" + output);
                }
            }
        }

        public static List<string> GetFilesRecursive(string startDir, string filePattern)
        {
            List<string> result = new List<string>();
            
            Stack<string> stack = new Stack<string>();
            stack.Push(startDir);
            while (stack.Count > 0)
            {
                string dir = stack.Pop();

                // Ignore directories starting with a tilde
                string[] pathComponents = dir.Split("\\".ToCharArray());
                if (pathComponents[pathComponents.Length - 1].Substring(0, 1) == "~")
                    continue;

                try
                {
                    result.AddRange(Directory.GetFiles(dir, filePattern));
                    foreach (string dn in Directory.GetDirectories(dir))
                        stack.Push(dn);
                }
                catch
                {
                    // Could not open the directory
                }
            }

            return result;
        }

        public static List<string> GetClassNamesFromFile(string path)
        {
            Regex classRegex = new Regex(@"class\s+(\S+)\s*", RegexOptions.Singleline);
            List<string> classNames = new List<string>();

            using (TextReader reader = new StreamReader(path))
            {
                string line = null;
                while ((line = reader.ReadLine()) != null)
                {
                    if (Regex.IsMatch(line, @"^\s*//"))
                        continue;

                    MatchCollection matches = classRegex.Matches(line);
                    foreach (Match match in matches)
                    {
                        if (match.Groups.Count >= 2)
                            classNames.Add(match.Groups[1].Value);
                    }
                }
            }

            return classNames;
        }
    }
}
