//------------------------------------------------------------------------------
// \file ProjectXml.cs - 1 May 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;

namespace Signal.Config
{
	public class ProjectXml
	{
		public readonly List< NameKeyPair >	GameFlags		= new List<NameKeyPair>( );
		public readonly List< GameEnum >	GameEnums		= new List<GameEnum>( );
		public readonly List< NameKeyPair >	GameEvents		= new List<NameKeyPair>( );
		public readonly List< NameKeyPair >	KeyframeEvents	= new List<NameKeyPair>( );
		public readonly List< NameKeyPair >	AIFlags			= new List<NameKeyPair>( );

		public void CreateGameEnum( string name )
		{
			GameEnums.Add( new GameEnum( name, GameEnums.Max( ge => ge.Key ) + 1 ) );
		}

		const string
			cGameFlags				= "GameFlags",
			cGameEnumeratedValues	= "GameEnumeratedValues",
			cGameEvents				= "GameEvents",
			cKeyframeEvents			= "KeyframeEvents",
			cAIFlags				= "AIFlags",
			cName					= "Name",
			cKey					= "Key",
			cItem					= "i",
			cCount					= "count";

		public static ProjectXml LoadFrom( XmlDocument doc )
		{
			var gf = new ProjectXml( );
			gf.GameFlags		.AddRange( doc.SelectNodes( "SigProj/GameFlags/i"				).Cast<XmlNode>( ).Select( node => new NameKeyPair( node ) ) );
			gf.GameEnums		.AddRange( doc.SelectNodes( "SigProj/GameEnumeratedValues/i"	).Cast<XmlNode>( ).Select( node => new GameEnum( node ) ) );
			gf.GameEvents		.AddRange( doc.SelectNodes( "SigProj/GameEvents/i"				).Cast<XmlNode>( ).Select( node => new NameKeyPair( node ) ) );
			gf.KeyframeEvents	.AddRange( doc.SelectNodes( "SigProj/KeyframeEvents/i"			).Cast<XmlNode>( ).Select( node => new NameKeyPair( node ) ) );
			gf.AIFlags			.AddRange( doc.SelectNodes( "SigProj/AIFlags/i"					).Cast<XmlNode>( ).Select( node => new NameKeyPair( node ) ) );
			return gf;
		}

		public void SaveTo( XmlDocument doc )
		{
			SaveNodes( SelectOrCreateSingleNode( doc, "SigProj/GameFlags"			), GameFlags );
			SaveNodes( SelectOrCreateSingleNode( doc, "SigProj/GameEnumeratedValues"), GameEnums );
			SaveNodes( SelectOrCreateSingleNode( doc, "SigProj/GameEvents"			), GameEvents );
			SaveNodes( SelectOrCreateSingleNode( doc, "SigProj/KeyframeEvents"		), KeyframeEvents );
			SaveNodes( SelectOrCreateSingleNode( doc, "SigProj/AIFlags"				), AIFlags );
		}

		public static ProjectXml LoadFrom( string path )
		{
			var doc = new XmlDocument( );
			doc.Load( path );
			return LoadFrom( doc );
		}

		public void SaveTo( string path )
		{
			var doc = new XmlDocument( );
			doc.Load( path );
			SaveTo( doc );
			using( var writer = XmlWriter.Create( path, XmlWriterSettings ) )
				doc.Save( writer );
		}

		static readonly XmlWriterSettings XmlWriterSettings = new XmlWriterSettings( )
		{
			// Preserve existing SigEngine XML format to minimize diffs
			Indent		= true,
			IndentChars	= "    ",
			Encoding	= Encoding.GetEncoding("ISO-8859-1"),
		};

		XmlNode SelectOrCreateSingleNode( XmlDocument doc, string xpath )
		{
			var node = doc.SelectSingleNode( xpath );
			if( node == null )
			{
				var sIndex = xpath.LastIndexOf( '/' );
				if( sIndex == -1 )
					return null;

				var parent = SelectOrCreateSingleNode( doc, xpath.Substring(0,sIndex) );
				node = parent.AppendChild( doc.CreateElement( xpath.Substring(sIndex+1) ) );
			}
			return node;
		}

		XmlNode AddElement( XmlNode parent, string name )
		{
			return parent.AppendChild( parent.OwnerDocument.CreateElement( name ) );
		}

		XmlNode AddElementValueCD( XmlNode parent, string name, string value )
		{
			var e = parent.AppendChild( parent.OwnerDocument.CreateElement( name ) );
			e.AppendChild( parent.OwnerDocument.CreateCDataSection( value ) );
			return e;
		}

		XmlNode AddElementValue( XmlNode parent, string name, string value )
		{
			var e = parent.AppendChild( parent.OwnerDocument.CreateElement( name ) );
			e.AppendChild( parent.OwnerDocument.CreateTextNode( value ) );
			return e;
		}

		void AddAttribute( XmlNode node, string name, string value )
		{
			var doc = node.OwnerDocument;
			node.Attributes.Append( doc.CreateAttribute( name ) ).Value = value;
		}

		XmlNode AddElementArray( XmlNode parent, string name, Func<XmlNode,IEnumerable<XmlNode>> nodesF )
		{
			var element = AddElement( parent, name );
			var count = 0;
			foreach( var node in nodesF(element) )
			{
				element.AppendChild( node );
				++count;
			}
			AddAttribute( element, "count", count.ToString() );
			return element;
		}

		IEnumerable<XmlNode> SaveNodes( XmlNode arrayNode, IEnumerable<NameKeyPair> nameKeys )
		{
			if( arrayNode == null && !nameKeys.Any() )
				return new XmlNode[0];

			var nodes = new List<XmlNode>( );
			arrayNode.RemoveAll( );
			var count = 0;

			foreach( var nameKey in nameKeys )
			{
				var i = AddElement( arrayNode, "i" );
				AddElementValueCD( i, "Name", nameKey.Name );
				AddElementValue( i, "Key", nameKey.Key.ToString( ) );
				nodes.Add( i );
				++count;
			}
			
			AddAttribute( arrayNode, "count", count.ToString() );
			return nodes;
		}

		IEnumerable<XmlNode> SaveNodes( XmlNode arrayNode, IEnumerable<GameEnum> enums )
		{
			var nodes = new List<XmlNode>( );
			arrayNode.RemoveAll( );

			foreach( var e in enums )
			{
				var i = AddElement( arrayNode, "i" );
				AddElementValueCD( i, "Name", e.Name );
				AddElementValue( i, "Key", e.Key.ToString( ) );
				AddElementValue( i, "Hide", e.Hide ? "1" : "0" );
				AddElementArray( i, "Aliases", aliasesNode => SaveNodes( aliasesNode, e.Aliases ) );
				AddElementArray( i, "Values", valuesNode => SaveNodes( valuesNode, e.Values ) );
				nodes.Add( i );
			}

			AddAttribute( arrayNode, "count", nodes.Count.ToString( ) );
			return nodes;
		}
	}
}
