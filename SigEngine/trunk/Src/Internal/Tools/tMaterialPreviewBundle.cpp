#include "ToolsPch.hpp"
#include "tMaterialPreviewBundle.hpp"
#include "tSceneGraph.hpp"
#include "Derml.hpp"
#include "tSpherePreviewGeometry.hpp"
#include "tPlanePreviewGeometry.hpp"
#include "tCubePreviewGeometry.hpp"
#include "HlslGen/tHlslGenTree.hpp"
#include "Gfx/tRenderContext.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gfx/tFullBrightMaterial.hpp"
#include "Gfx/tSolidColorMaterial.hpp"
#include "Gfx/tScreen.hpp"
#include "tProjectFile.hpp"

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

		const tDynamicArray<HlslGen::tPixelShaderOutput>& fPixelShaders( ) const { return mHlslGenOutput.mColorShadowPShaders; } // mColorPShaders

		virtual b32 fIsLit( ) const { return fPixelShaders( ).fCount( ) > 1; }

		virtual b32 fRendersDepth( ) const { return true; }

		virtual void fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const
		{
			b32 depthPass = (context.mRenderPassMode == tRenderState::cRenderPassShadowMap || context.mRenderPassMode == tRenderState::cRenderPassDepth);
			b32 gPass = (context.mRenderPassMode == tRenderState::cRenderPassGBuffer);
			const u32 pShaderIdx = fMin( fPixelShaders( ).fCount( ) - 1, context.mLightShaderConstants.mLightCount );
			const HlslGen::tPixelShaderOutput& pixelShader = gPass ? mHlslGenOutput.mGBufferPShader : (depthPass ? mHlslGenOutput.mDepthOnlyPShader : fPixelShaders( )[ pShaderIdx ]);
			const HlslGen::tVertexShaderOutput& vertexShader = gPass ? mHlslGenOutput.mGBufferStaticVShader : (depthPass ? mHlslGenOutput.mStaticVShaders[ HlslGen::cWriteModeDepth ] : mHlslGenOutput.mStaticVShaders[ HlslGen::cWriteModeColor ]);
			
			IDirect3DDevice9* d3ddev = device->fGetDevice( );
			d3ddev->SetVertexShader( vertexShader.mSh.fGetRawPtr( ) );
			d3ddev->SetPixelShader( pixelShader.mSh.fGetRawPtr( ) );

			const tShadeMaterialGluePtrArray& glueVs = vertexShader.mGlueShared;
			const tShadeMaterialGluePtrArray& gluePs = pixelShader.mGlueShared;
			for( u32 i = 0; i < glueVs.fCount( ); ++i )
				glueVs[ i ]->fApplyShared( *this, mHlslGenOutput.mMaterialGlueValues, device, context );
			for( u32 i = 0; i < gluePs.fCount( ); ++i )
				gluePs[ i ]->fApplyShared( *this, mHlslGenOutput.mMaterialGlueValues, device, context );
		}

		virtual void fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& instance ) const
		{
			b32 depthPass = (context.mRenderPassMode == tRenderState::cRenderPassShadowMap || context.mRenderPassMode == tRenderState::cRenderPassDepth);
			b32 gPass = (context.mRenderPassMode == tRenderState::cRenderPassGBuffer);
			const u32 pShaderIdx = fMin( fPixelShaders( ).fCount( ) - 1, context.mLightShaderConstants.mLightCount );
			const HlslGen::tPixelShaderOutput& pixelShader = gPass ? mHlslGenOutput.mGBufferPShader : (depthPass ? mHlslGenOutput.mDepthOnlyPShader : fPixelShaders( )[ pShaderIdx ]);
			const HlslGen::tVertexShaderOutput& vertexShader = gPass ? mHlslGenOutput.mGBufferStaticVShader : (depthPass ? mHlslGenOutput.mStaticVShaders[ HlslGen::cWriteModeDepth ] : mHlslGenOutput.mStaticVShaders[ HlslGen::cWriteModeColor ]);
			
			const tShadeMaterialGluePtrArray& glueVs = vertexShader.mGlueInstance;
			const tShadeMaterialGluePtrArray& gluePs = pixelShader.mGlueInstance;
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
		: mEnableRayCast( true )
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
			mPreviewGeometry.fPushBack( tMaterialPreviewGeometryPtr( new tPlanePreviewGeometry( true, Math::tMat3f::cIdentity, true ) ) );
		}
		else
		{
			mPreviewGeometry.fSetCount( cPreviewGeometryComponentCount );

			mPreviewGeometry[ cPreviewGeometryComponentSphere ].fReset( new tSpherePreviewGeometry( true ) );
			mPreviewGeometry[ cPreviewGeometryComponentOpaqueSphere ].fReset( new tSpherePreviewGeometry( false ) );
			mPreviewGeometry[ cPreviewGeometryComponentPlane ].fReset( new tPlanePreviewGeometry( false, Math::tMat3f::cIdentity, true ) );

			Math::tMat3f groundPlaneTransform( Math::tQuatf( Math::tAxisAnglef( Math::tVec3f::cXAxis, Math::cPiOver2 ) ), Math::tVec3f( 0, -1.f, 0 ) );
			groundPlaneTransform.fScaleLocal( Math::tVec3f( 3,3,3 ) );
			mPreviewGeometry[ cPreviewGeometryComponentGroundPlane ].fReset( new tPlanePreviewGeometry( false, groundPlaneTransform, false ) );

			Math::tMat3f skyBox = Math::tMat3f::cIdentity;
			const f32 cScale = -3.f;
			skyBox.fScaleLocal( Math::tVec3f( cScale, cScale, cScale ) );
			mPreviewGeometry[ cPreviewGeometryComponentSkyBox ].fReset( new tCubePreviewGeometry( skyBox, true ) );	
			
			Math::tMat3f lightXform = Math::tMat3f::cIdentity;
			lightXform.fSetTranslation( Math::tVec3f( 2, 1.5f, 2 ) );

			Gfx::tLight light;
			light.fSetTypePoint( Math::tVec2f( 2, 5 ) );
			mLensFlareLight.fReset( NEW Gfx::tLightEntity( lightXform, light, "LenseFlare" ) );

			if( tProjectFile::fInstance( ).mEngineConfig.mRendererSettings.mLenseFlares.fCount( ) )
				mLensFlareLight->mLenseFlareKey = tProjectFile::fInstance( ).mEngineConfig.mRendererSettings.mLenseFlares[ 0 ].mKey;

			lightXform.fScaleLocal( Math::tVec3f( 0.1f ) );
			mPreviewGeometry[ cPreviewGeometryComponentLight ].fReset( new tSpherePreviewGeometry( false ) );
			mPreviewGeometry[ cPreviewGeometryComponentLight ]->fMoveTo( lightXform );
			mPreviewGeometry[ cPreviewGeometryComponentLight ]->fEnableRaycast( false );
		}

		for( u32 i = 0; i < mPreviewGeometry.fCount( ); ++i )
		{
			if( !fFullBrightObject( i ) )
			{
				mPreviewGeometry[ i ]->fSetCastsShadow( true );
				mPreviewGeometry[ i ]->fSetReceivesShadow( true );
			}
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
		HlslGen::tHlslInput input = HlslGen::tHlslInput( HlslGen::cPidPcDx9, mVshStyle, true, false, true, toolType );
		mHlslGenOutput = HlslGen::tHlslOutput( );
		HlslGen::tHlslGenTree::fGenerateShaders( dermlFile, input, mHlslGenOutput );
		 
		if( mHlslGenOutput.fCreateDx9Shaders( mDevice ) )
			mMaterial.fReset( new Gfx::tShadeMaterialPreview( mHlslGenOutput ) );
		else
			mMaterial.fRelease( );

		if( !mCubeMapRoomMaterial )
		{
			mCubeMapRoomMaterial.fReset( new Gfx::tFullBrightMaterial( ) );

			if( Gfx::tDefaultAllocators::fInstance( ).mFullBrightMaterialFile )
				mCubeMapRoomMaterial->fSetMaterialFileResourcePtrOwned( Gfx::tDefaultAllocators::fInstance( ).mFullBrightMaterialFile );

			mFullBrightFormat.fAllocate( mDevice, Gfx::tFullBrightMaterial::cVertexFormat );
		}

		// regenerate geometry with proper vertex format
		for( u32 i = 0; i < mPreviewGeometry.fCount( ); ++i )
		{
			if( fFullBrightObject( i ) )
				mPreviewGeometry[ i ]->fRegenerateGeometry( mDevice, &mFullBrightFormat, mCubeMapRoomMaterial.fGetRawPtr( ) );	
			else
				mPreviewGeometry[ i ]->fRegenerateGeometry( mDevice, mHlslGenOutput.mStaticVShaders[ HlslGen::cWriteModeColor ].mVtxFormat.fGetRawPtr( ), mMaterial.fGetRawPtr( ) );
		}
	}

	void tMaterialPreviewBundle::fUpdateMaterial( const Derml::tMtlFile& dermlMtlFile, Dx9Util::tTextureCache& textureCache )
	{
		for( u32 i = 0; i < dermlMtlFile.mNodes.fCount( ); ++i )
			dermlMtlFile.mNodes[ i ]->fUpdateMaterialGlueValues( mHlslGenOutput.mMaterialGlueValues, textureCache );

		if( mCubeMapRoomMaterial )
		{	
			if( !mCubeMapRoomTexture )
			{
				mCubeMapRoomTexture = textureCache.fFindLoad2D( tFilePathPtr( "_tools/Lighting/en_cube_d.png" ) );
			}
		}
	}

	void tMaterialPreviewBundle::fSetCubeMapTexture( Dx9Util::tBaseTexturePtr& newTex )
	{
		if( mCubeMapRoomMaterial )
		{	
			mCubeMapRoomTexture = newTex;

			Gfx::tFullBrightMaterial* fbm = mCubeMapRoomMaterial->fStaticCast<Gfx::tFullBrightMaterial>( );
			fbm->mColorMap.fSetRaw( (Gfx::tTextureReference::tPlatformHandle)mCubeMapRoomTexture.fGetRawPtr( ) );
			fbm->mColorMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeClamp );
		}
	}

	void tMaterialPreviewBundle::fAddToSceneGraph( tSceneGraph& sg )
	{
		for( u32 i = 0; i < mPreviewGeometry.fCount( ); ++i )
			mPreviewGeometry[ i ]->fSpawnImmediate( sg.fRootEntity( ) );
		if( mLensFlareLight )
			mLensFlareLight->fSpawnImmediate( sg.fRootEntity( ) );
	}

	void tMaterialPreviewBundle::fRemoveFromSceneGraph( )
	{
		for( u32 i = 0; i < mPreviewGeometry.fCount( ); ++i )
			mPreviewGeometry[ i ]->fDeleteImmediate( );

		if( mLensFlareLight )
			mLensFlareLight->fDeleteImmediate( );
	}

	void tMaterialPreviewBundle::fSetYaw( f32 yawInRads )
	{
		for( u32 i = 0; i < mPreviewGeometry.fCount( ); ++i )
		{
			if( i != cPreviewGeometryComponentSkyBox && i != cPreviewGeometryComponentGroundPlane && i != cPreviewGeometryComponentLight )
				mPreviewGeometry[ i ]->fSetYaw( yawInRads );
		}
	}

	void tMaterialPreviewBundle::fSetCurrentPreviewGeometry( tPreviewGeometryMode mode )
	{
		if( mLensFlareLight )
			mLensFlareLight->fSetOn( false );

		if( mVshStyle == HlslGen::cVshFacingQuads )
		{
			for( u32 i = 0; i < mPreviewGeometry.fCount( ); ++i )
				mPreviewGeometry[ i ]->fSetDisabled( false );
		}
		else
		{
			for( u32 i = 0; i < mPreviewGeometry.fCount( ); ++i )
				mPreviewGeometry[ i ]->fSetDisabled( true );

			switch( mode )
			{
			case cPreviewGeometryModeSphere:
				mPreviewGeometry[ cPreviewGeometryComponentSphere ]->fSetDisabled( false );
				break;
			case cPreviewGeometryModePlane:
				mPreviewGeometry[ cPreviewGeometryComponentPlane ]->fSetDisabled( false );
				break;
			case cPreviewGeometryModeSphereAndShadowedPlane:
				mPreviewGeometry[ cPreviewGeometryComponentOpaqueSphere ]->fSetDisabled( false );
				mPreviewGeometry[ cPreviewGeometryComponentGroundPlane ]->fSetDisabled( false );
				mPreviewGeometry[ cPreviewGeometryComponentSkyBox ]->fSetDisabled( false );
				mPreviewGeometry[ cPreviewGeometryComponentLight ]->fSetDisabled( false );
				mLensFlareLight->fSetOn( true );
				break;
			}
		}
	}

}

