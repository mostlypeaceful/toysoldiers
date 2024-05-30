//------------------------------------------------------------------------------
// \file GameEnum.cs - 1 May 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

using System.Collections.Generic;
using System.Linq;
using System.Xml;

namespace Signal.Config
{
	public class GameEnum
	{
		public string						Name;
		public uint							Key;
		public bool							Hide;
		public readonly List<NameKeyPair>	Aliases = new List<NameKeyPair>( );
		public readonly List<NameKeyPair>	Values = new List<NameKeyPair>( );

		public GameEnum( string name, uint key )
		{
			Name	= name;
			Key		= key;
		}

		public GameEnum( XmlNode enumNode )
		{
			Name	= enumNode.SelectSingleNode( "Name" ).InnerText;
			Key		= uint.Parse( enumNode.SelectSingleNode( "Key" ).InnerText );
			Hide	= enumNode.SelectSingleNode( "Hide" ).InnerText == "1";
			Aliases = enumNode.SelectNodes( "Aliases/i" ).Cast<XmlNode>( ).Select( valueNode => new NameKeyPair( valueNode ) ).ToList( );
			Values	= enumNode.SelectNodes( "Values/i" ).Cast<XmlNode>( ).Select( valueNode => new NameKeyPair( valueNode ) ).ToList( );
		}

		public override string ToString( ) { return Name; }
	}
}
