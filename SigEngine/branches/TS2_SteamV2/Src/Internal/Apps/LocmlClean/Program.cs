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
                Regex regex = new Regex(@"<Text>(.*)</Text>");
                string line = "";
                using (StreamReader file = new StreamReader(inputFile))
                {
                    using (StreamWriter writer = new StreamWriter(File.Open(outputFile, FileMode.Create), Encoding.UTF8))
                    {
                        writer.Write("<StringTable>Text");
                        while ((line = file.ReadLine()) != null)
                        {
                            line = line.Replace("&#xA;", ""); //newline, no character for this, handled internally by TextBox
                            line = line.Replace("&amp;", "&");
                            line = line.Replace("&#38;", "&");
                            line = line.Replace("&#91;", "[");
                            line = line.Replace("&#93;", "]");
                            line = line.Replace("&#32;", " ");
                            line = line.Replace("&gt;", ">");
                            line = line.Replace("&lt;", "<");
                            
                            Match match = regex.Match(line);
                            if (match.Groups.Count > 1 && match.Groups[1].Value != "")
                                writer.Write(match.Groups[1].Value);
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
