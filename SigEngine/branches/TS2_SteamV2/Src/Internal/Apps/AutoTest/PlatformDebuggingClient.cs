using System;
using System.ComponentModel;
using System.Threading;

namespace AutoTest
{
    public class PlatformDebuggingClient
    {
        const int CHANNEL_LIMIT = 200;

        ENet.Host host;
        ENet.Peer peer;
        ENet.Address address = new ENet.Address( );

        BackgroundWorker pollingWorker = new BackgroundWorker( );
        AutoResetEvent pollingWorkerHasEndedNotifier = new AutoResetEvent( false );

        bool pendingReconnect = false;

        public PlatformDebuggingClient( string hostIP )
        {
            ENet.Library.Initialize( );

            host = new ENet.Host( );

            // Set the address to null so we aren't listening
            // for connections - we're a client, not a host
            host.Create( null, 1 );

            address.SetHost( hostIP );
            address.Port = 6666;

            pollingWorker.WorkerSupportsCancellation = true;
            pollingWorker.DoWork += new DoWorkEventHandler( pollingWorker_DoWork );
            pollingWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler( pollingWorker_RunWorkerCompleted );
        }

        void pollingWorker_RunWorkerCompleted( object sender, RunWorkerCompletedEventArgs e )
        {
            if( pendingReconnect )
            {
                peer = host.Connect( address, CHANNEL_LIMIT, 1234 );
                pollingWorker.RunWorkerAsync( );

                pendingReconnect = false;
            }
        }

        void pollingWorker_DoWork( object sender, DoWorkEventArgs e )
        {
            // Check once per second for new ENet messages
            while( host.Service( 1 ) >= 0 )
            {
                if( pollingWorker.CancellationPending )
                {
                    e.Cancel = true;
                    break;
                }

                ENet.Event eventReceived;
                while( host.CheckEvents( out eventReceived ) > 0 )
                {
                    switch( eventReceived.Type )
                    {
                        case ENet.EventType.Connect:
                            {
                                Console.WriteLine( "Client connected to " + eventReceived.Type.ToString( ) + "." );
                            }
                            break;
                        case ENet.EventType.Receive:
                            {
                                byte[ ] data = eventReceived.Packet.GetBytes( );

                                Console.WriteLine( "Client received data on channel " + eventReceived.ChannelID + ". Size:" + data.Length.ToString( ) + "." );

                                eventReceived.Packet.Dispose( );
                            }
                            break;
                        case ENet.EventType.Disconnect:
                            {
                                Console.WriteLine( "Client disconnected from " + eventReceived.Type.ToString( ) + "." );
                            }
                            break;
                    }
                }
            }
        }

        ~PlatformDebuggingClient( )
        {
            if( peer.State == ENet.PeerState.Connected )
                peer.Disconnect( 0 );

            ENet.Library.Deinitialize( );
        }

        public bool Send( string message )
        {
            return peer.Send( 0, System.Text.Encoding.UTF8.GetBytes( message + '\0' ) );
        }

        public void Reconnect( string newServerIP = null )
        {
            pendingReconnect = true;

            if( peer.IsSet && peer.State != ENet.PeerState.Disconnected )
                peer.DisconnectNow( 0 );
            else
            {
                if( newServerIP != null )
                    address.SetHost( newServerIP );

                peer = host.Connect( address, 1, 0 );
            }

            if( pollingWorker.IsBusy )
                pollingWorker.CancelAsync( );
            else
                pollingWorker.RunWorkerAsync( );
        }

        public bool IsConnected
        {
            get { return ( peer.State == ENet.PeerState.Connected ); }
        }
    }
}
