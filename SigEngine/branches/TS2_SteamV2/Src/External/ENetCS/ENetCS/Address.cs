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
using System.Net;
using System.Text;

namespace ENet
{
    public unsafe struct Address : IEquatable<Address>
    {
        public const uint IPv4HostAny = Native.ENET_HOST_ANY;
        public const uint IPv4HostBroadcast = Native.ENET_HOST_BROADCAST;

        Native.ENetAddress _address;

        public override bool Equals(object obj)
        {
            return obj is Address && Equals((Address)obj);
        }

        public bool Equals(Address address)
        {
            return Type == address.Type && Port == address.Port
                && Native.memcmp(GetHostBytes(), address.GetHostBytes());
        }

        public override int GetHashCode()
        {
            return Type.GetHashCode() ^ Port.GetHashCode() ^ IPv4Host.GetHashCode();
        }

        public byte[] GetHostBytes()
        {
            return BitConverter.GetBytes(IPAddress.NetworkToHostOrder((int)IPv4Host));
        }

        public string GetHostName()
        {
            byte[] name = new byte[256];
            fixed (byte* hostName = name)
            {
                if (Native.enet_address_get_host(ref _address, hostName, (IntPtr)name.Length) < 0)
                    { return null; }
            }
            return BytesToString(name);
        }

        public string GetHostIP()
        {
            byte[] ip = new byte[256];
            fixed (byte* hostIP = ip)
            {
                if (Native.enet_address_get_host_ip(ref _address, hostIP, (IntPtr)ip.Length) < 0)
                    { return null; }
            }
            return BytesToString(ip);
        }

        public bool SetHost(string hostName)
        {
            if (hostName == null) { throw new ArgumentNullException("hostName"); }
            return Native.enet_address_set_host(ref _address,
                Encoding.ASCII.GetBytes(hostName)) == 0;
        }

        static string BytesToString(byte[] bytes)
        {
            try { return Encoding.ASCII.GetString(bytes, 0, Native.strlen(bytes)); }
            catch { return null; }
        }

        public Native.ENetAddress NativeData
        {
            get { return _address; }
            set { _address = value; }
        }

        public uint IPv4Host
        {
            get { return _address.host; }
            set { _address.host = value; }
        }

        public ushort Port
        {
            get { return _address.port; }
            set { _address.port = value; }
        }

        public AddressType Type
        {
            get { return AddressType.IPv4; }
        }
    }
}
