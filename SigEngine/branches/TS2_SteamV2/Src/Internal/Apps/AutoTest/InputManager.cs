using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Threading;

namespace AutoTest
{
    public class InputManager
    {
        private PrintMessageCallback PrintMessage = null;
        private InputQueueCompletedCallback InputQueueCompleted = null;
        private Queue<string> responsesWeAreWaitingFor = new Queue<string>( );
        private bool responseReceived = false;

        List<string> buttonsHeldTogether = new List<string>( );
        PlatformDebuggingClient debuggingClient = null;

        public InputManager( ref PlatformDebuggingClient debuggingClient, PrintMessageCallback PrintMessage )
        {
            this.debuggingClient = debuggingClient;
            this.PrintMessage = PrintMessage;
        }

        ~InputManager( )
        {
        }

        public bool QueueButtonSequence( string scriptPath, InputQueueCompletedCallback InputQueueCompleted )
        {
            this.InputQueueCompleted = InputQueueCompleted;

            BackgroundWorker queueRunner = new BackgroundWorker( );
            queueRunner.DoWork += RunScriptedInput;
            queueRunner.RunWorkerCompleted += CheckQueueStatusCompleted;
            queueRunner.RunWorkerAsync( scriptPath );

            return false;
        }

        public void SetButtonFlags( string[ ] buttons )
        {
            buttonsHeldTogether.Clear( );
            buttonsHeldTogether.AddRange( buttons );
        }

        public void PressButtons( int framesToHold )
        {
            if( !debuggingClient.IsConnected )
            {
                debuggingClient.Reconnect( );

                // Wait for the successful connection
                for( int i = 0; i < 5; ++i )
                {
                    Thread.Sleep( 1000 );
                    if( debuggingClient.IsConnected )
                        break;
                }
                if( !debuggingClient.IsConnected )
                    throw new Exception( "Couldn't connect to host while trying to send a button sequence!" );
            }

            string message = "buttonsequence\0" + Convert.ToString( framesToHold );
            foreach( string button in buttonsHeldTogether )
                message += " " + button;

            // "Press" the buttons
            Console.WriteLine( message );
            debuggingClient.Send( message );
            Thread.Sleep( framesToHold );
        }

        private void RunScriptedInput( object sender, DoWorkEventArgs e )
        {
            responseReceived = false;

            string scriptPath = ( string ) e.Argument;
            PrintMessage( "Opening button sequence script " + scriptPath + "..." );

            try
            {
                using( StreamReader scriptFile = new StreamReader( scriptPath ) )
                {
                    PrintMessage( "Queuing button sequence..." );
                    string line = "";
                    while( ( line = scriptFile.ReadLine( ) ) != null )
                    {
                        // Split the line into buttons and duration
                        string[ ] components = line.Split( '\t' );

                        int holdCount = Convert.ToInt32( components[ 0 ] );
                        string buttons = components[ 1 ];

                        // Set a list of buttons to be held at once
                        SetButtonFlags( buttons.Split( '+' ) );

                        if( buttons == "WaitForResponse" )
                        {
                            PrintMessage( "Waiting for " + holdCount + " frame(s) for the following responses:" );

                            // This is what TestMessageReceived() will look at when parsing messages
                            for( uint i = 2; i < components.Length; ++i )
                            {
                                PrintMessage( "*** " + components[ i ] + " ***" );
                                responsesWeAreWaitingFor.Enqueue( components[ i ] );
                            }

                            for( int i = holdCount; i > 0 && !responseReceived; --i )
                                Thread.Sleep( 1 );

                            if( !responseReceived )
                                throw new Exception( "No response received while waiting for response(s)!" );
                        }
                        else
                        {
                            PrintMessage( "Pressing " + buttons + " for " + holdCount + " second(s)..." );
                            PressButtons( holdCount );
                        }
                    }
                }
            }
            catch( System.Exception ex )
            {
                PrintMessage( "ERROR: " + ex.Message );
                throw(ex);
            }
        }

        private void CheckQueueStatusCompleted( object sender, RunWorkerCompletedEventArgs e )
        {
            bool success = false;
            if( e.Error != null )
            {
                PrintMessage( "Could not queue sequence!" );
                success = false;
            }
            else
            {
                PrintMessage( "The queued input has completed." );
                success = true;
            }
            
            InputQueueCompleted( success );
        }

        public bool CheckResponse( string testMessage )
        {
            string message = testMessage.Replace( "@Test@", "" );
            if( responsesWeAreWaitingFor.Count > 0 && message == responsesWeAreWaitingFor.Peek( ) )
            {
                responsesWeAreWaitingFor.Dequeue( );
                if( responsesWeAreWaitingFor.Count <= 0 )
                    responseReceived = true;

                return false;
            }

            return true;
        }
    }
}
