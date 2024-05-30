using System;
using System.Diagnostics;
using System.IO;
using System.Threading;

namespace AutoTest
{
    class Program
    {
        static MapList mapList = new MapList( );

        static DebugMonitor monitor;

        static void Main( string[ ] args )
        {
            int exitCode = 0;

            try
            {
                RunMaps( mapList );

                if( monitor == null )
                    Console.WriteLine( "!WARNING! No maps were found! " );
                else if( monitor.CrashEncountered || monitor.MapNotFound )
                {
                    if( monitor.CrashEncountered )
                    {
                        Console.WriteLine( "!WARNING! Game crashed in map " + monitor.LastMap + "!" );
                        Console.WriteLine( "!WARNING! Last output from game was:\n" + monitor.LastGameMessage );
                    }

                    exitCode = 1;
                }
            }
            catch( System.Exception ex )
            {
                string message = ex.Message;
                if( ex.InnerException != null )
                {
                    message += "\n\nInner Exceptions:\n\n";

                    Exception innerE = ex.InnerException;
                    while( innerE != null )
                    {
                        message += "\n\n------------------------------";
                        message += innerE.Message;

                        innerE = innerE.InnerException;
                    }
                }
                if( ex.StackTrace != null )
                    message += "\n\nStack Trace:\n" + ex.StackTrace + "\n";

                Console.WriteLine( message );

                exitCode = 1;
            }

            Environment.Exit( exitCode );
        }

        static void RunMaps( MapList mapList )
        {
            DebugMonitor_Xbox360 monitor_xbox360 = null;
            DebugMonitor_PCDX9 monitor_pcdx9 = null;

            for( int i = 0; i < mapList.MapCount; ++i )
            {
                Map map = mapList[ i ];

                string launchArgs = "-min -nowatson -noprogress";

                // We use a switch so that, in theory, we can test both
                // Xbox 360 and PC versions of levels together
                switch( map.Platform )
                {
                    case Platforms.xbox360:
                        if( monitor_xbox360 == null )
                            monitor_xbox360 = new DebugMonitor_Xbox360( );

                        if( monitor_xbox360.ConnectionState == ConnectionState.NotConnected )
                        {
                            monitor_xbox360.Connect( DebugMonitor_Xbox360.XboxManager.DefaultConsole );
                            monitor_xbox360.LogInDefaultUser( );
                        }

                        monitor = monitor_xbox360;

                        launchArgs += " -platform xbox360";
                        break;
                    case Platforms.pcdx9:
                        if( monitor_pcdx9 == null )
                        {
                            monitor_pcdx9 = new DebugMonitor_PCDX9( );
                            DebugMonitor_PCDX9.Start( );
                        }

                        monitor = monitor_pcdx9;

                        launchArgs += " -platform pcdx9";
                        break;
                }

                // Reset the flag to load the next map, otherwise we'll happily load
                // all the maps before receiving the success message
                monitor.LoadNextMap = false;

                // If the map path says "No Preview," we just need to launch the front end
                if( map.Path != "No Preview" )
                {
                    if( !File.Exists( DebugMonitor.ProjectPath + "\\res" + map.Path ) )
                    {
                        monitor.PrintMessage( "!WARNING! File not found: " + map.Path + "!" );
                        monitor.MapNotFound = true;
                        continue;
                    }

                    launchArgs += " -options \"-preview " + map.Path + "\"";
                }

                monitor.LastMap = map.Path;

                monitor.PrintMessage( "Launching " + monitor.LastMap + "..." );

                // Execute the command
                Process process = new Process( );
                process.StartInfo.UseShellExecute = false;
                process.StartInfo.RedirectStandardOutput = true;
                process.StartInfo.FileName = "GameLauncher";
                process.StartInfo.Arguments = launchArgs;
                process.Start( );
                while( !process.StandardOutput.EndOfStream || !process.HasExited )
                    monitor.PrintMessage( process.StandardOutput.ReadLine( ) );

                if( process.ExitCode != 0 )
                    throw new Exception( "GameLauncher could not be run!" );

                // Wait until the console tells us it's OK to proceed
                int sleepSeconds = 180;
                while( !monitor.LoadNextMap && !monitor.CrashEncountered )
                {
                    Thread.Sleep( 1000 );

                    sleepSeconds--;
                    if( sleepSeconds <= 0 )
                    {
                        monitor.PrintMessage( "!WARNING! The message to continue to the next level was never received while testing " + monitor.LastMap + "!" );
                        monitor.CrashEncountered = true;
                    }
                }

                Process[ ] processes = Process.GetProcessesByName( "Game" );
                foreach( Process gameProcess in processes )
                {
                    monitor.PrintMessage( "Killing Game.exe process..." );
                    try
                    {
                        gameProcess.Kill( );
                    }
                    catch
                    {
                    }
                }

                if( monitor.CrashEncountered )
                    break;
            }

            if( monitor_xbox360 != null )
            {
                monitor_xbox360.RebootConsole( true );
                monitor_xbox360.Disconnect( );
            }

            if( monitor_pcdx9 != null )
                DebugMonitor_PCDX9.Stop( );
        }
    }
}
