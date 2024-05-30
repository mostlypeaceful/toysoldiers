using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ComponentModel;
using System.Threading;
//using System.Diagnostics.Process;

namespace PlatformMonitor
{
    public class PlatformMonitorServer
    {
        const int  CHANNEL_LIMIT = 200;
        const int  PEER_LIMIT = 32;
        const uint TIMEOUT_LIMIT = 100;

        PlatformMonitorForm form = null;

        System.Diagnostics.Process cloudStorageHost = null;

        ENet.Host host;
        List<PlatformMonitorClient> clientList = new List<PlatformMonitorClient>( );
        PlatformMonitorClient currClient = null;

        BackgroundWorker pollingWorker = new BackgroundWorker( );
        
        public PlatformMonitorServer( PlatformMonitorForm form )
        {
            string path = Environment.GetEnvironmentVariable( "SigEngine" );
            if( String.IsNullOrWhiteSpace( path ) )
            {
                throw new ArgumentNullException( "EnginePath", "SigEngine is null or empty!" );
            }
            else
            {
                // NOTE:
                // If we need more control over cloud storage host, then we 
                // should probably integrate it in a better way, i.e. shared
                // library or directly pulling source into this project. But
                // for now we just spawn the process and kill when this app
                // is finished.
                cloudStorageHost = new System.Diagnostics.Process( );
                cloudStorageHost.StartInfo.FileName = path + "\\bin\\CloudStorageHost.exe";
                // add more args as necessary
                cloudStorageHost.Start( );
            }

            this.form = form;
            
            ENet.Library.Initialize( );

            host = new ENet.Host( );

            // create a server that listens for any IPs
            ENet.Address address = new ENet.Address();
            address.IPv4Host = ENet.Address.IPv4HostAny;
            address.Port = 6666;
            host.Create( address, PEER_LIMIT );
                        
            pollingWorker.WorkerSupportsCancellation = true;
            pollingWorker.DoWork += new DoWorkEventHandler( pollingWorker_DoWork );
            pollingWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler( pollingWorker_RunWorkerCompleted );

            pollingWorker.RunWorkerAsync();
        }

        void pollingWorker_RunWorkerCompleted( object sender, RunWorkerCompletedEventArgs e )
        {
        }

        unsafe void pollingWorker_DoWork( object sender, DoWorkEventArgs e )
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
                            // get the IP address of the connected client
                            string ip = GetIpFromEvent( eventReceived );
                            Console.WriteLine( "Client connected from " + ip );

                            uint timeout = eventReceived.Data;
                            ENet.Native.ENetPeer* peer = eventReceived.NativeData.peer;
                            ENet.Native.enet_peer_timeout( peer,
                                                           TIMEOUT_LIMIT,
                                                           timeout,
                                                           timeout);

                            bool empty = clientList.Count( ) == 0;

                            // add the client to the list and update the form
                            clientList.Add( new PlatformMonitorClient( eventReceived.Peer, ip, timeout ) );
                            form.OnClientConnected( ip );

                            if( empty )
                                currClient = clientList.ElementAt( 0 );
                        }
                        break;

                    case ENet.EventType.Receive:
                        {
                            // get the IP address and see if its our current client
                            string ip = GetIpFromEvent( eventReceived );
                            if (currClient.IP == ip)
                            {
                                byte[] data = eventReceived.Packet.GetBytes();

                                // get the IP address of the connected client
                                Console.WriteLine("Client received data on channel " + eventReceived.ChannelID + ". Size:" + data.Length.ToString() + ".");

                                char[] ch = System.Text.Encoding.UTF8.GetChars(data);
                                string str = new string(ch);
                                string[] strSplit = str.Split('\0');

                                if (strSplit.Length > 0)
                                {
                                    string command = strSplit[0];
                                    string payload = strSplit.Length > 1 ? strSplit[1] : "";
                                    HandleCommand(command, payload);
                                }
                            }
                            eventReceived.Packet.Dispose( );
                        }
                        break;

                    case ENet.EventType.Disconnect:
                        {
                            string ip = GetIpFromEvent( eventReceived );
                            Console.WriteLine( "Client disconnected from " + ip );

                            // remove the client from the list (if it exists)
                            form.OnClientDisconnected( ip );
                            clientList.RemoveAll( delegate( PlatformMonitorClient c ) { return c.IP == ip; } );
                        }
                        break;
                    }
                }
            }
        }

        ~PlatformMonitorServer( )
        {
            foreach( PlatformMonitorClient client in clientList )
            {
                client.peer.DisconnectNow( 0 );
            }

            ENet.Library.Deinitialize( );

            if( cloudStorageHost != null )
                cloudStorageHost.Kill( );
        }

        unsafe public string GetIpFromEvent( ENet.Event evt )
        {
            ENet.Address addr = new ENet.Address( );
            addr.IPv4Host = evt.NativeData.peer->address.host;
            return addr.GetHostIP( );
        }
        
        public bool SendCommand( string command, string payload )
        {
            return currClient.peer.Send( 0, System.Text.Encoding.UTF8.GetBytes( command + '\0' + payload + '\0' ) );
        }

        unsafe public void RecvClientAssert( string payload )
        {
            // disable the timeout while we wait for a response
            ENet.Native.ENetPeer* peer = currClient.peer.NativeData;
            ENet.Native.enet_peer_timeout( peer,
                                           TIMEOUT_LIMIT,
                                           uint.MaxValue,
                                           uint.MaxValue );
            form.OnClientAssert( payload );
        }

        unsafe public bool SendAssertResponse( string response )
        {
            // reset the timeout on this peer
            ENet.Native.ENetPeer* peer = currClient.peer.NativeData;
            ENet.Native.enet_peer_timeout( peer,
                                           TIMEOUT_LIMIT,
                                           currClient.timeout,
                                           currClient.timeout );
            return SendCommand( "assertresponse", response );
        }
        
        public bool SetCurrentClient( string ip )
        {
            PlatformMonitorClient found = clientList.Find( delegate( PlatformMonitorClient c ) { return c.IP == ip; } );
            bool changed = ( found != currClient );
            currClient = found;
            return changed;
        }

        public bool ForceDisconnect( string ip )
        {
            PlatformMonitorClient found = clientList.Find(delegate(PlatformMonitorClient c) { return c.IP == ip; });
            if( found != null )
            {
                found.peer.DisconnectNow( 0 );
                return true;
            }
            return false;
        }

        public void HandleCommand( string command, string payload )
        {
            switch( command )
            {
            case "output":
                form.OnClientOutput( payload );
                break;

            case "assert":
                RecvClientAssert(payload);
                break;

            default:
                Console.WriteLine( "Error, unhandled command type [" + command + "]" );
                break;
            }
        }
    }
}
