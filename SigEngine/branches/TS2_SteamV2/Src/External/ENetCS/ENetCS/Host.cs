﻿#region License
/*
ENet for C#
Copyright (c) 2011 James F. Bellinger <jfb@zer7.com>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
#endregion

using System;

namespace ENet
{
    public unsafe class Host : IDisposable
    {
        Native.ENetHost* _host;

        public Host()
        {

        }

        ~Host()
        {
            Dispose(false);
        }

        void CheckChannelLimit(int channelLimit)
        {
            if (channelLimit < 0 || channelLimit > Native.ENET_PROTOCOL_MAXIMUM_CHANNEL_COUNT)
                { throw new ArgumentOutOfRangeException("channelLimit"); }
        }

        void CheckCreated()
        {
            if (_host == null) { throw new InvalidOperationException("Not created."); }
        }

        public void Create(ushort port, int peerLimit)
        {
            Address address = new Address(); address.Port = port;
            Create(address, peerLimit);
        }

        public void Create(Address? address, int peerLimit)
        {
            Create(address, peerLimit, 0);
        }

        public void Create(Address? address, int peerLimit, int channelLimit)
        {
            Create(address, peerLimit, channelLimit, 0, 0);
        }

        public void Create(Address? address, int peerLimit, int channelLimit, uint incomingBandwidth, uint outgoingBandwidth)
        {
            if (_host != null) { throw new InvalidOperationException("Already created."); }
            if (peerLimit < 0 || peerLimit > Native.ENET_PROTOCOL_MAXIMUM_PEER_ID)
                { throw new ArgumentOutOfRangeException("peerLimit"); }
            CheckChannelLimit(channelLimit);

            if (address != null)
            {
                Native.ENetAddress nativeAddress = address.Value.NativeData;
                _host = Native.enet_host_create(ref nativeAddress, (IntPtr)peerLimit,
                    (IntPtr)channelLimit, incomingBandwidth, outgoingBandwidth);
            }
            else
            {
                _host = Native.enet_host_create(null, (IntPtr)peerLimit,
                    (IntPtr)channelLimit, incomingBandwidth, outgoingBandwidth);
            }
            if (_host == null) { throw new ENetException(0, "Host creation call failed."); }
        }

        public void Dispose()
        {
            Dispose(true);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (_host != null)
            {
                Native.enet_host_destroy(_host);
                _host = null;
            }
        }

        public void Broadcast(byte channelID, ref Packet packet)
        {
            CheckCreated(); packet.CheckCreated();
            Native.enet_host_broadcast(_host, channelID, packet.NativeData);
            packet.NativeData = null; // Broadcast automatically clears this.
        }

        public void CompressWithRangeEncoder()
        {
            CheckCreated();
            Native.enet_host_compress_with_range_encoder(_host);
        }

        public void DoNotCompress()
        {
            CheckCreated();
            Native.enet_host_compress(_host, null);
        }

        public int CheckEvents(out Event @event)
        {
            CheckCreated(); Native.ENetEvent nativeEvent;
            int ret = Native.enet_host_check_events(_host, out nativeEvent);
            if (ret <= 0) { @event = new Event(); return ret; }
            @event = new Event(nativeEvent); return ret;
        }

        public Peer Connect(Address address, int channelLimit, uint data)
        {
            CheckCreated(); CheckChannelLimit(channelLimit);

            Native.ENetAddress nativeAddress = address.NativeData;
            Peer peer = new Peer(Native.enet_host_connect(_host, ref nativeAddress, (IntPtr)channelLimit, data));
            if (peer.NativeData == null) { throw new ENetException(0, "Host connect call failed."); }
            return peer;
        }

        public void Flush()
        {
            CheckCreated();
            Native.enet_host_flush(_host);
        }

        public int Service(int timeout)
        {
            if (timeout < 0) { throw new ArgumentOutOfRangeException("timeout"); }
            CheckCreated(); return Native.enet_host_service(_host, null, (uint)timeout);
        }

        public int Service(int timeout, out Event @event)
        {
            if (timeout < 0) { throw new ArgumentOutOfRangeException("timeout"); }
            CheckCreated(); Native.ENetEvent nativeEvent;

            int ret = Native.enet_host_service(_host, out nativeEvent, (uint)timeout);
            if (ret <= 0) { @event = new Event(); return ret; }
            @event = new Event(nativeEvent); return ret;
        }

        public void SetBandwidthLimit(uint incomingBandwidth, uint outgoingBandwidth)
        {
            CheckCreated();
            Native.enet_host_bandwidth_limit(_host, incomingBandwidth, outgoingBandwidth);
        }

        public void SetChannelLimit(int channelLimit)
        {
            CheckChannelLimit(channelLimit); CheckCreated();
            Native.enet_host_channel_limit(_host, (IntPtr)channelLimit);
        }

        public bool IsSet
        {
            get { return _host != null; }
        }

        public Native.ENetHost* NativeData
        {
            get { return _host; }
            set { _host = value; }
        }
    }
}
