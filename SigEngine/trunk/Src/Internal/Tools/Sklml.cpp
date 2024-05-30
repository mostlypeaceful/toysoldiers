//------------------------------------------------------------------------------
// \file Sklml.hpp - 08 Jul 2008
// \author Max Wagner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#include "ToolsPch.hpp"
#include "Sklml.hpp"
#include "tSkeletonFile.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"

namespace Sig { namespace Sklml
{
	const char* fGetFileExtension( )
	{
		return ".sklml";
	}

	b32 fIsSklmlFile( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( ) );
	}

	tFilePathPtr fSklmlPathToSklb( const tFilePathPtr& path )
	{
		return tFilePathPtr::fSwapExtension( path, tSkeletonFile::fGetFileExtension( ) );
	}

	///
	/// \section tBone
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tBone& o )
	{
		s( "MasterIndex", o.mMasterIndex );
		s( "Name", o.mName );
		s( "Xform", o.mXform );
		s( "Children", o.mChildren );
	}


	///
	/// \section tSkeleton
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tSkeleton& o )
	{
		s( "TotalBoneCount", o.mTotalBoneCount );
		s( "RefFrame",	o.mRefFrame );
		s( "Root", o.mRoot );
	}

	///
	/// \section tFile
	///

	namespace
	{
		static u32 gSklmlVersion = 1;
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile& o )
	{
		s.fAsAttribute( "Version", o.mVersion );

		s( "ModellingPackage", o.mModellingPackage );
		s( "SrcFile", o.mSrcFile );

		if( o.mVersion != gSklmlVersion )
		{
			log_warning( "Sklml file format is out of date (exported from " << o.mSrcFile << ") -> Please re-export." );
			return;
		}

		s( "Skeleton", o.mSkeleton );
		s( "AnimSourceSkeletons", o.mAnimSourceSkeletons );
	}

	tFile::tFile( )
		: mVersion( gSklmlVersion )
	{
	}

	b32 tFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Sklml", *this, promptToCheckout ) )
		{
			log_warning( "Couldn't save Sklml file [" << path << "]" );
			return false;
		}

		return true;
	}

	b32 tFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Sklml", *this ) )
		{
			log_warning( "Couldn't load Sklml file [" << path << "]" );
			return false;
		}

		mFilePath = ToolsPaths::fMakeResRelative( path );
		return true;
	}

	void tFile::fGenerateMasterIndices( )
	{
		u32 curIndex = 0;
		fGenerateMasterIndices( mSkeleton.mRoot, curIndex );
		mSkeleton.mTotalBoneCount = curIndex;
	}

	const tBone* tFile::fFindParentOf( const char* childName ) const
	{
		return fFindParentOf( mSkeleton.mRoot, childName );
	}

	u32 tFile::fMasterBoneIndex( const char* boneName ) const
	{
		return fMasterBoneIndex( mSkeleton.mRoot, boneName );
	}

	u32 tFile::fParentMasterBoneIndex( const char* boneName ) const
	{
		const tBone *bone = fFindParentOf( boneName );
		if( !bone ) return ~0;

		return bone->mMasterIndex;
	}

	void tFile::fQueryBoneNames( tGrowableArray<std::string>& boneNames ) const
	{
		fQueryBoneNames( mSkeleton.mRoot, boneNames );
	}

	//------------------------------------------------------------------------------
	void tFile::fAddAssetGenInputOutput( 
		iAssetGenPlugin::tInputOutputList& inputOutputsOut, 
		const tFilePathPtr& sklmlPath )
	{
		inputOutputsOut.fGrowCount( 1 );
		inputOutputsOut.fBack( ).mOriginalInput = sklmlPath;
		
		inputOutputsOut.fBack( ).mInputs.fPushBack( sklmlPath );
		
		const u32 count = mAnimSourceSkeletons.fCount( );
		for( u32 s = 0; s < count; ++s )
			inputOutputsOut.fBack( ).mInputs.fPushBack( mAnimSourceSkeletons[ s ] );

		inputOutputsOut.fBack( ).mOutput = ToolsPaths::fMakeResRelative( Sklml::fSklmlPathToSklb( sklmlPath ) );
	}

	void tFile::fGenerateMasterIndices( tBone& curBone, u32& curIndex )
	{
		curBone.mMasterIndex = curIndex++;
		for( u32 i = 0; i < curBone.mChildren.fCount( ); ++i )
			fGenerateMasterIndices( *curBone.mChildren[ i ], curIndex );
	}

	const tBone* tFile::fFindParentOf( const tBone& curParent, const char* childName ) const
	{
		for( u32 i = 0; i < curParent.mChildren.fCount( ); ++i )
		{
			if( _stricmp( childName, curParent.mChildren[ i ]->mName.c_str( ) ) == 0 )
				return &curParent;
		}

		for( u32 i = 0; i < curParent.mChildren.fCount( ); ++i )
		{
			const tBone* o = fFindParentOf( *curParent.mChildren[ i ], childName );
			if( o )
				return o;
		}

		return 0;
	}

	u32 tFile::fMasterBoneIndex( const tBone& curBone, const char* boneName ) const
	{
		if( _stricmp( boneName, curBone.mName.c_str( ) ) == 0 )
			return curBone.mMasterIndex;

		for( u32 i = 0; i < curBone.mChildren.fCount( ); ++i )
		{
			const u32 o = fMasterBoneIndex( *curBone.mChildren[ i ], boneName );
			if( o != ~0 )
				return o;
		}

		return ~0;
	}

	void tFile::fQueryBoneNames( const tBone& curBone, tGrowableArray<std::string>& boneNames ) const
	{
		boneNames.fPushBack( curBone.mName );
		for( u32 i = 0; i < curBone.mChildren.fCount( ); ++i )
			fQueryBoneNames( *curBone.mChildren[ i ], boneNames );
	}

}}

