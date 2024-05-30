using System.IO;

namespace PostBuildProcess
{
    class LogIt
    {
        private static bool enabled = true;
        private string path = "";

        public LogIt(string path)
        {
            if (!enabled)
                return;

            if (File.Exists(path))
                File.Delete(path);

            this.path = path;
        }

        public void LogLine(string line)
        {
            if (!enabled)
                return;

            using (TextWriter textFile = new StreamWriter(this.path, true))
            {
                textFile.WriteLine(line + "\n");
            }
        }
    }
}
