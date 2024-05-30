using System;
using System.Diagnostics;
using System.IO;

namespace CleanBuildUI
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

        public static bool FileIsCheckedOut( string filePath )
        {
            string user = GetP4SetVariable( "P4USER" );
            if(user == "")
                throw new Exception("Could not get P4USER.");

            P4Process process = new P4Process( "opened -u " + user + " \"" + filePath + "\"");
            process.Start( );
            process.WaitForExit( );

            return ( process.ReadToEnd( process.StandardOutput ) == "" ? false : true );
        }

        public static bool CheckOutFile( string filePath )
        {
            P4Process process = new P4Process( "edit " + filePath );
            process.Start( );
            process.WaitForExit( );
            if( process.ReadToEnd( process.StandardError ) != "" )
                throw new Exception( "Could not check out " + filePath + " from Perforce." );

            return FileIsCheckedOut( filePath );
        }

        public static bool RevertFile( string filePath )
        {
            P4Process process = new P4Process( "revert " + filePath );
            process.Start( );
            process.WaitForExit( );
            string errorOutput = process.ReadToEnd( process.StandardError );
            if( errorOutput != "" && !errorOutput.Contains( "file(s) not opened" ) )
                throw new Exception( "Could not revert " + filePath + " in Perforce." );

            return !FileIsCheckedOut( filePath );
        }

        public static string GetP4SetVariable( string variableName )
        {
            P4Process process = new P4Process( "set " + variableName );
            process.Start( );
            process.WaitForExit( );
            string value = process.ReadToEnd( process.StandardOutput );

            string[ ] splitData = value.Split( "=".ToCharArray( ), 2 );
            if( splitData.Length >= 2 )
                value = splitData[ 1 ];
            value = value.Replace( "(set)", "" ).Trim( );

            return value;
        }
    }
}
