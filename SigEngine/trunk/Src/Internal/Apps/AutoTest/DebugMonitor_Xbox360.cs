using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using Microsoft.Test.Xbox.Profiles;
using XDevkit;

namespace AutoTest
{
    /// <summary>
    /// Encapsulation of the XDevkit/XBDM functionality to monitor an Xbox 360 console.
    /// </summary>
    public class DebugMonitor_Xbox360 : DebugMonitor
    {
        private static XboxManager xboxManager = new XboxManager( );
        private XboxConsole xboxConsole = null;
        private IXboxDebugTarget debugTarget = null;
        private ConnectionState connectionState = ConnectionState.NotConnected;
        private string runningProcess = string.Empty;
        private XboxEvents_OnStdNotifyEventHandler onStdNotify = null;

        public DebugMonitor_Xbox360( )
        {
            onStdNotify = new XboxEvents_OnStdNotifyEventHandler( xboxConsole_OnStdNotify );
        }

        /// <summary>
        /// XDevkit/XBDM object for managing Xbox 360 consoles.
        /// </summary>
        public static XboxManager XboxManager { get { return xboxManager; } }

        /// <summary>
        /// The Xbox 360 console associated with this object.
        /// </summary>
        public XboxConsole XboxConsole { get { return xboxConsole; } }

        /// <summary>
        /// The name of the Xbox 360 console.
        /// </summary>
        public string Name
        {
            get
            {
                try
                {
                    return xboxConsole.Name;
                }
                catch( ArgumentNullException )
                {
                    return string.Empty;
                }
            }
        }

        /// <summary>
        /// The IP address of the Xbox 360 console.  NOTE: This is the debug IP, not the title IP.
        /// </summary>
        public IPAddress IPAddress
        {
            get
            {
                try
                {
                    long ip = ( long ) EndianSwap( xboxConsole.IPAddress );
                    return new IPAddress( ip );
                }
                catch( NullReferenceException )
                {
                    return IPAddress.None;
                }
            }
        }

        /// <summary>
        /// The IP address of the Xbox 360 title.
        /// </summary>
        public IPAddress IPAddressTitle
        {
            get
            {
                try
                {
                    long ip = ( long ) EndianSwap( xboxConsole.IPAddressTitle );
                    return new IPAddress( ip );
                }
                catch( NullReferenceException )
                {
                    return IPAddress.None;
                }
            }
        }

        /// <summary>
        /// The current state of the connection with the Xbox 360 console.
        /// </summary>
        public ConnectionState ConnectionState { get { return connectionState; } }

        /// <summary>
        /// Connects to the specified Xbox 360 console.
        /// </summary>
        public void Connect( string consoleNameOrIP )
        {
            if( string.IsNullOrEmpty( consoleNameOrIP ) )
                return;

            lock( this )
            {
                // Only connect if there is no existing connection.
                if( connectionState != ConnectionState.NotConnected )
                    return;

                try
                {
                    connectionState = ConnectionState.IsConnecting;

                    PrintMessage( string.Format( "Connecting to {0}...", consoleNameOrIP ) );

                    // Open the specified Xbox 360 console
                    xboxConsole = XboxManager.OpenConsole( consoleNameOrIP );

                    // Receive notifications
                    xboxConsole.OnStdNotify += onStdNotify;

                    // Attach as debugger
                    debugTarget = xboxConsole.DebugTarget;
                    debugTarget.ConnectAsDebugger( "DebugMonitor", XboxDebugConnectFlags.Force );

                    InitializeManagers( IPAddressTitle.ToString( ) );
                }
                catch( COMException e )
                {
                    LogCOMException( e );

                    connectionState = ConnectionState.NotConnected;
                    Disconnect( );

                    throw e;
                }
                catch( NullReferenceException e )
                {
                    connectionState = ConnectionState.NotConnected;
                    Disconnect( );

                    throw e;
                }
            }
        }

        /// <summary>
        /// Close the connection to the Xbox 360 console.
        /// </summary>
        public void Disconnect( )
        {
            lock( this )
            {
                try
                {
                    if( debugTarget != null )
                    {
                        // Detach from the Xbox 360 console.
                        debugTarget.DisconnectAsDebugger( );
                        debugTarget = null;
                    }

                    if( xboxConsole != null )
                    {
                        // Stop receiving notifications.
                        xboxConsole.OnStdNotify -= onStdNotify;
                        xboxConsole = null;
                    }
                }
                catch( COMException e )
                {
                    LogCOMException( e );
                }
                finally
                {
                    connectionState = ConnectionState.NotConnected;
                }
            }
        }

        /// <summary>
        /// Reboots the Xbox 360 console.
        /// </summary>
        public void RebootConsole( bool cold )
        {
            try
            {
                xboxConsole.Reboot( null, null, null, cold ? XboxRebootFlags.Cold : 0 );
            }
            catch( COMException e )
            {
                LogCOMException( e );
            }
            catch( NullReferenceException ) { }
        }

        public void SaveNetworkMinidump( )
        {
            string consoleName = xboxConsole.Name;
            string consolePath = @"DEVKIT:\" + ProjectName;

            DateTime now = DateTime.Now;
            string date = now.ToString( "MM-dd-yy" );
            string time = now.ToString( "H.mm" );

            const string dumpDir = @"\\signal\Shared\QA";
            string projDir = dumpDir + "\\" + ProjectName + "\\Crashdumps";
            string userDir = projDir + "\\" + consoleName;
            string dateDir = userDir + "\\" + date;
            string timeDir = dateDir + "\\" + time;

            // Create a structure of <project>\<console_name>\<date>\<time>
            if( !Directory.Exists( dumpDir ) )
                throw ( new DirectoryNotFoundException( ) );
            if( !Directory.Exists( dateDir ) )
                Directory.CreateDirectory( dateDir );
            if( !Directory.Exists( timeDir ) )
                Directory.CreateDirectory( timeDir );

            string sharePath = timeDir;

            PrintMessage( "Copying DMP file from console..." );
            SaveMinidump( sharePath + @"\dump.dmp", false );

            PrintMessage( "Copying debug output to file..." );
            log.Copy( sharePath + @"\output.txt" );

            PrintMessage( "Saving screen capture..." );
            xboxConsole.ScreenShot( sharePath + @"\capture.bmp" );
        }

        /// <summary>
        /// Generates a minidump (optionally with heap), saving to the specified file.
        /// </summary>
        public void SaveMinidump( string filename, bool withHeap )
        {
            PrintMessage( string.Format( "Saving minidump{0} to {1}...", withHeap ? " w/ heap" : string.Empty, filename ) );

            // NOTE: IXboxDebugTarget::WriteDump cannot always overwrite a file.
            // If the file exists, delete it first.
            if( File.Exists( filename ) )
                File.Delete( filename );

            // Generate the minidump.
            XboxDumpFlags flags = withHeap ? XboxDumpFlags.WithFullMemory : XboxDumpFlags.Normal;
            debugTarget.WriteDump( filename, flags );
        }

        /// <summary>
        /// Adds the current time to the specified text.
        /// </summary>
        private static string AddTimestamp( string text )
        {
            StringBuilder result = new StringBuilder( );

            string timestamp = string.Format( "{0} : ", DateTime.Now );
            string blanks = string.Empty.PadLeft( timestamp.Length, ' ' );
            bool first = true;

            string[ ] lines = text.Split( "\n".ToCharArray( ) );
            foreach( string line in lines )
            {
                string trimmed = line.TrimEnd( " \t\n\r".ToCharArray( ) );
                if( first )
                {
                    result.AppendFormat( "{0}{1}", timestamp, trimmed );
                    first = false;
                }
                else
                {
                    result.AppendFormat( "\n{0}{1}", blanks, trimmed );
                }
            }

            return result.ToString( );
        }

        /// <summary>
        /// Continues execution of the Xbox 360 console.
        /// </summary>
        public void ContinueExecution( IXboxEventInfo eventInfo )
        {
            try
            {
                bool isStopped = ( eventInfo.Info.IsThreadStopped != 0 );

                if( isStopped )
                {
                    eventInfo.Info.Thread.Continue( true );

                    bool notStopped;
                    debugTarget.Go( out notStopped );
                }
            }
            catch( COMException e )
            {
                LogCOMException( e );
            }
        }

        /// <summary>
        /// Swaps the byte-order of the specified value.
        /// </summary>
        private static uint EndianSwap( uint value )
        {
            return ( uint ) ( ( ( value & 0x000000ff ) << 24 )
                | ( ( value & 0x0000ff00 ) << 8 )
                | ( ( value & 0x00ff0000 ) >> 8 )
                | ( ( value & 0xff000000 ) >> 24 ) );
        }

        /// <summary>
        /// Write the COM exception to the log window.
        /// </summary>
        private void LogCOMException( COMException e )
        {
            // Generate the log message.
            StringBuilder message = new StringBuilder( );
            message.Append( "Error: " );

            string translated = xboxManager.TranslateError( e.ErrorCode );
            if( !string.IsNullOrEmpty( translated ) )
                message.Append( translated );
            else
                message.Append( "Unknown exception" );

            message.AppendFormat( " (HRESULT = 0x{0:X8})", e.ErrorCode );

            PrintMessage( message.ToString( ) );
        }

        private const int MaximumAttempts = 5;
        private const int AttemptDelay = 500;

        /// <summary>
        /// Worker thread to obtain running process information from the Xbox 360 console.
        /// </summary>
        private void RunningProcessThread( )
        {
            string name = string.Empty;

            int attempts = 0;
            for( ; ; )
            {
                try
                {
                    name = xboxConsole.RunningProcessInfo.ProgramName;
                    if( !string.IsNullOrEmpty( name ) )
                        break;
                }
                catch( COMException ) { }
                catch( NullReferenceException ) { break; }

                if( ++attempts >= MaximumAttempts )
                    break;

                // Wait before trying again.
                Thread.Sleep( AttemptDelay );
            }

            runningProcess = name;
        }

        /// <summary>
        /// Sets the return value on the Xbox 360 console.
        /// </summary>
        public void SetReturnValue( IXboxEventInfo eventInfo, long value )
        {
            IXboxStackFrame context = eventInfo.Info.Thread.TopOfStack;
            try
            {
                context.SetRegister64( XboxRegisters64.r3, value );
                context.FlushRegisterChanges( );
            }
            catch( COMException e )
            {
                LogCOMException( e );
            }
            finally
            {
                context = null;
            }
        }

        /// <summary>
        /// Handle the AssertionFailed (DM_ASSERT) notification.
        /// </summary>
        private void OnAssertionFailed( IXboxEventInfo eventInfo )
        {
            // Overview.
            StringBuilder overview = new StringBuilder( );
            overview.AppendFormat( "An assertion failed on thread 0x{0:X8} \"{1}\".",
                eventInfo.Info.Thread.ThreadId,
                eventInfo.Info.Thread.ThreadInfo.Name );

            // Details.
            StringBuilder details = new StringBuilder( );
            details.AppendFormat( "{0}\n\n", overview );
            details.AppendFormat( "{0}\n", eventInfo.Info.Message );

            PrintMessage( overview.ToString( ) );
            PrintMessage( details.ToString( ) );
        }

        /// <summary>
        /// Handle the Exception (DM_EXCEPTION) notification.
        /// </summary>
        private void OnException( IXboxEventInfo eventInfo )
        {
            // Overview.
            StringBuilder overview = new StringBuilder( );
            overview.AppendFormat( "An exception occurred on thread 0x{0:X8} \"{1}\".",
                eventInfo.Info.Thread.ThreadId,
                eventInfo.Info.Thread.ThreadInfo.Name );

            // Details.
            StringBuilder details = new StringBuilder( );
            details.AppendFormat( "{0}\n\n", overview );
            details.AppendFormat( " Exception: 0x{0:X8}\n", eventInfo.Info.Code );
            details.AppendFormat( "   Address: 0x{0:X8}\n", eventInfo.Info.Address );

            StringBuilder parameters = new StringBuilder( );
            for( uint index = 0; index < eventInfo.Info.ParameterCount; ++index )
                parameters.AppendFormat( "0x{0:X8} ", eventInfo.Info.Parameters[ index ] );
            details.AppendFormat( "Parameters: {0}\n", parameters.ToString( ) );

            switch( eventInfo.Info.Code )
            {
                case 0xc0000005: // STATUS_ACCESS_VIOLATION
                    {
                        bool isReading = eventInfo.Info.Parameters[ 0 ] == 0;
                        uint address = eventInfo.Info.Parameters[ 1 ];

                        details.AppendFormat( "\nAccess violation {0} memory at 0x{1:X8}.\n",
                            isReading ? "reading" : "writing", address );
                    }
                    break;
            }

            PrintMessage( overview.ToString( ) );
            PrintMessage( details.ToString( ) );
        }

        /// <summary>
        /// Handle the RIP (DM_RIP) notification.
        /// </summary>
        private void OnRIP( IXboxEventInfo eventInfo )
        {
            // Overview.
            StringBuilder overview = new StringBuilder( );
            overview.AppendFormat( "A fatal error (RIP) occurred on thread 0x{0:X8} \"{1}\".",
                eventInfo.Info.Thread.ThreadId,
                eventInfo.Info.Thread.ThreadInfo.Name );

            // Details.
            StringBuilder details = new StringBuilder( );
            details.AppendFormat( "{0}\n\n", overview );
            details.AppendFormat( "RIP: {0}\n", eventInfo.Info.Message );

            PrintMessage( overview.ToString( ) );
            PrintMessage( details.ToString( ) );
        }

        /// <summary>
        /// Handle the ExecStateChange (DM_EXEC) notification.
        /// </summary>
        private void OnExecStateChange( IXboxEventInfo eventInfo )
        {
            // Confirm we are connected.
            if( connectionState == ConnectionState.IsConnecting )
            {
                connectionState = ConnectionState.IsConnected;

                string nameAndIP = string.Format( "{0} ({1})", Name, IPAddress );
                PrintMessage( string.Format( "Connected to {0}.", nameAndIP ) );
            }

            // Update current running process.
            new Thread( new ThreadStart( RunningProcessThread ) ).Start( );

            // Watch for reboots.
            if( eventInfo.Info.ExecState == XboxExecutionState.Rebooting ||
                eventInfo.Info.ExecState == XboxExecutionState.RebootingTitle )
                PrintMessage( "\nConsole is rebooting..." );
        }

        private void OnExecutionBreak( IXboxEventInfo eventInfo )
        {
            StringBuilder overview = new StringBuilder( );
            overview.AppendFormat( "Execution broke on thread 0x{0:X8} \"{1}\".",
                eventInfo.Info.Thread.ThreadId,
                eventInfo.Info.Thread.ThreadInfo.Name );

            StringBuilder details = new StringBuilder( );
            details.AppendFormat( "{0}\n\n", overview );
            details.AppendFormat( "{0}\n", eventInfo.Info.Message );

            PrintMessage( overview.ToString( ) );
            PrintMessage( details.ToString( ) );
        }

        /// <summary>
        /// Handler for debug notifications from the Xbox 360 console.
        /// </summary>
        void xboxConsole_OnStdNotify( XboxDebugEventType eventCode, IXboxEventInfo eventInfo )
        {
            switch( eventCode )
            {
                case XboxDebugEventType.ExecStateChange: // DM_EXEC
                    OnExecStateChange( eventInfo );
                    break;

                case XboxDebugEventType.DebugString: // DM_DEBUGSTR
                    {
                        string message = eventInfo.Info.Message.TrimEnd( " \t\n".ToCharArray( ) );

                        ParseDebugString( message );

                        ContinueExecution( eventInfo );
                    }
                    break;

                case XboxDebugEventType.ExecutionBreak:
                    OnExecutionBreak( eventInfo );
                    SaveNetworkMinidump( );
                    CrashEncountered = true;
                    break;

                case XboxDebugEventType.AssertionFailed: // DM_ASSERT
                    OnAssertionFailed( eventInfo );
                    SaveNetworkMinidump( );
                    CrashEncountered = true;
                    break;

                case XboxDebugEventType.Exception: // DM_EXCEPTION
                    {
                        // Only handle first-chance exceptions.
                        if( eventInfo.Info.Flags != XboxExceptionFlags.FirstChance )
                            break;

                        // Ignore exception caused by SetThreadName.
                        if( eventInfo.Info.Code == 0x406D1388 )
                            break;

                        OnException( eventInfo );
                    }
                    SaveNetworkMinidump( );
                    CrashEncountered = true;
                    break;

                case XboxDebugEventType.RIP: // DM_RIP
                    OnRIP( eventInfo );
                    SaveNetworkMinidump( );
                    CrashEncountered = true;
                    break;

                case XboxDebugEventType.DataBreak:
                    PrintMessage( "Thread " + eventInfo.Info.Thread + " caused a break at address " + eventInfo.Info.Address + "!" );
                    SaveNetworkMinidump( );
                    CrashEncountered = true;
                    break;
            }
        }

        public void LogInDefaultUser( )
        {
            ConsoleProfilesManager profilesManager = xboxConsole.CreateConsoleProfilesManager( );

            PrintMessage( "Enumerating profiles on console " + xboxConsole.Name + "." );
            IEnumerable<ConsoleProfile> profiles = profilesManager.EnumerateConsoleProfiles( );
            if( !profiles.Any( ) )
                throw new Exception( "No profiles were found on the console!" );
            else
            {
                PrintMessage( "Profiles found:" );
                foreach( var profile in profiles )
                    PrintMessage( "\t" + profile );
            }

            ConsoleProfile firstProfile = profiles.First( );
            PrintMessage( "First profile found is " + firstProfile + "." );
            PrintMessage( "Subscription tier for profile " + firstProfile + " is " + firstProfile.Tier + "." );

            if( firstProfile.GetUserSigninState( ) == SignInState.SignedInToLive )
            {
                PrintMessage( "The current profile is already signed into the LIVE service." );
                return;
            }

            PrintMessage( "The current profile is not signed into LIVE. Attempting to sign in..." );

            profilesManager.SignOutAllUsers( );

            PrintMessage( "Signing in profile " + firstProfile + "." );
            firstProfile.SignIn( UserIndex.Zero );

            PrintMessage( "User index for profile " + firstProfile + " is " + firstProfile.GetUserIndex( ) + "." );

            SignInState signinState = firstProfile.GetUserSigninState( );
            PrintMessage( "Sign-in state for profile " + firstProfile + " is " + signinState + "." );
            if( signinState == SignInState.SignedInLocally )
                PrintMessage( "!WARNING! The current profile is signed in locally, but is not connected to the LIVE service!" );
            else if( signinState == SignInState.NotSignedIn )
                throw new Exception( "!WARNING! The current profile could not be signed in!" );
        }
    }

    public enum ConnectionState
    {
        NotConnected,
        IsConnecting,
        IsConnected,
    }
}
