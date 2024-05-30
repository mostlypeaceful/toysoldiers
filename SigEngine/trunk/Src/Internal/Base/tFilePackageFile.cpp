#include "BasePch.hpp"
#include "tFilePackageFile.hpp"
#include "tResource.hpp"

namespace Sig
{
	///
	/// \section tFilePackageFile
	///

	const char* tFilePackageFile::fGetFileExtension( )
	{
		return ".sacb";
	}

	define_lip_version( tFilePackageFile, 1, 0, 1 );

	tFilePackageFile::tFilePackageFile( )
		: mHeaderTableSize( 0 )
	{
	}

	tFilePackageFile::tFilePackageFile( tNoOpTag )
		: tLoadInPlaceFileBase( cNoOpTag )
		, mFileHeaders( cNoOpTag )
	{
	}


	///
	/// \section tFilePackage::tFileHeader
	///

	tFilePackageFile::tFileHeader::tFileHeader( )
	{
		fZeroOut( this );
	}

	tFilePackageFile::tFileHeader::tFileHeader( tNoOpTag )
	{
	}

	void tFilePackageFile::fQueryFilePaths(
		tFilePathPtrList& output,
		const tFilePathPtr& folder,
		const tFilePathPtrList& extList ) const
	{
		for( u32 i = 0; i < mFileHeaders.fCount( ); ++i )
		{
			if( !strstr( mFileHeaders[ i ].mFileName.fBegin( ), folder.fCStr( ) ) )
				continue;

			// check for valid extensions (if no extensions passed, we take all files)
			u32 iext = 0;
			for( ; iext < extList.fCount( ); ++iext )
			{
				if( StringUtil::fCheckExtension( mFileHeaders[ i ].mFileName.fBegin( ), extList[ iext ].fCStr( ) ) )
					break;
			}
			if( iext == extList.fCount( ) && extList.fCount( ) > 0 )
				continue;

			output.fPushBack( tFilePathPtr( mFileHeaders[ i ].mFileName.fBegin( ) ) );
		}
	}

	u32 tFilePackageFile::fFindFile( const tFilePathPtr& path ) const
	{
		switch( mVersion )
		{
		case 0:		return fFindFilev0( path );
		case 1:		return fFindFilev1( path );
		default:	sig_nodefault( ); return ~0u;
		}
	}

	u32 tFilePackageFile::fFindFilev0( const tFilePathPtr& path ) const
	{
		// mFileNameHash did not exist in cVersion==0, no smart sorting was done, so we naive O(n) loop FFFFFFFF
		for( u32 i = 0; i < mFileHeaders.fCount( ); ++i )
			if( strcmp( path.fCStr( ), mFileHeaders[ i ].mFileName.fBegin( ) ) == 0 )
				return i;

		// We seriously just did an O(n) loop through all of mFileHeaders to find out we don't have that file.
		return ~0;
	}

	u32 tFilePackageFile::fFindFilev1( const tFilePathPtr& path ) const
	{
		const u32 searchHash = path.fGetStableHashValue( );
		const u32 headers = mFileHeaders.fCount( );

		sigcheckfail( headers > 0, return ~0u ); // 0 files in the package is almost certainly a build failure.

		// minI/maxI are inclusive.
		u32 minI = 0;
		u32 maxI = headers - 1;

		while( minI <= maxI )
		{
			// We're doing a binary search.  Verify our sort condition holds.
			sigcheckfail( mFileHeaders[ minI ].mFileNameStableHashValue <= mFileHeaders[ maxI ].mFileNameStableHashValue, return fFindFilev0( path ) );

			const u32 midI = ( minI + maxI ) / 2;
			const u32 midHash = mFileHeaders[ midI ].mFileNameStableHashValue;
			if( searchHash < midHash )
			{
				if( midI == 0 )
					return ~0u; // avoid underflow if minI = midI = 0, maxI = 1
				else
					maxI = midI - 1;
			}
			else if( searchHash > midHash )
			{
				minI = midI + 1; // no overflow danger as midI calc will round down
			}
			else
			{
				sigassert( searchHash == midHash );

				// Multiple headers could have the same hash, resort to linear search around this index for simplicity.
				minI = midI;
				while( minI > 0 && searchHash == mFileHeaders[ minI-1 ].mFileNameStableHashValue )
					--minI;

				for( u32 i=minI; i <= maxI && mFileHeaders[ i ].mFileNameStableHashValue == searchHash; ++i )
					if( strcmp( path.fCStr( ), mFileHeaders[ i ].mFileName.fBegin( ) ) == 0 )
						return i;

				return ~0u;
			}
		}

		return ~0u;
	}
}

