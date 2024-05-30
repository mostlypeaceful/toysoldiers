using System;
using System.IO;

namespace AutoTest
{
    public delegate void PrintMessageCallback( string message );
    public delegate void InputQueueCompletedCallback( bool success );
    
    /// <summary>
    /// Encapsulation of the XDevkit/XBDM functionality to monitor an Xbox 360 console.
    /// </summary>
    public class DebugMonitor
    {
        protected InputManager inputManager = null;

        protected static PlatformDebuggingClient debugClient;
        
        protected Logger log = new Logger( Path.GetTempFileName( ) );

        private bool loadNextMap = false;
        public bool LoadNextMap
        {
            get { return loadNextMap; }
            set { loadNextMap = value; }
        }

        private bool crashEncountered = false;
        public bool CrashEncountered
        {
            get { return crashEncountered; }
            set { crashEncountered = value;  }
        }

        private bool mapNotFound = false;
        public bool MapNotFound
        {
            get { return mapNotFound; }
            set { mapNotFound = value; }
        }

        private string lastGameMessage = String.Empty;
        public string LastGameMessage
        {
            get { return lastGameMessage; }
        }

        private string lastMap = String.Empty;
        public string LastMap
        {
            get { return lastMap; }
            set { lastMap = value; }
        }

        static public string ProjectName
        {
            get
            {
                string name = Environment.GetEnvironmentVariable( "SigCurrentProjectName" );
                if( String.IsNullOrWhiteSpace( name ) )
                    throw new ArgumentNullException( "ProjectName", "SigCurrentProjectName is null or empty!" );

                return name;
            }
        }

        static public string ProjectPath
        {
            get
            {
                string path = Environment.GetEnvironmentVariable( "SigCurrentProject" );
                if( String.IsNullOrWhiteSpace( path ) )
                    throw new ArgumentNullException( "ProjectPath", "SigCurrentProject is null or empty!" );

                return path;
            }
        }

        static public string EnginePath
        {
            get
            {
                string path = Environment.GetEnvironmentVariable( "SigEngine" );
                if( String.IsNullOrWhiteSpace( path ) )
                    throw new ArgumentNullException( "EnginePath", "SigEngine is null or empty!" );

                return path;
            }
        }

        protected void InitializeManagers( string hostIP )
        {
            debugClient = new PlatformDebuggingClient( hostIP );
            inputManager = new InputManager( ref debugClient, new PrintMessageCallback( this.PrintMessage ) );
        }

        public void PrintMessage( string message )
        {
            log.Log( message );
        }

        private void QueuedInputComplete( bool success )
        {
            if( !success )
            {
                crashEncountered = true;
                return;
            }

            lastMap = "";
            loadNextMap = true;
        }

        protected void ParseDebugString( string message )
        {
            PrintMessage( message );
            if( message.Length <= 0 )
                return;

            bool error = false;
            if( message[ 0 ] == '@' )
            {
                if( message.Contains( "@ERROR@" ) )
                    error = true;

                if( error && message.Contains( "@END@" ) )
                    crashEncountered = true;

                // Allow the input system to look at the response and tell
                // us whether we should continue to process it or not
                bool continueProcessing = inputManager.CheckResponse( message );

                if( continueProcessing && message.Contains( "@LEVEL_START_SUCCESS@" ) )
                    RunMapScript( );
            }

            lastGameMessage = message;

            // Make sure CCNET will pull out and highlight the last message in
            // the event of an error by ensuring it begins with "!WARNING!"
            if( error && !lastGameMessage.Contains( message ) && !lastGameMessage.Contains( "!WARNING!" ) )
                lastGameMessage = "!WARNING! " + lastGameMessage;
        }

        private void RunMapScript( )
        {
            string mapName = Path.GetFileNameWithoutExtension( LastMap );
            string mapScriptPath = Environment.ExpandEnvironmentVariables( "%SigCurrentProject%\\Test\\Bin\\" + mapName + ".txt" );

            // If there is an input script, run it
            if( File.Exists( mapScriptPath ) )
            {
                if( inputManager.QueueButtonSequence( mapScriptPath, QueuedInputComplete ) )
                    crashEncountered = true;
            }
            else
                loadNextMap = true;
        }
    }
}
