#include "ToolsPch.hpp"
#include "tAnimlConverter.hpp"
#include "tFileWriter.hpp"
#include "tSkeletonFile.hpp"
#include "tProjectFile.hpp"
#include "tLoadInPlaceSerializer.hpp"

namespace Sig
{
	tAnimlConverter::tAnimlConverter( )
	{
	}

	tAnimlConverter::~tAnimlConverter( )
	{
	}

	b32 tAnimlConverter::fLoadAnimlFiles( const tFilePathPtrList& animlFilePaths, const tFilePathPtr& outputResourcePath )
	{
		if( animlFilePaths.fCount( ) < 2 )
		{
			// we require at least two input files, the .anipk and one .sklml, otherwise the situation is invalid
			log_warning( 0, "Not enough input files to convert an .anipk file; requires one .anipk and one .sklml file." );
			return false;
		}

		mExportFiles.fSetCount( animlFilePaths.fCount( ) - 2 );
		b32 success = true;
		for( u32 i = 0; success && i < animlFilePaths.fCount( ); ++i )
		{
			if( Anipk::fIsAnipkFile( animlFilePaths[ i ] ) )
				success = mAnipkFile.fLoadXml( animlFilePaths[ i ] );
			else if( Animl::fIsAnimlFile( animlFilePaths[ i ] ) )
			{
				success = mExportFiles[ i ].fLoadXml( animlFilePaths[ i ] );
				if( success )
					mExportFiles[ i ].mSimpleName = StringUtil::fStripExtension( StringUtil::fNameFromPath( animlFilePaths[ i ].fCStr( ) ).c_str( ) );
			}
		}

		if( success && mSklmlFile.fLoadXml( ToolsPaths::fMakeResAbsolute( mAnipkFile.mSkeletonRef ) ) )
		{
			mResourcePath = outputResourcePath;
			return true;
		}

		mExportFiles.fDeleteArray( );
		return false;
	}

	b32 tAnimlConverter::fConvertPlatformCommon( )
	{
		const tFilePathPtr skelResPath = Sklml::fSklmlPathToSklb( mAnipkFile.mSkeletonRef );
		mSkeletonResource = fAddLoadInPlaceResourcePtr( tResourceId::fMake< tSkeletonFile >( skelResPath ) );

		mAnims.fNewArray( mExportFiles.fCount( ) );

		for( u32 i = 0; i < mExportFiles.fCount( ); ++i )
		{
			if( !fConvertAnim( mAnims[ i ], mExportFiles[ i ] ) )
				return false;
		}

		return true;
	}

	b32 tAnimlConverter::fConvertPlatformSpecific( tPlatformId pid )
	{
		return true;
	}

	b32 tAnimlConverter::fOutput( tFileWriter& ofile, tPlatformId pid )
	{
		sigassert( ofile.fIsOpen( ) );

		// set the binary file signature
		tBinaryFileBase::fSetSignature( pid, Rtti::fGetClassId<tAnimPackFile>( ), tAnimPackFile::cVersion );

		// output
		tLoadInPlaceSerializer ser;
		ser.fSave( static_cast<tAnimPackFile&>( *this ), ofile, pid );

		return true;
	}

	b32 tAnimlConverter::fConvertAnim( 
		tKeyFrameAnimation& oanim, 
		const tExportAnim& ianim )
	{
		log_line( 0, "   -> Converting animation [" << ianim.mSimpleName << "]" );

		Anipk::tAnimationMetaData animMetaData;
		const Anipk::tAnimationMetaData* findAnimMetaData = mAnipkFile.fFindAnimMetaData( ianim.mSimpleName );
		if( findAnimMetaData )
			animMetaData = *findAnimMetaData;

		const f32 pError = animMetaData.mDisableCompression ? 0.f : animMetaData.mCompressionErrorP;
		const f32 rError = animMetaData.mDisableCompression ? 0.f : animMetaData.mCompressionErrorR;
		const f32 sError = animMetaData.mDisableCompression ? 0.f : animMetaData.mCompressionErrorS;

		oanim.mPackFile = this;
		oanim.mName = fAddLoadInPlaceStringPtr( ianim.mSimpleName.c_str( ) );
		oanim.mFramesPerSecond = ianim.mFramesPerSecond;
		oanim.mLengthOneShot = ( f32 )ianim.mTotalFrames / ( f32 )ianim.mFramesPerSecond;
		oanim.mLengthLooping = fMax<f32>( 0.f, ianim.mTotalFrames - 1.f ) / ( f32 )ianim.mFramesPerSecond;

		// convert reference frame (as long as user didn't exclude it)
		if( !ianim.mSkeleton.mRefFrame.mExclude )
		{
			oanim.mFlags |= tKeyFrameAnimation::cFlagContainsRefFrame;

			Animl::tKeyFrameList newKeys;
			fConvertReferenceFrame( ianim, ianim.mSkeleton.mRefFrame, newKeys );

			Animl::tDeltaCompressionResult compressionResult;
			Animl::tBone::fDeltaCompress( compressionResult, newKeys, pError, rError, sError );

			if( !fConvertBone( oanim, oanim.mReferenceFrame, ianim.mSkeleton.mRefFrame, compressionResult, true ) )
				return false;
		}
		else
			oanim.mFlags |= tKeyFrameAnimation::cFlagPartial;

		// convert all bones (again, checking for exclusion first)
		for( u32 i = 0; i < ianim.mSkeleton.mBones.fCount( ); ++i )
		{
			const Animl::tBone& ibone = *ianim.mSkeleton.mBones[ i ];
			if( ibone.mExclude )
			{
				oanim.mFlags |= tKeyFrameAnimation::cFlagPartial;
				continue;
			}

			Animl::tKeyFrameList newKeys;
			if( !fConvertBoneToParentSpace( ianim, ibone, newKeys ) )
				return false;

			Animl::tDeltaCompressionResult compressionResult;
			Animl::tBone::fDeltaCompress( compressionResult, newKeys, pError, rError, sError );

			oanim.mBones.fPushBack( tKeyFrameAnimation::tBone( ) );
			if( !fConvertBone( oanim, oanim.mBones.fBack( ), *ianim.mSkeleton.mBones[ i ], compressionResult, false ) )
				return false;
		}

		// reorder bones in hierarchical order
		std::sort( oanim.mBones.fBegin( ), oanim.mBones.fEnd( ) );		

		// convert keyframe events
		oanim.mEvents.fNewArray( animMetaData.mKeyFrameTags.fCount( ) );
		for( u32 i = 0; i < animMetaData.mKeyFrameTags.fCount( ); ++i )
		{
			oanim.mEvents[ i ].mTime = animMetaData.mKeyFrameTags[ i ].mTime;
			oanim.mEvents[ i ].mTag = fAddLoadInPlaceStringPtr( animMetaData.mKeyFrameTags[ i ].mTag.c_str( ) );
			oanim.mEvents[ i ].mEventTypeCppValue = tProjectFile::fGetCurrentProjectFileCached( ).fFindKeyframeEventIndexByKey( animMetaData.mKeyFrameTags[ i ].mEventTypeKey );
		}

		return true;
	}

	b32 tAnimlConverter::fConvertBone( 
		tKeyFrameAnimation& oanim, 
		tKeyFrameAnimation::tBone& obone, 
		const Animl::tBone& ibone,
		const Animl::tDeltaCompressionResult& compressionResult,
		b32 refFrame )
	{
		obone.mMasterBoneIndex = mSklmlFile.fMasterBoneIndex( ibone.mName.c_str( ) );
		obone.mParentMasterBoneIndex = mSklmlFile.fParentMasterBoneIndex( ibone.mName.c_str( ) );

		if( !refFrame && obone.mMasterBoneIndex == ~0 )
		{
			log_warning( 0, "Bone [" << ibone.mName << "] not found in master skeleton file; aborting conversion of current animation." );
			return false;
		}

		obone.mName = fAddLoadInPlaceStringPtr( ibone.mName.c_str( ) );
		
		if( ibone.mAdditive )
		{
			obone.mFlags |= tKeyFrameAnimation::tBone::cFlagAdditive;
			oanim.mFlags |= tKeyFrameAnimation::cFlagPartial;
		}

		obone.mPMin = compressionResult.mPMin;
		obone.mPMax = compressionResult.mPMax;
		obone.mSMin = compressionResult.mSMin;
		obone.mSMax = compressionResult.mSMax;

		obone.mPositionFrameNums.fNewArray( compressionResult.mPKeyCount );
		obone.mPositionKeys.fNewArray( compressionResult.mPKeyCount );
		for( u32 i = 0, ikey = 0; i < compressionResult.mCompressionKeys.fCount( ); ++i )
		{
			if( compressionResult.mCompressionKeys[ i ].mP )
			{
				obone.mPositionFrameNums[ ikey ] = i;
				obone.mPositionKeys[ ikey ] = tKeyFrameAnimation::fToPositionKey( compressionResult.mPRSKeys[ i ].mP, compressionResult.mPMin, compressionResult.mPMax );
				++ikey;
			}
		}

		obone.mRotationFrameNums.fNewArray( compressionResult.mRKeyCount );
		obone.mRotationKeys.fNewArray( compressionResult.mRKeyCount );
		for( u32 i = 0, ikey = 0; i < compressionResult.mCompressionKeys.fCount( ); ++i )
		{
			if( compressionResult.mCompressionKeys[ i ].mR )
			{
				obone.mRotationFrameNums[ ikey ] = i;
				obone.mRotationKeys[ ikey ] = tKeyFrameAnimation::fToRotationKey( compressionResult.mPRSKeys[ i ].mR );
				++ikey;
			}
		}

		obone.mScaleFrameNums.fNewArray( compressionResult.mSKeyCount );
		obone.mScaleKeys.fNewArray( compressionResult.mSKeyCount );
		for( u32 i = 0, ikey = 0; i < compressionResult.mCompressionKeys.fCount( ); ++i )
		{
			if( compressionResult.mCompressionKeys[ i ].mS )
			{
				obone.mScaleFrameNums[ ikey ] = i;
				obone.mScaleKeys[ ikey ] = tKeyFrameAnimation::fToScaleKey( compressionResult.mPRSKeys[ i ].mS, compressionResult.mSMin, compressionResult.mSMax );
				++ikey;
			}
		}

		obone.mIKPriority = ibone.mIKPriority;
		obone.mIKAxisLimitsOrder = ibone.mIKRotLimitOrder;
		obone.mIKAxisLimits = ibone.mIKRotLimits;

		// rotation is swapped in maya
		Math::tVec3f maxTemp, minTemp;
		for( u32 i = 0; i < 3; ++i )
		{
			maxTemp[ i ] = -obone.mIKAxisLimits.mMin[ i ];
			minTemp[ i ] = -obone.mIKAxisLimits.mMax[ i ];
		}

		obone.mIKAxisLimits.mMin = minTemp;
		obone.mIKAxisLimits.mMax = maxTemp;

		return true;
	}

	void tAnimlConverter::fConvertReferenceFrame( const Animl::tFile& animl, const Animl::tBone& animlBone, Animl::tKeyFrameList& newKeysOut )
	{
		newKeysOut = animlBone.mKeyFrames;
	}

	b32 tAnimlConverter::fConvertBoneToParentSpace( const Animl::tFile& animl, const Animl::tBone& animlBone, Animl::tKeyFrameList& newKeysOut )
	{
		newKeysOut = animlBone.mKeyFrames;

		// find parent bone from Sklml file
		const Sklml::tBone* sklmlParent = mSklmlFile.fFindParentOf( animlBone.mName.c_str( ) );

		// now that we have the parent bone (and hence the name of the parent), we find the bone in the animl file;
		// note that if there is no parent, we root the bone to the skeleton's reference frame
		const Animl::tBone* animlParent = sklmlParent ? animl.fFindBone( sklmlParent->mName.c_str( ) ) : &animl.mSkeleton.mRefFrame;
		if( !animlParent )
		{
			log_warning( 0, "Couldn't find bone [" << sklmlParent->mName << "] in Animl file; parenting to reference frame." );
			animlParent = &animl.mSkeleton.mRefFrame;
			return false;
		}

		// now we can iterate through both sets of keys simulataneously, transforming the child into the parent space
		sigassert( newKeysOut.fCount( ) == animlParent->mKeyFrames.fCount( ) );
		for( u32 ikey = 0; ikey < newKeysOut.fCount( ); ++ikey )
			newKeysOut[ ikey ] = animlParent->mKeyFrames[ ikey ].fInverse( ) * newKeysOut[ ikey ];

		return true;
	}

}
