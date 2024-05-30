//------------------------------------------------------------------------------
// \file ResourceFinder.cs - 25 Sep 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------

using System.Threading;
using System.Collections.Generic;
using System.Linq;
using System.IO;
using System;

namespace ResourceMover
{
	public class ResourceFinder
	{
		public ManualResetEvent Event { get; private set; }
		public IList<string> Results { get { return mResults; } }
		public IList<string> Filters { get { return mFilters; } }
		public string RootDirectory { get; private set; }

		public ResourceFinder( IEnumerable<string> filters, string root )
		{
			mFilters = filters.ToArray( );
			List<string> test = new List<string>( );
			RootDirectory = root;
			Event = new ManualResetEvent( false );
		}

		public void DoWork( object ignored )
		{
			mResults.Clear( );
			foreach( var filter in mFilters )
				mResults.AddRange( Directory.GetFiles( RootDirectory, filter, SearchOption.AllDirectories ) );
			Event.Set( );
		}

		public string RelativeResult( int index )
		{
			Uri uri1 = new Uri( RootDirectory );
			Uri uri2 = new Uri( mResults[ index ]);

			Uri relativeUri = uri1.MakeRelativeUri( uri2 );

			return relativeUri.ToString( );
		}

		private List<string> mResults = new List<string>( );
		private string[] mFilters;
	}
}