#ifndef __Anipk__
#define __Anipk__
#include "iAssetGenPlugin.hpp"

namespace Sig { namespace Anipk
{
	tools_export const char* fGetFileExtension( );
	tools_export b32 fIsAnipkFile( const tFilePathPtr& path );
	tools_export tFilePathPtr fAnipkPathToAnib( const tFilePathPtr& path );
	tools_export tFilePathPtr fAnibPathToAnipk( const tFilePathPtr& path );

	class tools_export tKeyFrameTag
	{
	public:
		std::string			mTag;
		f32					mTime;
		u32					mEventTypeKey;

	public:
		explicit tKeyFrameTag( )
			: mTag( "" ), mTime( 0.f ), mEventTypeKey( ~0 ) { }

		explicit tKeyFrameTag( const std::string& tag, f32 time, u32 type )
			: mTag( tag ), mTime( time ), mEventTypeKey( type ) { }
	};

	class tools_export tAnimationMetaData
	{
	public:
		std::string			mName;
		b32					mDisableCompression;
		f32					mCompressionErrorP;
		f32					mCompressionErrorR;
		f32					mCompressionErrorS;
		tGrowableArray<tKeyFrameTag> mKeyFrameTags;
		
		b32 fDirty( ) const { return mDirty; }
		void fSetDirty( ) { mDirty = true; }
		void fClearDirty( ) { mDirty = false; }

		void fSetTagTime( u32 idx, f32 newTime );

	public:
		explicit tAnimationMetaData( const std::string& name = "" );

	private:
		b32					mDirty; // Run-time only, not serialized.
	};

	class tools_export tFile
	{
	public:
		u32					mVersion;
		tFilePathPtr		mSkeletonRef;
		tGrowableArray<tAnimationMetaData> mAnimMetaData;

	public:
		tFile( );
		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout, b32 displayWarnings = false );
		b32 fLoadXml( const tFilePathPtr& path );
		tAnimationMetaData& fFindOrAddAnimMetaData( const std::string& animName );
		const tAnimationMetaData* fFindAnimMetaData( const std::string& animName ) const;
		void fAddAssetGenInputOutput( iAssetGenPlugin::tInputOutputList& inputOutputsOut, const tFilePathPtr& anipkPath );
	};

}}

#endif//__Anipk__
