using System;
using System.IO;

namespace AutoTest
{
    public class Logger
    {
        private string filePath;
        private StreamWriter logToFile;

        public Logger( string filePath )
        {
            this.filePath = filePath;
            logToFile = new StreamWriter( filePath );
            logToFile.AutoFlush = true;
        }

        ~Logger( )
        {
            if( File.Exists( filePath ) )
                File.Delete( filePath );
        }

        public void Log( string message )
        {
            if( logToFile != null )
                logToFile.WriteLine( message );

            Console.WriteLine( message );
        }

        public void Close( )
        {
            logToFile.Close( );
        }

        public void Copy( string dest )
        {
            File.Copy( filePath, dest );
        }
    }
}
