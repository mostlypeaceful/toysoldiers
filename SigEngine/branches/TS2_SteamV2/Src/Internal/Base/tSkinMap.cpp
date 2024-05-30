#include "BasePch.hpp"
#include "tSkinMap.hpp"
#include "tMesh.hpp"
#include "tSkeletonFile.hpp"

namespace Sig
{
	tSkinMap::tSkinMap( tAnimatedSkeleton& animatedSkel, const tMesh* mesh )
		: mAnimatedSkeleton( &animatedSkel )
		, mValid( true )
	{
		sigassert( mAnimatedSkeleton && mAnimatedSkeleton->fSkeletonResource( ) );
		sigassert( mesh && mesh->mSkin );

		const tSkeletonFile* skelFile = mAnimatedSkeleton->fSkeletonResource( )->fCast< tSkeletonFile >( );
		sigassert( skelFile );

#ifdef  sig_logging
		tGrowableArray<tStringPtr> unfoundBones;
#endif//sig_logging

		mMasterBoneIndices.fNewArray( mesh->mSkin->mInfluences.fCount( ) );
		for( u32 i = 0; i < mMasterBoneIndices.fCount( ); ++i )
		{
			const tStringPtr& searchFor = mesh->mSkin->mInfluences[ i ].mName->fGetStringPtr( );
			const tBone* find = skelFile->fFindBone( searchFor );
			if( find )
				mMasterBoneIndices[ i ] = find->mMasterIndex;
			else
			{
				mValid = false;
#ifdef  sig_logging
				unfoundBones.fPushBack( searchFor );
#endif//sig_logging
				break;
			}
		}

#ifdef  sig_logging
		if( !mValid )
		{
			//log_warning( Log::cFlagAnimation, "Invalid skin/skeleton pairing; skeleton file [" << mAnimatedSkeleton->fSkeletonResource( )->fGetPath( ) << "]. The following bones are in the skin, but not found in the skeleton:" );
			//for( u32 i = 0; i < unfoundBones.fCount( ); ++i )
			//	log_line( 0, "\t" << unfoundBones[ i ] );
		}
#endif//sig_logging
	}

	tSkinMap::tSkinMap( const tMesh* mesh )
		: mValid( false )
	{
		sigassert( mesh && mesh->mSkin );
		mMasterBoneIndices.fNewArray( mesh->mSkin->mInfluences.fCount( ) );
	}

	void tSkinMap::fFillScratchMatrixPalette( tScratchMatrixPalette& matPaletteOut, u32& numBonesOut ) const
	{
		numBonesOut = mMasterBoneIndices.fCount( );

		if( mValid )
		{
			const tAnimatedSkeleton::tMatrixPalette& matPaletteSrc = mAnimatedSkeleton->fMatrixPalette( );
			for( u32 i = 0; i < mMasterBoneIndices.fCount( ); ++i )
				matPaletteOut[ i ] = matPaletteSrc[ mMasterBoneIndices[ i ] ];
		}
		else
		{
			for( u32 i = 0; i < mMasterBoneIndices.fCount( ); ++i )
				matPaletteOut[ i ] = Math::tMat3f::cIdentity;
		}
	}

}

