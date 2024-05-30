using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace PlatformMonitor
{
    public class PlatformMonitorClient
    {
        public ENet.Peer peer;
        public string ip;
        public uint timeout;

        public PlatformMonitorClient( ENet.Peer peer, string ip, uint timeout )
        {
            this.peer = peer;
            this.ip = ip;
            this.timeout = timeout;
        }

        public string IP
        {
            get { return ip; }
            set { ip = value;  }
        }

        public uint Timeout
        {
            get { return timeout; }
            set { timeout = value; }
        }

        public bool Equal( PlatformMonitorClient c ) 
        { 
            return c.IP == ip; 
        }
    }
}
