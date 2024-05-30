//------------------------------------------------------------------------------
// \file ReferenceExtensionTable.cs - 26 Sep 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------

using System.Collections.Generic;
using System.IO;

namespace ResourceMover
{
	public class ReferenceExtensionList
	{
		public string[] Extensions { get; private set; }
		public ReferenceExtensionList( string[] exts )
		{
			Extensions = exts;
		}

		public string[] CullPaths( string[] paths )
		{
			var toKeep = new List<string>( paths.Length );
			foreach( string path in paths )
			{
				if( path.Length > 0 )
				{
					string pathExt = Path.GetExtension( path );
					foreach( string ext in Extensions )
					{
						if( pathExt.Equals( ext, System.StringComparison.InvariantCultureIgnoreCase ) )
						{
							toKeep.Add( path );
							break;
						}
					}
				}
				else toKeep.Add( path );
			}

			return toKeep.ToArray( );
		}
	}

	static public class ReferenceExtensionTable
	{
		public static string[] SupportedExtensions { get; private set; }

		public static ReferenceExtensionList GetReferenceList( string type )
		{
			ReferenceExtensionList refList;
			mReferenceTable.TryGetValue( type, out refList );
			return refList;
		}

		public static string[] CullPaths( string ext, string[] paths )
		{
			ReferenceExtensionList list;
			if( mReferenceTable.TryGetValue( ext, out list ) )
				return list.CullPaths( paths );
			return paths;
		}

		static ReferenceExtensionTable( )
		{
			SupportedExtensions = new[] { ".xlsx", ".tab", ".sigml", ".mshml", ".animl", ".nut", ".goaml", ".tatml", ".fxml", ".derml", ".sklml", ".anipk", ".tiledml" };

			mReferenceTable = new Dictionary<string, ReferenceExtensionList>( );

			mReferenceTable[ ".sigml" ]	  = new ReferenceExtensionList( new[] { ".sigml", ".mshml", ".nut", ".tatml", ".fxml" } );
			mReferenceTable[ ".mshml" ]	  = new ReferenceExtensionList( new[] { ".mb", ".ma", ".tga", ".png", ".bmp", ".dds", ".jpg", ".tatml", ".derml", ".sklml" } );
			mReferenceTable[ ".animl" ]	  = new ReferenceExtensionList( new[] { ".mb", ".ma", ".sklml" } );
			mReferenceTable[ ".nut" ]	  = new ReferenceExtensionList( new[] { ".sigml", ".mshml", ".nut", ".tga", ".png", ".bmp", ".dds", ".jpg", ".goaml", ".anipk" });
			mReferenceTable[ ".goaml" ]	  = new ReferenceExtensionList( new[] { ".goaml", ".nut" } );
			mReferenceTable[ ".tatml" ]	  = new ReferenceExtensionList( new[] { ".tga", ".png", ".bmp", ".dds", ".jpg" } );
			mReferenceTable[ ".fxml" ]	  = new ReferenceExtensionList( new[] { ".tga", ".png", ".bmp", ".dds", ".jpg", ".mshml", ".derml", ".sigml", ".nut" } );
			mReferenceTable[ ".derml" ]	  = new ReferenceExtensionList( new[] { ".tga", ".png", ".bmp", ".dds", ".jpg" } );
			mReferenceTable[ ".sklml" ]	  = new ReferenceExtensionList( new[] { ".mb", ".ma" } );
			mReferenceTable[ ".anipk" ]	  = new ReferenceExtensionList( new[] { ".sklml" } );
			mReferenceTable[ ".tiledml" ] = new ReferenceExtensionList( new[] { ".sigml", ".mshml" } );
		}

		private static Dictionary<string, ReferenceExtensionList> mReferenceTable;
	}
}
