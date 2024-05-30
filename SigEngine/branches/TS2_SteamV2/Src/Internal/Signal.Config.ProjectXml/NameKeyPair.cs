//------------------------------------------------------------------------------
// \file NameKeyPair.cs - 1 May 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

using System.Xml;

namespace Signal.Config
{
	public class NameKeyPair
	{
		public string	Name;
		public uint		Key;

		public NameKeyPair( string name, uint key )
		{
			Name = name;
			Key = key;
		}

		public NameKeyPair( XmlNode node )
		{
			Name	= node.SelectSingleNode( "Name" ).InnerText;
			Key		= uint.Parse( node.SelectSingleNode( "Key" ).InnerText );
		}

		public override string ToString( ) { return Name; }
	}
}
