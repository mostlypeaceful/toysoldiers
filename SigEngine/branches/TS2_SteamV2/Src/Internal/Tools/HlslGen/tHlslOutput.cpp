#include "ToolsPch.hpp"
#include "tHlslOutput.hpp"
#include "tFileWriter.hpp"

namespace Sig { namespace HlslGen
{

	void tShaderOutputBase::fAddMaterialGlue( const Gfx::tShadeMaterialGluePtr& glue )
	{
		Gfx::tShadeMaterialGluePtrArray& glueArray = glue->fIsShared( ) ? mGlueShared : mGlueInstance;
		u32 entryWithSameType = glue->fIsUnique( ) ? 0 : glueArray.fCount( );
		for( ; entryWithSameType < glueArray.fCount( ); ++entryWithSameType )
			if( glueArray[ entryWithSameType ]->fClassId( ) == glue->fClassId( ) )
				break;
		if( entryWithSameType < glueArray.fCount( ) )
			glueArray[ entryWithSameType ].fReset( glue->fClone( ) );
		else
			glueArray.fPushBack( Gfx::tShadeMaterialGluePtr( glue->fClone( ) ) );
	}

	void tShaderOutputBase::fWriteToFile( const tFilePathPtr& folder ) const
	{
		sigassert( !folder.fNull( ) );
		tFileWriter o( tFilePathPtr::fConstructPath( folder, tFilePathPtr( mName ) ) );
		o( mHlsl.c_str( ), ( u32 )mHlsl.length( ) );

		if( mErrors.length( ) > 0 )
		{
			std::stringstream ss;
			ss << std::endl << std::endl;
			ss << "/*" << std::endl << std::endl;
			ss << "!Compilation Errors!" << std::endl;
			ss << mErrors << std::endl;
			ss << "*/" << std::endl;
			const std::string errorsText = ss.str( );
			o( errorsText.c_str( ), ( u32 )errorsText.length( ) );
		}
		else
		{
			std::stringstream ss;
			ss << std::endl << std::endl;
			ss << "/*" << std::endl;
			ss << "Shader compiled successfully..." << std::endl;
			ss << "*/" << std::endl;
			const std::string successText = ss.str( );
			o( successText.c_str( ), ( u32 )successText.length( ) );
		}
	}

	tPlatformId tHlslInput::fRealPlatformId( ) const
	{
		switch( mPid )
		{
		case cPidDefault:
		case cPidPcDx9:
			return cPlatformPcDx9;
		case cPidPcDx10:
			return cPlatformPcDx10;
		case cPidXbox360:
			return cPlatformXbox360;
		case cPidPs3:
			return cPlatformPs3Ppu;
		}

		return cPlatformPcDx9;
	}

	tHlslOutput::tHlslOutput( u32 numColorPShaders )
		: mColorPShaders( numColorPShaders )
	{
		const std::string modeNames[]= { "color", "depthonly", "depthalpha" };
		sig_static_assert( array_length( modeNames ) == cWriteModeCount );

		for( u32 i = 0; i < cWriteModeCount; ++i )
			mStaticVShaders[ i ].mName = modeNames[ i ] + "_static.vsh";
		for( u32 i = 0; i < cWriteModeCount; ++i )
			mSkinnedVShaders[ i ].mName = modeNames[ i ] + "_skin.vsh";
		for( u32 i = 0; i < mColorPShaders.fCount( ); ++i )
		{
			std::stringstream fileName;
			fileName << "color_" << i << "lights.psh";
			mColorPShaders[ i ].mName = fileName.str( );
		}
		{
			mDepthOnlyPShader.mName = "depthonly.psh";
		}
		{
			mDepthWithAlphaPShader.mName = "depthalpha.psh";
		}
	}

	void tHlslOutput::fWriteShadersToFile( const tFilePathPtr& inFolder ) const
	{
		const tFilePathPtr folder = inFolder.fNull( ) ? tFilePathPtr::fConstructPath( ToolsPaths::fGetEngineTempFolder( ), tFilePathPtr( "hlslgen" ) ) : inFolder;

		for( u32 i = 0; i < cWriteModeCount; ++i )
			mStaticVShaders[ i ].fWriteToFile( folder );
		for( u32 i = 0; i < cWriteModeCount; ++i )
			mSkinnedVShaders[ i ].fWriteToFile( folder );
		for( u32 i = 0; i < mColorPShaders.fCount( ); ++i )
			mColorPShaders[ i ].fWriteToFile( folder );
		mDepthOnlyPShader.fWriteToFile( folder );
		mDepthWithAlphaPShader.fWriteToFile( folder );
	}
			
	b32 tHlslOutput::fCreateDx9Shaders( const Gfx::tDevicePtr& device )
	{
		b32 error = false;
		for( u32 i = 0; i < cWriteModeCount; ++i )
		{
			if( mStaticVShaders[ i ].mHlsl.length( ) > 0 )
			{
				mStaticVShaders[ i ].mSh = Dx9Util::fCreateVertexShader( device, mStaticVShaders[ i ].mHlsl, 0, &mStaticVShaders[ i ].mErrors );
				error = error || mStaticVShaders[ i ].mSh.fNull( );
				if( mStaticVShaders[ i ].mSh )
					mStaticVShaders[ i ].mVtxFormat->fAllocateInPlace( device );
			}
		}
		for( u32 i = 0; i < cWriteModeCount; ++i )
		{
			if( mSkinnedVShaders[ i ].mHlsl.length( ) > 0 )
			{
				mSkinnedVShaders[ i ].mSh = Dx9Util::fCreateVertexShader( device, mSkinnedVShaders[ i ].mHlsl, 0, &mSkinnedVShaders[ i ].mErrors );
				error = error || mSkinnedVShaders[ i ].mSh.fNull( );
				if( mSkinnedVShaders[ i ].mSh )
					mSkinnedVShaders[ i ].mVtxFormat->fAllocateInPlace( device );
			}
		}
		for( u32 i = 0; i < mColorPShaders.fCount( ); ++i )
		{
			if( mColorPShaders[ i ].mHlsl.length( ) > 0 )
			{
				mColorPShaders[ i ].mSh = Dx9Util::fCreatePixelShader( device, mColorPShaders[ i ].mHlsl, 0, &mColorPShaders[ i ].mErrors );
				error = error || mColorPShaders[ i ].mSh.fNull( );
			}
		}
		if( mDepthOnlyPShader.mHlsl.length( ) > 0 )
		{
			mDepthOnlyPShader.mSh = Dx9Util::fCreatePixelShader( device, mDepthOnlyPShader.mHlsl, 0, &mDepthOnlyPShader.mErrors );
			error = error || mDepthOnlyPShader.mSh.fNull( );
		}
		if( mDepthWithAlphaPShader.mHlsl.length( ) > 0 )
		{
			mDepthWithAlphaPShader.mSh = Dx9Util::fCreatePixelShader( device, mDepthWithAlphaPShader.mHlsl, 0, &mDepthWithAlphaPShader.mErrors );
			error = error || mDepthWithAlphaPShader.mSh.fNull( );
		}

		return !error;
	}

}}

