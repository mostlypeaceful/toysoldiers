//------------------------------------------------------------------------------
// \file Sklml.hpp - 08 Jul 2008
// \author Max Wagner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __Sklml__
#define __Sklml__
#include "tStrongPtr.hpp"
#include "iAssetGenPlugin.hpp"

namespace Sig { namespace Sklml
{
	tools_export const char* fGetFileExtension( );
	tools_export b32 fIsSklmlFile( const tFilePathPtr& path );
	tools_export tFilePathPtr fSklmlPathToSklb( const tFilePathPtr& path );

	class tBone;
	typedef tRefCounterPtr< tBone > tBonePtr;
	typedef tGrowableArray< tBonePtr > tBoneArray;

	class tools_export tBone : public tRefCounter
	{
	public:
		u32				mMasterIndex;
		std::string		mName;
		Math::tMat3f	mXform;
		tBoneArray		mChildren;
	public:
		tBone( ) : mMasterIndex( ~0 ), mXform( Math::tMat3f::cIdentity ) { }
	};

	class tools_export tSkeleton : public tRefCounter
	{
	public:
		tBone		mRefFrame;
		tBone		mRoot;
		u32			mTotalBoneCount; // not counting reference frame
	public:
		tSkeleton( ) : mTotalBoneCount( 0 ) { }
	};

	typedef tRefCounterPtr< tSkeleton > tSkeletonPtr;

	///
	/// \class tFile
	/// \brief A skeleton convertible to/from xml
	class tools_export tFile
	{
	public:
		u32					mVersion;
		tStringPtr			mModellingPackage;
		tFilePathPtr		mSrcFile;
		tFilePathPtr		mFilePath;
		tSkeleton			mSkeleton;

		tGrowableArray<tFilePathPtr> mAnimSourceSkeletons;

	public:
		tFile( );
		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		b32 fLoadXml( const tFilePathPtr& path );
		void fGenerateMasterIndices( );
		const tBone* fFindParentOf( const char* childName ) const;
		u32 fMasterBoneIndex( const char* boneName ) const;
		u32 fParentMasterBoneIndex( const char* boneName ) const;
		void fQueryBoneNames( tGrowableArray<std::string>& boneNames ) const;
		void fAddAssetGenInputOutput( 
			iAssetGenPlugin::tInputOutputList& inputOutputsOut, 
			const tFilePathPtr& sklmlPath );

	private:
		void fGenerateMasterIndices( tBone& curBone, u32& curIndex );
		const tBone* fFindParentOf( const tBone& curParent, const char* childName ) const;
		u32 fMasterBoneIndex( const tBone& curBone, const char* boneName ) const;
		void fQueryBoneNames( const tBone& curBone, tGrowableArray<std::string>& boneNames ) const;

	};

}}

#endif//__Sklml__
