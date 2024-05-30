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

	const u32 tFilePackageFile::cVersion = 0;

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
		for( u32 i = 0; i < mFileHeaders.fCount( ); ++i )
		{
			if( strcmp( path.fCStr( ), mFileHeaders[ i ].mFileName.fBegin( ) ) == 0 )
				return i;
		}

		return ~0;
	}
}

