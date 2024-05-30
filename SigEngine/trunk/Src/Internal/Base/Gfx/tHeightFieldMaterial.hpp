#ifndef __tHeightFieldMaterial__
#define __tHeightFieldMaterial__
#include "tHeightFieldMaterialBase.hpp"
#include "tSphericalHarmonics.hpp"

namespace Sig { namespace Gfx
{

	class base_export tHeightFieldMaterial : public tHeightFieldMaterialBase
	{
		declare_reflector( );
		define_dynamic_cast( tHeightFieldMaterial, tHeightFieldMaterialBase );
		implement_rtti_serializable_base_class( tHeightFieldMaterial, 0xA3B757A1 );
	public:

		//------------------------------------------------------------------------------
		struct base_export tLightingPass
		{
			declare_reflector( );
		public:
			enum tVS
			{
				cVSDiffuse,
				cVSDiffuseNormalMap,
				cVSDiffuseCompressed,
				cVSDiffuseNormalMapCompressed,

				cVSDiffuseGBuffer,
				cVSDiffuseNormalMapGBuffer,
				cVSDiffuseGBufferCompressed,
				cVSDiffuseNormalMapGBufferCompressed,

				// last
				cVSCount
			};
			enum tPS
			{
				cPSDiffuse,
				cPSDiffuseShadow,
				cPSDiffuseNormalMap,
				cPSDiffuseNormalMapShadow,

				cPSDiffuseGBuffer,
				cPSDiffuseNormalMapGBuffer,

				// last
				cPSCount
			};
			enum tVSConstants
			{
				cVSLocalToWorld		= 0,
				cVSWorldToProj		= 3,
				cVSWorldEyePos		= 7,
				cVSWorldToLight		= 8,
			};
			enum tPSConstants
			{
				cPSWorldEyePos = 0,
				cPSWorldSpaceDims_TextureAtlasMaxPixelDims = 1,
				cPSDiffuseCount_NormalCount = 2,
				cPSFogValues = 3,
				cPSFogColor = 4,
				cPSRgbaTint = 5,
				cPSShadowMapEpsilon = 6,
				cPSShadowMapEpsilon_TexelSize_Amount_Split = 7,
				cPSShadowMapTarget_Split = 8,
				cPSTilingFactors = 9,
				cPSSphericalHarmonics = cPSTilingFactors + 8,
				cPSWorldToLightArray = cPSSphericalHarmonics + Gfx::tSphericalHarmonics::cFactorCount,
				cPSLightVectorFirst = cPSWorldToLightArray + 4 * cMaxShadowLayers,
			};
		public:
			tEnum<tVS,u16>	mVS;
			tEnum<tPS,u16>	mPS;
		public:
			tLightingPass( );
			tLightingPass( tNoOpTag );
			void fApplyShared( const tHeightFieldMaterial& mtl, const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch  ) const;
			void fApplyInstance( const tHeightFieldMaterial& mtl, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const;
			static u32 fPixelShaderSlot( tPS shaderSlot, u32 numLights );
		};
		//------------------------------------------------------------------------------
		struct base_export tShadowMapPass
		{
			declare_reflector( );
		public:
			enum tVS
			{
				cVSDepth,
				// last
				cVSCount
			};
			enum tPS
			{
				cPSDepth,
				// last
				cPSCount
			};
			enum tVSConstants
			{
				cVSLocalToWorld		= 0,
				cVSWorldToProj		= 3,
			};
			enum tPSConstants
			{
			};
		public:
			tEnum<tVS,u16>	mVS;
			tEnum<tPS,u16>	mPS;
		public:
			tShadowMapPass( );
			tShadowMapPass( tNoOpTag );
			void fApplyShared( const tHeightFieldMaterial& mtl, const tDevicePtr& device, const tRenderContext& context ) const;
			void fApplyInstance( const tHeightFieldMaterial& mtl, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const;
		};
		//------------------------------------------------------------------------------
		struct base_export tGBufferPass
		{
			declare_reflector( );
		public:
			enum tVS
			{
				cVSGBuffer,
				// last
				cVSCount
			};
			enum tPS
			{
				cPSGBuffer,
				// last
				cPSCount
			};
			enum tVSConstants
			{
				cVSLocalToWorld		= 0,
				cVSWorldToProj		= 3,
			};
			enum tPSConstants
			{
				cPSWorldSpaceDims_TextureAtlasDims = 1,
				cPSTextureAtlasPixelDims_LayerCount = 2,
				cPSTilingFactors = 8,
			};
		public:
			tEnum<tVS,u16>	mVS;
			tEnum<tPS,u16>	mPS;
		public:
			tGBufferPass( );
			tGBufferPass( tNoOpTag );
			void fApplyShared( const tHeightFieldMaterial& mtl, const tDevicePtr& device, const tRenderContext& context ) const;
			void fApplyInstance( const tHeightFieldMaterial& mtl, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const;
		};
		//------------------------------------------------------------------------------

		static tFilePathPtr fMaterialFilePath( );

	public:

		tLightingPass				mLightingPass;
		tShadowMapPass				mShadowMapPass;

	public:
		tHeightFieldMaterial( );
		tHeightFieldMaterial( tNoOpTag );
		virtual void fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const;
		virtual void fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const;
		virtual void fSetLightingPassToUseNormalMap( );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tHeightFieldMaterial );

}}//Sig::Gfx

#endif//__tHeightFieldMaterial__
