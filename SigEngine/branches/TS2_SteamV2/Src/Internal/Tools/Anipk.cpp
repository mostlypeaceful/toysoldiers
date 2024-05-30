#include "ToolsPch.hpp"
#include "Anipk.hpp"
#include "Animl.hpp"
#include "tAnimPackFile.hpp"
#include "tXmlSerializer.hpp"
#include "tXmlDeserializer.hpp"
#include "FileSystem.hpp"

namespace Sig { namespace Anipk
{
	const char* fGetFileExtension( )
	{
		return ".anipk";
	}

	b32 fIsAnipkFile( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( ) );
	}
	
	tFilePathPtr fAnipkPathToAnib( const tFilePathPtr& path )
	{
		return tFilePathPtr::fSwapExtension( path, tAnimPackFile::fGetFileExtension( ) );
	}

	tFilePathPtr fAnibPathToAnipk( const tFilePathPtr& path )
	{
		return tFilePathPtr::fSwapExtension( path, fGetFileExtension( ) );
	}

	///
	/// \section tKeyFrameTag
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tKeyFrameTag& o )
	{
		s( "Tag", o.mTag );
		s( "TypeKey", o.mEventTypeKey );
		s( "Time", o.mTime );
	}

	///
	/// \section tAnimationMetaData
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tAnimationMetaData& o )
	{
		s( "Name", o.mName );
		s( "DisableCompression", o.mDisableCompression );
		s( "CompressionErrorP", o.mCompressionErrorP );
		s( "CompressionErrorR", o.mCompressionErrorR );
		s( "CompressionErrorS", o.mCompressionErrorS );
		s( "KeyFrameTags", o.mKeyFrameTags );
	}

	tAnimationMetaData::tAnimationMetaData( const std::string& name )
		: mName( name )
		, mDisableCompression( false )
		, mCompressionErrorP( 1.f/100.f )
		, mCompressionErrorR( 1.f/1000.f )
		, mCompressionErrorS( 1.f/100.f )
	{
	}

	///
	/// \section tFile
	///

	namespace
	{
		static u32 gAnipkVersion = 0;
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile& o )
	{
		s.fAsAttribute( "Version", o.mVersion );

		if( o.mVersion != gAnipkVersion )
		{
			log_warning( 0, "Anipk file format is out of date -> Please re-export." );
			return;
		}

		s( "SkeletonRef", o.mSkeletonRef );
		s( "AnimMetaData", o.mAnimMetaData );
	}

	tFile::tFile( )
		: mVersion( gAnipkVersion )
	{
	}

	b32 tFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Anipk", *this, promptToCheckout ) )
		{
			log_warning( 0, "Couldn't save Anipk file [" << path << "]" );
			return false;
		}

		return true;
	}

	b32 tFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Anipk", *this ) )
		{
			log_warning( 0, "Couldn't load Anipk file [" << path << "]" );
			return false;
		}

		return true;
	}

	tAnimationMetaData& tFile::fFindOrAddAnimMetaData( const std::string& animName )
	{
		for( u32 i = 0; i < mAnimMetaData.fCount( ); ++i )
		{
			if( _stricmp( animName.c_str( ), mAnimMetaData[ i ].mName.c_str( ) ) == 0 )
				return mAnimMetaData[ i ];
		}
		mAnimMetaData.fPushBack( tAnimationMetaData( animName ) );
		return mAnimMetaData.fBack( );
	}

	const tAnimationMetaData* tFile::fFindAnimMetaData( const std::string& animName ) const
	{
		for( u32 i = 0; i < mAnimMetaData.fCount( ); ++i )
		{
			if( _stricmp( animName.c_str( ), mAnimMetaData[ i ].mName.c_str( ) ) == 0 )
				return &mAnimMetaData[ i ];
		}
		return 0;
	}

	void tFile::fAddAssetGenInputOutput( iAssetGenPlugin::tInputOutputList& inputOutputsOut, const tFilePathPtr& anipkPath )
	{
		inputOutputsOut.fGrowCount( 1 );
		inputOutputsOut.fBack( ).mOriginalInput = anipkPath;
		inputOutputsOut.fBack( ).mOutput = ToolsPaths::fMakeResRelative( Anipk::fAnipkPathToAnib( anipkPath ) );

		const tFilePathPtr curDir = tFilePathPtr( StringUtil::fDirectoryFromPath( anipkPath.fCStr( ) ) );

		tFilePathPtrList immediateInputFiles;
		FileSystem::fGetFileNamesInFolder( immediateInputFiles, curDir, true );

		for( u32 i = 0; i < immediateInputFiles.fCount( ); ++i )
		{
			if( Animl::fIsAnimlFile( immediateInputFiles[ i ] ) )
				inputOutputsOut.fBack( ).mInputs.fPushBack( immediateInputFiles[i] );
		}

		inputOutputsOut.fBack( ).mInputs.fPushBack( ToolsPaths::fMakeResAbsolute( mSkeletonRef ) );
		inputOutputsOut.fBack( ).mInputs.fPushBack( anipkPath );
	}

}}

