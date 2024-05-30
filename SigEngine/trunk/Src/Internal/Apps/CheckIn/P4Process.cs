using System.Diagnostics;
using System.IO;

namespace CheckIn
{
    class P4Process : Process
    {
        static int logLevel = 0;
        static string logFilePath = "log.txt";

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

        public new bool Start()
        {
            if (logLevel > 0)
            {
                TextWriter logFile = new StreamWriter(logFilePath, true);
                logFile.WriteLine(base.StartInfo.FileName + " " + base.StartInfo.Arguments);
                logFile.Close();
            }

            return base.Start();
        }

        public string ReadToEnd(StreamReader reader)
        {
            string output = "";
            while (!reader.EndOfStream)
                output += reader.ReadLine() + "\r\n";

            if (logLevel > 1)
            {
                TextWriter logFile = new StreamWriter(logFilePath, true);
                logFile.WriteLine(output);
                logFile.Close();
            }

            return output;
        }

        public string ReadLine(StreamReader reader)
        {
            string output = reader.ReadLine();

            if (logLevel > 1)
            {
                TextWriter logFile = new StreamWriter(logFilePath, true);
                logFile.WriteLine(output);
                logFile.Close();
            }

            return output;
        }
    }
}
