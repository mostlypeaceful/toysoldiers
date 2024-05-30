using System;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;

namespace LocmlClean
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length < 2)
                Environment.Exit(1);

            string inputFile = Environment.ExpandEnvironmentVariables(args[0]);
            string outputFile = Environment.ExpandEnvironmentVariables(args[1]);

            try
            {
                Regex regex = new Regex(@"<Text>(.*)</Text>", RegexOptions.Singleline);
                string line = "";
                using (StreamReader file = new StreamReader(inputFile))
                {
                    using (StreamWriter writer = new StreamWriter(File.Open(outputFile, FileMode.Create), Encoding.UTF8))
                    {
                        writer.Write("<StringTable>Text");
                        line = file.ReadToEnd();
                        Match match = regex.Match(line);
                        for (int i = 0; i < match.Groups.Count; ++i)
                        {
                            if (match.Groups[i].Value != "")
                                writer.Write(match.Groups[i].Value);
                        }
                        writer.Write("</StringTable>");
                    }
                }
            }
            catch (System.Exception ex)
            {
                Console.WriteLine(ex.Message);
                Environment.Exit(1);
            }
        }
    }
}
