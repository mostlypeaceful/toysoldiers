#ifndef __tDecalMaterial__
#define __tDecalMaterial__
#include "tMaterial.hpp"
#include "tTextureReference.hpp"

namespace Sig { namespace Gfx
{
	struct base_export tDecalRenderVertex
	{
		declare_reflector( );
	public:
		Math::tVec3f	mP;
		Math::tVec3f	mN;
		Math::tVec4f	mTan;
		Math::tVec2f	mUv;
		u32				mColor;

		inline tDecalRenderVertex( ) { }
		inline tDecalRenderVertex( tNoOpTag ) { }
		inline tDecalRenderVertex( const Math::tVec3f& p, const Math::tVec3f& n, const Math::tVec4f& tan, const Math::tVec2f& uv, u32 c )
			: mP( p ), mN( n ), mTan( tan ), mUv( uv ), mColor( c ) { }
	};

	class base_export tDecalMaterial : public tMaterial
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tDecalMaterial, 0x8264297A );
		define_dynamic_cast(tDecalMaterial, tMaterial);
	public:

		enum tShaderCategory
		{
			cShaderCategoryVS,
			cShaderCategoryPS,

			// last
			cShaderCategoryCount
		};

		enum tVS
		{
			cVSFullBright,
			cVSDiffuse,
			cVSDiffuseNormal,

			cVSCount
		};

		enum tPS
		{
			// Note: when adding shaders, add them to the proper type in this enum.

			cPSFullBright,
			cPSNumUnlit,

			cPSDiffuse = cPSNumUnlit,
			cPSDiffuseShadow,
			cPSDiffuseNormal,
			cPSDiffuseNormalShadow,

			cPSCount,

			cPSNumLit = cPSCount - cPSNumUnlit,
		};

		enum tVSConstants
		{
			cVSLocalToWorldPos		= 0,
			cVSLocalToWorldNorm		= 3,
			cVSWorldToProj			= 6,
			cVSWorldEyePos			= 10,
			cVSWorldToLight			= 11,
		};

		enum tPSConstants
		{
			cPSRgbaTint				= 0,
			cPSDiffuseUvXform		= 1,
			cPSNormalUvXform		= 2,
			cPSBumpDepth_SpecSize_Opacity_BackFaceFlip = 3,
			cPSFogValues			= 4,
			cPSFogColor				= 5,
			cPSWorldEyePos			= 6,
			cPSShadowMapEpsilon		= 7,
			cPSShadowMapEpsilon_TexelSize_Amount_Split = 8,
			cPSShadowMapTarget_Split = 9,
			cPSWorldToLightArray	= 11,

			cPSLightVectorFirst		= cPSWorldToLightArray + 4*cMaxShadowLayers,
		};

	public:
		tEnum<tVS,u16>	mVS;
		tEnum<tPS,u16>	mPS;

		tTextureReference		mDiffuseMap;
		tTextureReference		mNormalMap;

		Math::tVec4f			mDiffuseUvXform;
		Math::tVec4f			mNormalUvXform;

		Math::tVec4f			mBumpDepth_SpecSize_Opacity_BackFaceFlip;

	public:
		void fApplySharedTex( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const;
		void fApplyInstanceTex( const tDevicePtr& device, const tRenderContext& context, const tDrawCall& drawCall ) const;
		void fApplySharedFB( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const;
		void fApplyInstanceFB( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const;
		static u32 fPixelShaderSlot( u32 shaderSlot, u32 numLights );

		static const tVertexFormat cVertexFormat;
		static tFilePathPtr fMaterialFilePath( );

	public:
		tDecalMaterial( );
		tDecalMaterial( tNoOpTag );
		virtual const tVertexFormat& fVertexFormat( ) const { return cVertexFormat; }
		virtual b32 fIsLit( ) const;
		virtual b32 fRendersDepth( ) const { return fIsLit( ); }
		virtual void fApplyShared( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch ) const;
		virtual void fApplyInstance( const tDevicePtr& device, const tRenderContext& context, const tRenderBatchData& renderBatch, const tDrawCall& drawCall ) const;
		void fSetAcceptsLights( b32 acceptLights );
	};

}}


#endif//__tDecalMaterial__
