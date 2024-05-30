#ifndef __tHeightFieldMaterialGen__
#define __tHeightFieldMaterialGen__
#include "tMaterialGenBase.hpp"

namespace Sig
{

	///
	/// \brief Generates a HeightField material file.
	class tools_export tHeightFieldMaterialGen : public tMaterialGenBase
	{
		implement_rtti_serializable_base_class( tHeightFieldMaterialGen, 0x239AC8AB );
	public:
		Math::tVec2f mWorldSpaceDims;
		Math::tVec2u mTextureAtlasDims;
		Math::tVec2f mSubDiffuseRectDims;
		Math::tVec2f mSubNormalRectDims;
		tFixedArray<Math::tNoOpVec4f,8> mTileFactors;
		tFilePathPtr mMaskTextureName;
		tFilePathPtr mMtlIdsTextureName;
		tFilePathPtr mDiffuseTextureName;
		tFilePathPtr mNormalMapTextureName;
	public:
		tHeightFieldMaterialGen( );
		virtual const char* fGetMaterialName( ) const { return "HeightField"; }
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual Gfx::tMaterial* fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned ) { return fCreateGfxMaterial( lipFileCreator, skinned, false ); }
		Gfx::tMaterial* fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned, b32 compressedVerts );
		virtual b32  fIsEquivalent( const Sigml::tMaterial& other ) const;
		virtual void fGetTextureResourcePaths( tFilePathPtrList& resourcePathsOut ) { }
		virtual tFilePathPtr fMaterialFilePath( ) const;
		virtual void fGenerateMaterialFileWii( tPlatformId pid );
		virtual void fGenerateMaterialFilePcDx9( tPlatformId pid );
		virtual void fGenerateMaterialFilePcDx10( tPlatformId pid );
		virtual void fGenerateMaterialFileXbox360( tPlatformId pid );
		virtual void fGenerateMaterialFilePs3Ppu( tPlatformId pid );
	};

}

#endif//__tHeightFieldMaterialGen__
