//------------------------------------------------------------------------------
// \file tSklmlConverter.cpp - 11 Jul 2008
// \author mwagner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#include "ToolsPch.hpp"
#include "tSklmlConverter.hpp"
#include "tFileWriter.hpp"
#include "tLoadInPlaceSerializer.hpp"

namespace Sig
{
	tSklmlConverter::tSklmlConverter( )
	{
	}

	tSklmlConverter::~tSklmlConverter( )
	{
	}

	b32 tSklmlConverter::fLoadSklmlFile( const tFilePathPtr& sklmlFilePath, const tFilePathPtr& outputResourcePath )
	{
		if( mExportFile.fLoadXml( sklmlFilePath ) )
		{
			mResourcePath = outputResourcePath;
			return true;
		}

		return false;
	}

	b32 tSklmlConverter::fConvertPlatformCommon( )
	{
		// Copy the reference frame
		fCopyBone( mReferenceFrame, mExportFile.mSkeleton.mRefFrame, 0);

		mMasterBoneList.fNewArray( mExportFile.mSkeleton.mTotalBoneCount );
		mRoot = mExportFile.mSkeleton.mRoot.mMasterIndex;

		// Convert the bones
		fConvertBone( 
			mMasterBoneList[ mExportFile.mSkeleton.mRoot.mMasterIndex ], 
			mExportFile.mSkeleton.mRoot, 
			0);

		// Build the skeleton maps
		fBuildSkeletonMaps( );
		return true;
	}

	b32 tSklmlConverter::fConvertPlatformSpecific( tPlatformId pid )
	{
		return true;
	}

	b32 tSklmlConverter::fOutput( tFileWriter& ofile, tPlatformId pid )
	{
		sigassert( ofile.fIsOpen( ) );

		// set the binary file signature
		this->fSetSignature<tSkeletonFile>( pid );

		// output
		tLoadInPlaceSerializer ser;
		ser.fSave( static_cast<tSkeletonFile&>( *this ), ofile, pid );

		return true;
	}

	//------------------------------------------------------------------------------
	void tSklmlConverter::fConvertBone( 
		tBone& obone, 
		const Sklml::tBone& ibone, 
		const tBone * parent )
	{
		fCopyBone(obone, ibone, parent);

		if( obone.mParent >= 0 )
			mPreOrderTraversal.fPushBack( ibone.mMasterIndex );

		obone.mChildren.fNewArray( ibone.mChildren.fCount( ) );
		for( u32 i = 0; i < ibone.mChildren.fCount( ); ++i )
		{
			tBone& childBone = mMasterBoneList[ ibone.mChildren[ i ]->mMasterIndex ];
			childBone.mParent = ibone.mMasterIndex;
			obone.mChildren[ i ] = ibone.mChildren[ i ]->mMasterIndex;
			fConvertBone( childBone, *ibone.mChildren[ i ], &obone );
		}
	}

	//------------------------------------------------------------------------------
	void tSklmlConverter::fCopyBone(
		tBone & obone, 
		const Sklml::tBone & ibone, 
		const tBone * parent)
	{
		obone.mMasterIndex	= ibone.mMasterIndex;
		obone.mName			= fAddLoadInPlaceStringPtr( ibone.mName.c_str( ) );
		obone.mRefPose		= ibone.mXform;
		obone.mRefPoseInv	= ibone.mXform.fInverse( );
		obone.mRefLocalPose = Math::tPRSXformf( 
			parent ? parent->mRefPoseInv * ibone.mXform : ibone.mXform );
	}

	//------------------------------------------------------------------------------
	void tSklmlConverter::fBuildSkeletonMaps( )
	{
		// Load the files
		tHashTable< tHashTablePtrInt, Sklml::tFile * > files;
		u32 loadErrors = fLoadSkeletonMapFiles( mExportFile, files );

		// If we have errors add a little context to the file failure messages
		if( loadErrors )
		{
			log_warning( "Couldn't build " 
				<< loadErrors 
				<< " skeleton maps. See above for .sklml load errors" );
		}

		// Resize the map array
		u32 mapIndex = 0;
		mSkeletonMaps.fNewArray( files.fGetItemCount( ) - loadErrors );

		const Sklml::tSkeleton & tgtSkel = mExportFile.mSkeleton;

		// Iterate over our files
		tHashTable< tHashTablePtrInt, Sklml::tFile *>::tIterator fileItr = files.fBegin( );
		tHashTable< tHashTablePtrInt, Sklml::tFile *>::tIterator fileEnd = files.fEnd( );
		for (; fileItr != fileEnd; ++fileItr )
		{
			if ( fileItr->fNullOrRemoved( ) )
				continue;

			Sklml::tFile * file = fileItr->mValue;

			// Could have been a file load error
			if( !file )
				continue;

			const Sklml::tSkeleton & srcSkel = file->mSkeleton;
			tSkeletonMap & map = mSkeletonMaps[ mapIndex++ ];

			map.mSrcSkeletonPath = fAddLoadInPlaceStringPtr( 
				tSkeletonFile::fConvertToBinary(file->mFilePath).fCStr( ) );
			map.mSrc2TgtBones.fNewArray( srcSkel.mTotalBoneCount );
			fBuildSkeletonMap( map, tgtSkel, srcSkel.mRoot, 0 );

			// Delete the file and zero its iterator
			delete file;
			fileItr->mValue = 0;
		}
	}

	//------------------------------------------------------------------------------
	u32 tSklmlConverter::fLoadSkeletonMapFiles( 
		const Sklml::tFile & srcFile,
		tHashTable< tHashTablePtrInt, Sklml::tFile * > & skels)
	{
		u32 errors = 0;

		const u32 count = srcFile.mAnimSourceSkeletons.fCount( );
		for( u32 s = 0; s < count; ++s )
		{
			tFilePathPtr path = srcFile.mAnimSourceSkeletons[ s ];
			tHashTablePtrInt pathHash = path.fGetHashValue( );

			// No cyclic redundancies
			if ( path == mExportFile.mFilePath )
				continue;

			// Already loaded
			if ( skels.fFind( pathHash ) )
				continue;

			Sklml::tFile * file = new Sklml::tFile( );

			// Try to load the file
			if( !file->fLoadXml( path ) )
			{
				// Failure to load, increment error count, delete and zero ptr
				++errors;

				delete file; 
				file = 0;
			}

			// Add the ptr whether there's an error or not ensuring 
			// that we don't try to load erroneous files multiple times
			skels.fInsert(pathHash, file);

			// If no file continue
			if( !file )
				continue;

			// Load the childs anim source skeletons
			errors += fLoadSkeletonMapFiles( *file, skels );
		}

		return errors;
	}

	//------------------------------------------------------------------------------
	const Sklml::tBone* tSklmlConverter::fFindBone(
		const std::string & name, 
		const Sklml::tBone & root, 
		const Sklml::tBone *& parent )
	{
		// Is the root our target?
		if( root.mName == name)
			return &root;

		// Try the children
		const u32 childCount = root.mChildren.fCount( );
		for( u32 b = 0; b < childCount; ++b )
		{
			const Sklml::tBone * bone = fFindBone( name, *root.mChildren[b], parent );
			if( bone )
			{
				// Set the parent if it hasn't been set - the check prevents
				// the recursive behavior from overwriting with the wrong parent
				if( !parent )
					parent = &root;

				return bone;
			}
		}

		// The bone does not exist
		return 0;
	}

	//------------------------------------------------------------------------------
	void tSklmlConverter::fBuildSkeletonMap( 
		tSkeletonMap & map, 
		const Sklml::tSkeleton & target, 
		const Sklml::tBone & srcBone, 
		const Sklml::tBone * srcParent )
	{
		tSkeletonMap::tBoneMap & boneMap = map.mSrc2TgtBones[ srcBone.mMasterIndex ];

		const Sklml::tBone * tgtParent = 0;
		const Sklml::tBone * tgtBone = fFindBone( srcBone.mName, target.mRoot, tgtParent );
		
		// If it exists in the target then process
		if( tgtBone )
		{
			// Set the bone re-index
			boneMap.mTargetBoneIndex = tgtBone->mMasterIndex;

			// Put the bones into local space
			Math::tMat3f tgtLocal = tgtParent ? tgtParent->mXform.fInverse( ) * tgtBone->mXform : tgtBone->mXform;
			Math::tMat3f srcLocal = srcParent ? srcParent->mXform.fInverse( ) * srcBone.mXform : srcBone.mXform;

			Math::tPRSXformf xform = Math::tPRSXformf(tgtLocal) - Math::tPRSXformf(srcLocal);

			// If it's not the identity store it in the transform list
			if( !xform.fEqual( Math::tPRSXformf::cZeroXform ) )
			{
				boneMap.mSrc2TgtXformIndex = map.mSrc2TgtXforms.fCount( );
				map.mSrc2TgtXforms.fPushBack( xform );
			}
		}

		// Now process the children
		const u32 childCount = srcBone.mChildren.fCount( );
		for( u32 b = 0; b < childCount; ++b )
			fBuildSkeletonMap( map, target, *srcBone.mChildren[ b ], &srcBone );
	}
}
