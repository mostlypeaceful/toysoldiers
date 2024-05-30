#ifndef __tHeightFieldMaterial__
#define __tHeightFieldMaterial__
#include "tMaterial.hpp"
#include "tTextureReference.hpp"

namespace Sig { namespace Gfx
{

	class base_export tHeightFieldMaterial : public tMaterial
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tHeightFieldMaterial, 0xAA8B16E8 );
		define_dynamic_cast(tHeightFieldMaterial, tMaterial);
	public:

		enum tShaderCategory
		{
			cShaderCategoryLightingVS,
			cShaderCategoryLightingPS,
			cShaderCategoryShadowMapVS,
			cShaderCategoryShadowMapPS,

			// last
			cShaderCategoryCount
		};

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

				// last
				cVSCount
			};
			enum tPS
			{
				cPSDiffuse,
				cPSDiffuseNormalMap,

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
				cPSWorldSpaceDims_TextureAtlasDims = 1,
				cPSTextureAtlasPixelDims_LayerCount = 2,
				cPSFogValues = 3,
				cPSFogColor = 4,
				cPSRgbaTint = 5,
				cPSShadowMapEpsilon_TexelSize_Amount_Split = 6,
				cPSShadowMapTarget_Split = 7,
				cPSTilingFactors = 8,
				cPSWorldToLightArray = cPSTilingFactors + 8,
				cPSLightVectorFirst = cPSWorldToLightArray + 4*cMaxShadowLayers,
			};
		public:
			tEnum<tVS,u16>	mVS;
			tEnum<tPS,u16>	mPS;
		public:
			tLightingPass( );
			tLightingPass( tNoOpTag );
			void fApplyShared( const tHeightFieldMaterial& mtl, const tDevicePtr& device, const tRenderContext& context ) const;
			void fApplyInstance( const tHeightFieldMaterial& mtl, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const;
			static u32 fPixelShaderSlot( tPS shaderSlot, u32 numLights );
		};


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

		static const tVertexFormat cVertexFormat;
		static const tVertexFormat cCompressedVertexFormat;
		static tFilePathPtr fMaterialFilePath( );

	public:

		u32							mVertexFormatSlot;
		tLightingPass				mLightingPass;
		tShadowMapPass				mShadowMapPass;

		tTextureReference			mMaskMap;
		tTextureReference			mMtlIdsMap;
		tTextureReference			mDiffuseMap;
		tTextureReference			mNormalMap;

		Math::tVec2f				mWorldSpaceDims;
		Math::tVec2u				mTextureAtlasDims;
		Math::tVec2f				mSubDiffuseRectDims;
		Math::tVec2f				mSubNormalRectDims;
		tFixedArray<Math::tNoOpVec4f,8>	mTileFactors;

	public:
		tHeightFieldMaterial( );
		tHeightFieldMaterial( tNoOpTag );
		virtual const tVertexFormat& fVertexFormat( ) const { return cVertexFormat; }
		virtual b32 fIsLit( ) const { return true; }
		virtual b32 fRendersDepth( ) const { return true; }
		virtual void fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const;
		virtual void fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tHeightFieldMaterial );

}}


#endif//__tHeightFieldMaterial__

