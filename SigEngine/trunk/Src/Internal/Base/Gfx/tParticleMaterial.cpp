#include "BasePch.hpp"
#include "tParticleMaterial.hpp"
#include "tMaterialFile.hpp"
#include "tDrawCall.hpp"
#include "tRenderContext.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		const tVertexElement gVertexFormatElements[]=
		{
			tVertexElement( tVertexElement::cSemanticPosition, tVertexElement::cFormat_f32_3 ),
			tVertexElement( tVertexElement::cSemanticColor, tVertexElement::cFormat_u8_4_Color ),
			tVertexElement( tVertexElement::cSemanticTexCoord, tVertexElement::cFormat_f32_4, 0 ),
		};
		const tFilePathPtr cParticlePath( "Shaders/Engine/Particle.mtlb" );
	}
	const tVertexFormat tParticleMaterial::cVertexFormat( gVertexFormatElements, array_length( gVertexFormatElements ) );

	tFilePathPtr tParticleMaterial::fMaterialFilePath( )
	{
		return cParticlePath;
	}

	tParticleMaterial::tParticleMaterial( )
	{
		mRenderState = tRenderState::cDefaultColorOpaque;
	}

	tParticleMaterial::tParticleMaterial( const tResourcePtr& particleMtlFile )
	{
		fSetMaterialFileResourcePtrOwned( particleMtlFile );
		//mEmissiveMap.fSetDynamic( textureFile.fGetRawPtr( ) );
		//mEmissiveMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeClamp );
		//mEmissiveMap.fGetDynamic( )->fLoadDefault( this );
	}

	tParticleMaterial::tParticleMaterial( tNoOpTag )
	{
	}

	tParticleMaterial::~tParticleMaterial( )
	{
		//if( mEmissiveMap.fGetDynamic( ) )
		//	mEmissiveMap.fGetDynamic( )->fUnload( this );
	}

	void tParticleMaterial::fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const
	{
		sigassert( sizeof( tParticleRenderVertex ) == cVertexFormat.fVertexSize( ) );

		const tMaterialFile* mtlFile = mMaterialFile->fGetResourcePtr( )->fCast<tMaterialFile>( );

		// apply vertex and pixel shaders
		mtlFile->fApplyShader( device, 0, cShaderSlotVS );
		mtlFile->fApplyShader( device, 0, cShaderSlotPS );

		// vertex shader constants
		fApplyMatrix4VS( device, cVSViewToProj, context.mSceneCamera->fGetCameraToProjection( ) );
		fApplyVector4VS( device, cVSWorldEyePos, Math::tVec4f( context.mSceneCamera->fGetTripod( ).mEye, context.mSceneCamera->fGetLens( ).mFarPlane ) );
		
		// apply textures
		//mEmissiveMap.fApply( device, 0 );
	}

	void tParticleMaterial::fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const
	{
		const tRenderInstance& instance = drawCall.fRenderInstance( );

		// vertex shader constants
		const Math::tMat3f localToView = context.mSceneCamera->fGetWorldToCamera() *	instance.fRI_ObjectToWorld( );
		fApplyMatrix3VS( device, cVSLocalToView, localToView );

		// set pixel shader constants
		fApplyVector4PS( device, cPSRgbaTint, drawCall.fInstanceRgbaTint( ) );

		renderBatch.fRenderInstance( device );
	}

}}
