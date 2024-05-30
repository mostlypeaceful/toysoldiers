#include "BasePch.hpp"
#include "tSkeletonFile.hpp"

namespace Sig
{
	///
	/// \section tBone
	///

	tBone::tBone( )
		: mMasterIndex( -1 )
		, mName( 0 )
		, mParent( -1 )
	{
	}

	tBone::tBone( tNoOpTag )
		: mRefPose( cNoOpTag )
		, mRefPoseInv( cNoOpTag )
		, mChildren( cNoOpTag )
	{
	}

	///
	/// \section tSkeletonMap
	///

	tSkeletonMap::tSkeletonMap( )
		: mSrcSkeletonPath( 0 )
	{ }

	tSkeletonMap::tSkeletonMap( tNoOpTag )
		: mSrc2TgtBones( cNoOpTag )
		, mSrc2TgtXforms( cNoOpTag )
	{
	}

	///
	/// \section tSkeletonFile
	///

	define_lip_version( tSkeletonFile, 1, 1, 1 );

	const char*	tSkeletonFile::fGetFileExtension( )
	{
		return ".sklb";
	}

	tSkeletonFile::tSkeletonFile( )
		: mRoot( -1 )
	{
	}

	tSkeletonFile::tSkeletonFile( tNoOpTag )
		: tLoadInPlaceFileBase( cNoOpTag )
		, mReferenceFrame( cNoOpTag )
		, mMasterBoneList( cNoOpTag )
		, mBonesByName( cNoOpTag )
		, mPreOrderTraversal( cNoOpTag )
		, mSkeletonMaps( cNoOpTag )
	{
	}

	tSkeletonFile::~tSkeletonFile( )
	{
	}

	//------------------------------------------------------------------------------
	const tSkeletonMap* tSkeletonFile::fFindSkeletonMap( const tStringPtr & skeletonPath ) const
	{
		// Note: if this list starts to get long, sorting and bsearching may be required
		const u32 count = mSkeletonMaps.fCount( );
		for( u32 m = 0; m < count; ++m )
		{
			const tSkeletonMap & map = mSkeletonMaps[ m ];
			if( map.mSrcSkeletonPath->fGetStringPtr( ) == skeletonPath )
				return &map;
		}

		return 0;
	}

	void tSkeletonFile::fOnFileLoaded( const tResource& ownerResource )
	{
		// construct bone map with twice as many entries as
		// there are bones, just to maintain a fairly sparse harsh table
		mBonesByName.fTreatAsObject( ).fSetCapacity( 2 * mMasterBoneList.fCount( ) + 1 );

		// initialize the bone map (i.e., insert all bones, recursively)
		tBoneMap& boneMap = mBonesByName.fTreatAsObject( );
		for( u32 i = 0; i < mMasterBoneList.fCount( ); ++i )
			boneMap.fInsert( mMasterBoneList[ i ].mName->fGetStringPtr( ).fGetHashValue( ), &mMasterBoneList[ i ] );
	}

	void tSkeletonFile::fOnFileUnloading( const tResource& ownerResource )
	{
		mBonesByName.fDestroy( );
	}

}
