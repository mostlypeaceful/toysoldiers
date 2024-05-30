using System.Diagnostics;
using System.Text.RegularExpressions;

namespace P4Delta
{
    class P4Process : Process
    {
        public P4Process(string arguments)
        {
            StartInfo = base.StartInfo;
            StartInfo.UseShellExecute = false;
            StartInfo.RedirectStandardOutput = true;
            StartInfo.RedirectStandardError = true;
            StartInfo.RedirectStandardInput = true;
            StartInfo.CreateNoWindow = true;
            StartInfo.FileName = "p4";
            StartInfo.Arguments = "-z tag " + arguments;
        }

        public static string GetTagData(string line, string tagName)
        {
            if (line == null || line == "")
                return "";

            Regex pattern = new Regex(@"^\.\.\. " + tagName + " (.+)\r*$", RegexOptions.Multiline);

            Match match = pattern.Match(line);
            if (match.Groups.Count < 2)
                return "";

            return match.Groups[1].ToString().Trim();
        }

        public static string GetTag(string line)
        {
            if (line == null || line == "")
                return "";

            string[] components = line.Split(" ".ToCharArray());
            if (components.Length < 3 || components[0] != "...")
                return "";

            return components[1];
        }

        public static string GetP4SetVariable(string variableName)
        {
            P4Process process = new P4Process("set " + variableName);
            process.Start();
            process.WaitForExit();
            string value = process.StandardOutput.ReadToEnd();

            string[] splitData = value.Split("=".ToCharArray(), 2);
            if (splitData.Length >= 2)
                value = splitData[1];
            value = value.Replace("(set)", "").Trim();

            return value;
        }
    }
}
