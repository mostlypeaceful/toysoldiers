#include "BasePch.hpp"
#include "tSkinMap.hpp"
#include "tMesh.hpp"
#include "tSkeletonFile.hpp"
//#include "tSceneRefEntity.hpp" //only for debugging

namespace Sig
{
	tSkinMap::tSkinMap( Anim::tAnimatedSkeleton& animatedSkel, const tMesh* mesh )
		: mAnimatedSkeleton( &animatedSkel )
		, mValid( true )
		, mInOrder( true )
	{
		sigassert( mAnimatedSkeleton && mAnimatedSkeleton->fSkeletonResource( ) );
		sigassert( mesh && mesh->mSkin );

		const tSkeletonFile* skelFile = mAnimatedSkeleton->fSkeletonResource( )->fCast< tSkeletonFile >( );
		sigassert( skelFile );

#ifdef  sig_logging
		tStringPtr unfoundBone;
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
				unfoundBone = searchFor;
#endif//sig_logging
				break;
			}
		}

#ifdef  sig_logging
		if( !mValid )
		{
			/*
				This may happen and is not necessarily a bug. It would be best to check the validity after everything has finished spawning.
				What can happen is first a turret will propagate the skeleton to all its children.
				 This makes invalid skin maps on the crew-members.
				Then the crew are initialized and their skeletons and skins are updated. In most cases fixing the invalidness here.
			*/
			//tSceneRefEntity* sceneEnt = mAnimatedSkeleton->fOwner( )->fDynamicCast<tSceneRefEntity>( );
			//log_warning( "Invalid skin/skeleton pairing; skeleton file [" << mAnimatedSkeleton->fSkeletonResource( )->fGetPath( ) << "] sigml: [" << (sceneEnt ? sceneEnt->fSgResource( )->fGetPath( ).fCStr( ) : "") << "]. [" << unfoundBone << "] found in skin, but not found in the skeleton." );
		}
#endif//sig_logging

		for(u32 i = 0; i < mMasterBoneIndices.fCount( ); ++i)
		{
			if( mMasterBoneIndices[i] != i )
			{
				mInOrder = false;
				break;
			}
		}
	}

	tSkinMap::tSkinMap( const tMesh* mesh )
		: mValid( false )
		, mInOrder( false )
	{
		sigassert( mesh && mesh->mSkin );
		mMasterBoneIndices.fNewArray( mesh->mSkin->mInfluences.fCount( ) );
	}

	void tSkinMap::fFillScratchMatrixPalette( tScratchMatrixPalette& matPaletteOut, u32& numBonesOut ) const
	{
		numBonesOut = mMasterBoneIndices.fCount( );

		if( mValid )
		{
			const Anim::tAnimatedSkeleton::tMatrixPalette& matPaletteSrc = mAnimatedSkeleton->fMatrixPalette( );
			for( u32 i = 0; i < mMasterBoneIndices.fCount( ); ++i )
				matPaletteOut[ i ] = matPaletteSrc[ mMasterBoneIndices[ i ] ];
		}
		else
		{
			for( u32 i = 0; i < mMasterBoneIndices.fCount( ); ++i )
				matPaletteOut[ i ] = Math::tMat3f::cIdentity;
		}
	}

	Math::tMat3f* tSkinMap::fBegin( ) const
	{
		return mAnimatedSkeleton->fMatrixPalette( ).fBegin( );
	}

	u32 tSkinMap::fCount( ) const
	{
		return mAnimatedSkeleton->fMatrixPalette( ).fCount( );
	}

}

