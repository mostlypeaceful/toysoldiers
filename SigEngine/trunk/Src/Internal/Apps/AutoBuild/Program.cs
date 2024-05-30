using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

namespace AutoBuild
{
    class Program
    {
        static string cleanBuildFile = Environment.ExpandEnvironmentVariables( @"%SigCurrentProject%\cleanbuild" );
        static string cleanGameFile = Environment.ExpandEnvironmentVariables( @"%SigCurrentProject%\cleangame" );
        static string cleanGameDirectory = Environment.ExpandEnvironmentVariables( @"%SigCurrentProject%\Game" );

        static bool FindArg( string argName, string[ ] args )
        {
            foreach( string i in args )
            {
                if( i == argName )
                    return true;
            }
            return false;
        }

        static int Main( string[ ] args )
        {
            try
            {
                bool p4 = FindArg( "p4", args );
                bool sync = FindArg( "sync", args );
                bool build2010 = FindArg( "2010", args );

                if( sync )
                {
                    if( p4 )
                    {
                        // Get the latest p4 updates
                        ExecuteCommand( @"%SigEngine%\Bin", "AutoBuildHelper_P4Update.cmd" );
                    }
                    else
                    {
                        // Get the latest SVN updates
                        ExecuteCommand( @"%SigEngine%\Bin", "AutoBuildHelper_SvnUpdate.cmd" );
                    }
                }

                bool cleanBuildNeeded = false;
                List<string> cleanbuildLines = new List<string>( );
                using( TextReader reader = new StreamReader( cleanBuildFile ) )
                {
                    string line;
                    while( ( line = reader.ReadLine( ) ) != null )
                    {
                        if( line != String.Empty && line != "" )
                            cleanbuildLines.Add( line );
                    }
                }
                if( cleanbuildLines.Count > 0 && cleanbuildLines[ 0 ] != "0" )
                    cleanBuildNeeded = true;

                int cleanGameCounter = -1;
                using( TextReader reader = new StreamReader( cleanGameFile ) )
                {
                    try
                    {
                        cleanGameCounter = Convert.ToInt32( reader.ReadLine( ) ) + 1;
                    }
                    catch
                    {
                        cleanGameCounter = 0;
                    }
                }

                if( cleanBuildNeeded )
                {
                    if( cleanbuildLines[ 0 ] == "1" )
                        ExecuteCommand( @"%SigEngine%\Bin", "AutoBuildHelper_Clean.cmd" );
                    else
                    {
                        // Assume the file is full of extensions; get rid of any preceding dot
                        for( int i = 0; i < cleanbuildLines.Count; ++i )
                        {
                            cleanbuildLines[ i ] = cleanbuildLines[ i ].ToLower( );

                            if( cleanbuildLines[ i ].Substring( 0, 1 ) != "." )
                                cleanbuildLines[ i ] = "." + cleanbuildLines[ i ];
                        }

                        List<string> filesToDelete = new List<string>( );
                        ProcessDirectory( cleanGameDirectory, ref filesToDelete, ref cleanbuildLines );
                        foreach( string file in filesToDelete )
                        {
                            Console.WriteLine( "Deleting " + file + "..." );
                            File.Delete( file );
                        }
                    }

                    UpdateCleanFile( p4, cleanBuildFile, Convert.ToString( 0 ), "Project cleaned by AutoBuild." );
                }

                string command = build2010 ? "AutoBuildHelper_Build_VS2010.cmd" : "AutoBuildHelper_Build.cmd";
                ExecuteCommand( @"%SigEngine%\Bin", command );

                if( cleanBuildNeeded )
                    UpdateCleanFile( p4, cleanGameFile, Convert.ToString( cleanGameCounter ), "Counter incremented by AutoBuild." );
            }
            catch( Exception ex )
            {
                Console.WriteLine( ex.Message );
                Console.WriteLine( "Encountered an error during project build process." );

                return 1;
            }

            return 0;
        }

        static void UpdateCleanFile( bool p4, string cleanFile, string updatedText, string submitText )
        {
            if( p4 )
                ExecuteCommand( "%SigCurrentProject%", @"p4 edit " + cleanFile );

            FileStream fileStream = new FileStream( cleanFile, FileMode.Truncate );
            fileStream.Close( );
            using( TextWriter writer = new StreamWriter( cleanFile ) )
                writer.WriteLine( updatedText );

            if( p4 )
                ExecuteCommand( "%SigCurrentProject%", "p4 submit -d \"" + submitText + "\" " + cleanFile );
            else
                ExecuteCommand( "%SigCurrentProject%", @"svn commit -m " + submitText + " " + cleanFile );
        }

        static void ExecuteCommand( string workingDir, params string[ ] commands )
        {
            Process process = new Process( );
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.FileName = "cmd.exe";
            process.StartInfo.RedirectStandardInput = true;
            process.StartInfo.WorkingDirectory = Environment.ExpandEnvironmentVariables( workingDir );

            process.Start( );

            // What we're doing is basically treating each command as if it were
            // typed into a CMD instance so that we can have things persist
            // across multiple commands when necessary
            foreach( string command in commands )
                process.StandardInput.WriteLine( Environment.ExpandEnvironmentVariables( command ) );

            process.StandardInput.Close( );
            process.WaitForExit( );

            if( process.ExitCode != 0 )
            {
                string message = "!WARNING! Error executing command(s):\n";

                foreach( string command in commands )
                    message += command + "\n";

                throw ( new Exception( message ) );
            }
        }

        static void ProcessDirectory( string targetDirectory, ref List<string> fileList, ref List<string> extList )
        {
            // Process the list of files found in the directory
            string[ ] fileEntries = Directory.GetFiles( targetDirectory );
            foreach( string fileName in fileEntries )
            {
                string ext = Path.GetExtension( fileName ).ToLower( );
                if( extList.Contains( ext ) )
                    fileList.Add( fileName );
            }

            // Recurse into subdirectories of this directory.
            string[ ] subdirectoryEntries = Directory.GetDirectories( targetDirectory );
            foreach( string subdirectory in subdirectoryEntries )
                ProcessDirectory( subdirectory, ref fileList, ref extList );
        }
    }
}
