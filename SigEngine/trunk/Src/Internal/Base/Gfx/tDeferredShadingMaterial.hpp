#ifndef __tDeferredShadingMaterial__
#define __tDeferredShadingMaterial__
#include "tMaterial.hpp"
#include "tCamera.hpp"
#include "tRenderContext.hpp"
#include "tSolidColorGeometry.hpp"

namespace Sig { namespace Gfx
{

	class tLightEntity;

	class base_export tDeferredShadingMaterial : public tMaterial
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tDeferredShadingMaterial, 0xD6F775DD );
		define_dynamic_cast(tDeferredShadingMaterial, tMaterial);
	public:

		enum tShaderCategory
		{
			cShaderCategoryVS,
			cShaderCategoryPS,
			cShaderCategoryCount
		};

		enum tVSShaders
		{
			cVSShaderQuad, // Renders a full screen quad
			cVSShader3D,   // Renders world space geometry, such as a light sphere
			cVSShader3DDepthOnly,
			cVSShadersCount
		};

		enum tPSShaders
		{
			cPSShadersDirectionalLight_Shadowed,
			cPSShadersPointLight,
			cPSShadersPointLight_Shadowed,
			cPSShadersPointLight_ShadowedPCF,
			cPSShadersDirectionalLight_NormalShadowed, // No shadow map sampling, but still normal based calcs.
			cPSShadersDirectionalLight,
			cPSShadersCount
		};

		enum tVSConstants
		{
			cVSScreenCamera		= 0,
			cVSSceneCamera		= 4,
			cVSRenderTargetDims	= 8,
			cVSViewportXform	= 9,
		};
		enum tPSConstants
		{
			cPSProjToWorld		= 0,
			cPSWorldEyePos		= 4,
			cPSShadowMapEpsilon = 5,
			cPSShadowMapEpsilon_TexelSize_Amount_Split = 6,
			cPSShadowMapTarget_Split = 7,
			cPSHalfLambert = 8,
			cPSWorldToLightArray = 9,
			cPSLightVectorFirst = cPSWorldToLightArray + 4*cMaxShadowLayers,
		};

		enum tTextureUnits
		{
			cTexUnitDepth,
			cTexUnitShadow,
			cTexUnitGbuffer0,	// [Diffuse RGB, SpecPower]
			cTexUnitGbuffer1,	// [Normal XYZ, SpecValue]
			cTexUnitGbuffer2,	// [Emissive RGB, unused ]
			cTexUnitGbuffer3,	// [Ambient RGB, unused ]
			cTexUnitCount
		};

		static const tVertexFormat cVertexFormat;
		static const tVertexFormat cCompressedVertexFormat;
		static tFilePathPtr fMaterialFilePath( );

		static Math::tVec3f fDebugUseHalfLambert( );

	public:
		struct tDeferredShadingContext
		{
			declare_null_reflector( );
			Gfx::tCamera		mFullScreenCamera;
			tRenderState		mRenderState;
			tRenderContext		mContext;
			tRenderBatchData	mFullScreenQuad;
			tRenderBatchData	mUnitSphere;
			tRenderBatchData*	mLastAppliedBatch;

			tDeferredShadingContext( );
			void fSetScreen( tScreen& screen );
		};

		mutable u32		mLastAppliedVS;
		mutable u32		mLastAppliedPS;
		mutable tDeferredShadingContext mContext;



	public:
		tDeferredShadingMaterial( );
		tDeferredShadingMaterial( tNoOpTag );
		virtual const tVertexFormat& fVertexFormat( ) const { return cVertexFormat; }
		virtual b32 fIsLit( ) const { return true; }
		virtual b32 fRendersDepth( ) const { return true; }

		virtual void fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const;
		virtual void fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const;
		
		void fResetLightPass( );
		void fRenderLightList( tLightEntityList& list, tLightEntityList& shadowLights, tTextureReference& blankShadowMap, tScreen& screen );

	private:
		void fApplyLightVS( b32 depthOnly, const tDevicePtr& device, const tRenderContext& context, const tLightEntity& light );
		void fApplyLightPS( const tDevicePtr& device, const tRenderContext& context, const tLightEntity& light, b32 castsShadows );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tDeferredShadingMaterial );

	class base_export tDeferredShadingSphere : public tSolidColorGeometry
	{
		f32 mRadius;
	public:
		tDeferredShadingSphere( );
		virtual void fOnDeviceReset( tDevice* device );
		void fGenerate( f32 radius = 1.f );
	};
}}


#endif//__tDeferredShadingMaterial__

