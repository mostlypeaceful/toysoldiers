#ifndef __tPhongMaterial__
#define __tPhongMaterial__
#include "tMaterial.hpp"
#include "tSphericalHarmonics.hpp"

namespace Sig { namespace Gfx
{

	class base_export tPhongMaterial : public tMaterial
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPhongMaterial, 0xCB723280 );
		define_dynamic_cast(tPhongMaterial, tMaterial);
	public:

		enum tShaderCategory
		{
			cShaderCategoryLightingVS,
			cShaderCategoryLightingPS,
			cShaderCategoryShadowMapVS,
			cShaderCategoryShadowMapPS,
			cShaderCategoryGBufferVS,
			cShaderCategoryGBufferPS,

			// last
			cShaderCategoryCount
		};

		struct base_export tLightingPass
		{
			declare_reflector( );
		public:
			enum tVS
			{
				cVSPosNormalColor,
				cVSPosColorUv0,
				cVSPosNormalColorUv0,
				cVSPosNormalTangentColorUv0,

				cVSPosNormalColor_Skinned,
				cVSPosColorUv0_Skinned,
				cVSPosNormalColorUv0_Skinned,
				cVSPosNormalTangentColorUv0_Skinned,

				cVSNonInstancedCount,

				cVSPosNormalColor_Instanced = cVSNonInstancedCount,
				cVSPosColorUv0_Instanced,
				cVSPosNormalColorUv0_Instanced,
				cVSPosNormalTangentColorUv0_Instanced,

				// last
				cVSCount
			};

			enum tPS
			{
				// non-normal mapped
				cPSPosNormalColor,
				cPSPosColor_EmissiveMapUv0,
				cPSPosNormalColor_DiffuseMapUv0,
				cPSPosNormalColor_DiffuseMapUv0_SpecColorMapUv0,
				cPSPosNormalColor_DiffuseMapUv0_EmissiveMapUv0,
				cPSPosNormalColor_DiffuseMapUv0_SpecColorMapUv0_EmissiveMapUv0,

				// normal mapped versions of the above
				cPSPosNormalTangentColor_NormalMapUv0,
				cPSPosNormalTangentColor_DiffuseMapUv0_NormalMapUv0,
				cPSPosNormalTangentColor_DiffuseMapUv0_SpecColorMapUv0_NormalMapUv0,
				cPSPosNormalTangentColor_DiffuseMapUv0_EmissiveMapUv0_NormalMapUv0,
				cPSPosNormalTangentColor_DiffuseMapUv0_SpecColorMapUv0_EmissiveMapUv0_NormalMapUv0,

				// last
				cPSCount
			};

			enum tVSConstants
			{
				cVSLocalToWorldPos		= 0,
				cVSLocalToWorldNorm		= 3,
				cVSWorldToProj			= 6,
				cVSWorldEyePos			= 10,
				cVSWorldToLight			= 11,
				cVSMatrixPalette		= 30,
			};

			enum tPSConstants
			{
				cPSRgbaTint				= 0,
				cPSDiffuseUvXform		= 1,
				cPSSpecColorUvXform		= 2,
				cPSEmissiveUvXform		= 3,
				cPSNormalUvXform		= 4,

				cPSDiffuseColor			= 5,
				cPSSpecColor			= 6,
				cPSEmissiveColor		= 7,
				cPSBumpDepth_SpecSize_Opacity_BackFaceFlip = 8,
				cPSFogValues			= 9,
				cPSFogColor				= 10,
				cPSWorldEyePos			= 11,
				cPSShadowMapEpsilon		= 12,
				cPSShadowMapEpsilon_TexelSize_Amount_Split = 13,
				cPSShadowMapTarget_Split = 14,
				cPSSphericalHarmonics	= 15,
				cPSWorldToLightArray	= cPSSphericalHarmonics + Gfx::tSphericalHarmonics::cFactorCount,

				cPSLightVectorFirst		= cPSWorldToLightArray + 4*cMaxShadowLayers,
			};

		public:
			tEnum<tVS,u16>	mVS;
			tEnum<tPS,u16>	mPS;
		public:
			tLightingPass( );
			tLightingPass( tNoOpTag );
			void fSetSkinned( b32 skinned );
			void fApplyShared( const tPhongMaterial& mtl, const tDevicePtr& device, const tRenderContext& context ) const;
			void fApplyInstance( const tPhongMaterial& mtl, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const;
			static u32 fPixelShaderSlot( tPS shaderSlot, u32 numLights );

			void fGetPSType( u32& type, u32& index, const tRenderContext& context ) const;
			void fGetVSType( u32& type, u32& index, const tRenderContext& context ) const;
		};

		struct base_export tShadowMapPass
		{
			declare_reflector( );
		public:
			enum tVS
			{
				// Directional Maps
				cVSDepth,
				cVSDepthCutOut,

				cVSDepth_Skinned,
				cVSDepthCutOut_Skinned,

				// Dual paraboloid
				cVSDepth_DP,
				cVSDepthCutOut_DP,

				cVSDepth_Skinned_DP,
				cVSDepthCutOut_Skinned_DP,

				cVSNonInstancedCount,

				cVSDepth_Instanced = cVSNonInstancedCount,
				cVSDepthCutOut_Instanced,

				// last
				cVSCount
			};

			enum tPS
			{
				// Directional Maps
				cPSDepth,
				cPSDepthCutOut,

				// Dual paraboloid
				cPSDepth_DP,
				cPSDepthCutOut_DP,

				// last
				cPSCount
			};

			enum tVSConstants
			{
				cVSLocalToWorldPos		= 0,
				cVSLocalToWorldNorm		= 3,
				cVSWorldToProj			= 6,
				cVSShadowMapSplits		= 10,
				cVSMatrixPalette		= 30,
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
			void fSetSkinned( b32 skinned );
			void fApplyShared( const tPhongMaterial& mtl, const tDevicePtr& device, const tRenderContext& context ) const;
			void fApplyInstance( const tPhongMaterial& mtl, const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const;

			void fGetPSType( u32& index, const tRenderContext& context ) const;
			void fGetVSType( u32& index, const tRenderContext& context ) const;
		};

		static const u32 cVertexFormatCount = tLightingPass::cVSCount;
		static const tVertexFormat cVertexFormats[cVertexFormatCount];
		static tFilePathPtr fMaterialFilePath( );

	public:

		u32								mVertexFormatSlot;
		tLightingPass					mLightingPass;
		tShadowMapPass					mShadowMapPass;

		tLoadInPlaceResourcePtr* 		mDiffuseMap;
		tLoadInPlaceResourcePtr* 		mSpecColorMap;
		tLoadInPlaceResourcePtr* 		mEmissiveMap;
		tLoadInPlaceResourcePtr* 		mNormalMap;

		Math::tVec4f					mDiffuseUvXform;
		Math::tVec4f					mSpecColorUvXform;
		Math::tVec4f					mEmissiveUvXform;
		Math::tVec4f					mNormalUvXform;

		Math::tVec4f					mDiffuseColor;
		Math::tVec4f					mSpecColor;
		Math::tVec4f					mEmissiveColor;
		Math::tVec4f					mBumpDepth_SpecSize_Opacity_BackFaceFlip;

	public:
		tPhongMaterial( );
		tPhongMaterial( tNoOpTag );
		void fSetSkinned( b32 skinned );
		virtual const tVertexFormat& fVertexFormat( ) const { sigassert( mVertexFormatSlot < cVertexFormatCount ); return cVertexFormats[ mVertexFormatSlot ]; }
		virtual b32 fIsLit( ) const;
		virtual b32 fRendersDepth( ) const { return fIsLit( ); }
		virtual b32 fSupportsInstancing( ) const;
		virtual void fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const;
		virtual void fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const;
	};

}}


#endif//__tPhongMaterial__
