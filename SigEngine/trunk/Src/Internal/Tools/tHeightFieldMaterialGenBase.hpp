#ifndef __tHeightFieldMaterialGenBase__
#define __tHeightFieldMaterialGenBase__
#include "tMaterialGenBase.hpp"

namespace Sig
{

	///
	/// \brief Generates a HeightField material file.
	class tools_export tHeightFieldMaterialGenBase : public tMaterialGenBase
	{
		implement_rtti_serializable_base_class( tHeightFieldMaterialGenBase, 0x0410A038 );
	public:
		Math::tVec2f mWorldSpaceDims;
		Math::tVec2f mSubDiffuseRectDims;
		Math::tVec2f mSubNormalRectDims;
		Math::tVec4f mDiffuseCount_NormalCount; //number of textures across and down in each texture atlas (a tVec4f instead of a tVec4u to prevent the LHS at runtime that would occur when converting tVec4u -> tVec4f)
		tFixedArray<Math::tVec4f,8> mTileFactors;
		tFilePathPtr mMaskTextureName;
		tFilePathPtr mMtlIdsTextureName;
		tFilePathPtr mDiffuseTextureName;
		tFilePathPtr mNormalMapTextureName;
	public:
		tHeightFieldMaterialGenBase( );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual Gfx::tMaterial* fCreateHeightFieldGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned, b32 compressedVerts ) = 0;
		virtual b32  fIsEquivalent( const Sigml::tMaterial& other ) const;
		virtual void fGetTextureResourcePaths( tFilePathPtrList& resourcePathsOut ) { }
	};

}

#endif//__tHeightFieldMaterialGenBase__
