#include "ToolsPch.hpp"
#include "tMaterialPreviewBundle.hpp"
#include "tSceneGraph.hpp"
#include "Derml.hpp"
#include "tSpherePreviewGeometry.hpp"
#include "tPlanePreviewGeometry.hpp"
#include "HlslGen/tHlslGenTree.hpp"
#include "Gfx/tRenderContext.hpp"

namespace Sig { namespace Gfx
{
	class tools_export tShadeMaterialPreview : public tMaterial
	{
		implement_rtti_serializable_base_class( tShadeMaterialPreview, 0x19DFC707 );
	private:
		const HlslGen::tHlslOutput& mHlslGenOutput;
	public:
		tShadeMaterialPreview( const HlslGen::tHlslOutput& hlslGenOutput )
			: mHlslGenOutput( hlslGenOutput ) { }

		virtual const tVertexFormat& fVertexFormat( ) const
		{
			static const tVertexFormat cNullFormat;
			HlslGen::tVertexShaderOutput::tVertexFormatVRamPtr vtxFormat = mHlslGenOutput.mStaticVShaders[ HlslGen::cWriteModeColor ].mVtxFormat;
			return vtxFormat ? *vtxFormat : cNullFormat;
		}

		virtual b32 fIsLit( ) const { return mHlslGenOutput.mColorPShaders.fCount( ) > 1; }

		virtual b32 fRendersDepth( ) const { return false; }

		virtual void fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const
		{
			IDirect3DDevice9* d3ddev = device->fGetDevice( );
			const u32 pShaderIdx = fMin( mHlslGenOutput.mColorPShaders.fCount( ) - 1, context.mLightShaderConstants.mLightCount );
			d3ddev->SetVertexShader( mHlslGenOutput.mStaticVShaders[ HlslGen::cWriteModeColor ].mSh.fGetRawPtr( ) );
			d3ddev->SetPixelShader( mHlslGenOutput.mColorPShaders[ pShaderIdx ].mSh.fGetRawPtr( ) );

			const tShadeMaterialGluePtrArray& glueVs = mHlslGenOutput.mStaticVShaders[ HlslGen::cWriteModeColor ].mGlueShared;
			const tShadeMaterialGluePtrArray& gluePs = mHlslGenOutput.mColorPShaders[ pShaderIdx ].mGlueShared;
			for( u32 i = 0; i < glueVs.fCount( ); ++i )
				glueVs[ i ]->fApplyShared( *this, mHlslGenOutput.mMaterialGlueValues, device, context );
			for( u32 i = 0; i < gluePs.fCount( ); ++i )
				gluePs[ i ]->fApplyShared( *this, mHlslGenOutput.mMaterialGlueValues, device, context );
		}

		virtual void fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& instance ) const
		{
			const u32 pShaderIdx = fMin( mHlslGenOutput.mColorPShaders.fCount( ) - 1, context.mLightShaderConstants.mLightCount );
			const tShadeMaterialGluePtrArray& glueVs = mHlslGenOutput.mStaticVShaders[ HlslGen::cWriteModeColor ].mGlueInstance;
			const tShadeMaterialGluePtrArray& gluePs = mHlslGenOutput.mColorPShaders[ pShaderIdx ].mGlueInstance;
			for( u32 i = 0; i < glueVs.fCount( ); ++i )
				glueVs[ i ]->fApplyInstance( *this, mHlslGenOutput.mMaterialGlueValues, device, context, instance );
			for( u32 i = 0; i < gluePs.fCount( ); ++i )
				gluePs[ i ]->fApplyInstance( *this, mHlslGenOutput.mMaterialGlueValues, device, context, instance );

			if( renderBatch.mIndexBuffer )
				renderBatch.fRenderInstance( device );
		}
	};
}}


namespace Sig
{
	tMaterialPreviewGeometry::tMaterialPreviewGeometry( )
	{
	}

	void tMaterialPreviewGeometry::fSetYaw( f32 yawInRads )
	{
		const Math::tVec3f zAxis = Math::tQuatf( Math::tAxisAnglef( Math::tVec3f::cYAxis, yawInRads ) ).fRotate( Math::tVec3f::cZAxis );
		Math::tMat3f xform = Math::tMat3f::cIdentity;
		xform.fOrientZAxis( zAxis );
		fMoveTo( xform );
	}

	tMaterialPreviewBundle::tMaterialPreviewBundle( const Gfx::tDevicePtr& device, HlslGen::tVshStyle vshStyle ) 
		: mDevice( device )
		, mVshStyle( vshStyle )
	{
		if( mVshStyle == HlslGen::cVshFacingQuads )
		{
			mPreviewGeometry.fPushBack( tMaterialPreviewGeometryPtr( new tPlanePreviewGeometry( true ) ) );
		}
		else
		{
			mPreviewGeometry.fPushBack( tMaterialPreviewGeometryPtr( new tSpherePreviewGeometry( ) ) );
			mPreviewGeometry.fPushBack( tMaterialPreviewGeometryPtr( new tPlanePreviewGeometry( ) ) );
		}
	}

	tMaterialPreviewBundle::~tMaterialPreviewBundle( )
	{
	}

	b32 tMaterialPreviewBundle::fHasShaders( ) const
	{
		return mHlslGenOutput.mColorPShaders.fCount( ) > 0;
	}

	void tMaterialPreviewBundle::fGenerateShaders( const Derml::tFile& dermlFile, HlslGen::tToolType toolType )
	{
		// generate hlsl shader code and associated dx shader objects
		HlslGen::tHlslInput input = HlslGen::tHlslInput( HlslGen::cPidPcDx9, mVshStyle, false, false, true, toolType );
		mHlslGenOutput = HlslGen::tHlslOutput( );
		HlslGen::tHlslGenTree::fGenerateShaders( dermlFile, input, mHlslGenOutput );
		
		if( mHlslGenOutput.fCreateDx9Shaders( mDevice ) )
			mMaterial.fReset( new Gfx::tShadeMaterialPreview( mHlslGenOutput ) );
		else
			mMaterial.fRelease( );

		// regenerate geometry with proper vertex format
		for( u32 i = 0; i < mPreviewGeometry.fCount( ); ++i )
			mPreviewGeometry[ i ]->fRegenerateGeometry( mDevice, mHlslGenOutput.mStaticVShaders[ HlslGen::cWriteModeColor ].mVtxFormat.fGetRawPtr( ), mMaterial.fGetRawPtr( ) );
	}

	void tMaterialPreviewBundle::fUpdateMaterial( const Derml::tMtlFile& dermlMtlFile, Dx9Util::tTextureCache& textureCache )
	{
		for( u32 i = 0; i < dermlMtlFile.mNodes.fCount( ); ++i )
			dermlMtlFile.mNodes[ i ]->fUpdateMaterialGlueValues( mHlslGenOutput.mMaterialGlueValues, textureCache );
	}

	void tMaterialPreviewBundle::fAddToSceneGraph( tSceneGraph& sg )
	{
		for( u32 i = 0; i < mPreviewGeometry.fCount( ); ++i )
			mPreviewGeometry[ i ]->fSpawnImmediate( sg.fRootEntity( ) );
	}

	void tMaterialPreviewBundle::fRemoveFromSceneGraph( )
	{
		for( u32 i = 0; i < mPreviewGeometry.fCount( ); ++i )
			mPreviewGeometry[ i ]->fDeleteImmediate( );
	}

	void tMaterialPreviewBundle::fSetYaw( f32 yawInRads )
	{
		for( u32 i = 0; i < mPreviewGeometry.fCount( ); ++i )
			mPreviewGeometry[ i ]->fSetYaw( yawInRads );
	}

	void tMaterialPreviewBundle::fSetCurrentPreviewGeometry( u32 currentIndex )
	{
		currentIndex = currentIndex % mPreviewGeometry.fCount( );
		for( u32 i = 0; i < mPreviewGeometry.fCount( ); ++i )
			mPreviewGeometry[ i ]->fSetDisabled( i != currentIndex );
	}

}

