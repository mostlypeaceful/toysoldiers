#ifndef __tHeightFieldMaterialBase__
#define __tHeightFieldMaterialBase__
#include "tMaterial.hpp"
#include "tTextureReference.hpp"

namespace Sig { namespace Gfx
{

	class base_export tHeightFieldMaterialBase : public tMaterial
	{
		declare_reflector( );
		define_dynamic_cast( tHeightFieldMaterialBase, tMaterial );
		implement_rtti_serializable_base_class( tHeightFieldMaterialBase, 0x78A86B6E );
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

		static const tVertexFormat cVertexFormat;
		static const tVertexFormat cCompressedVertexFormat;

		tTextureReference			mMaskMap;
		tTextureReference			mMtlIdsMap;
		tTextureReference			mDiffuseMap;
		tTextureReference			mNormalMap;

		Math::tVec2f				mWorldSpaceDims;
		Math::tVec2f				mSubDiffuseRectDims;
		Math::tVec2f				mSubNormalRectDims;
		Math::tVec4f				mDiffuseCount_NormalCount;	//number of textures across and down in each texture atlas (a tVec4f instead of a tVec4u to prevent the LHS at runtime that would occur when converting tVec4u -> tVec4f)
		tFixedArray<Math::tVec4f,8>	mTileFactors;

	public:
		tHeightFieldMaterialBase( );
		tHeightFieldMaterialBase( tNoOpTag );
		virtual const tVertexFormat& fVertexFormat( ) const { return cVertexFormat; }
		virtual b32 fIsLit( ) const { return true; }
		virtual b32 fRendersDepth( ) const { return true; }
		virtual void fSetLightingPassToUseNormalMap( ) = 0;
	protected:
		static b32 fLod_TerrainNormals( );
		static b32 fLod_TerrainShadows( );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tHeightFieldMaterialBase );

}}//Sig::Gfx

#endif//__tHeightFieldMaterialBase__
