#ifndef __tHeightFieldMaterialGen__
#define __tHeightFieldMaterialGen__
#include "tHeightFieldMaterialGenBase.hpp"

namespace Sig
{

	///
	/// \brief Generates a HeightField material file.
	class tools_export tHeightFieldMaterialGen : public tHeightFieldMaterialGenBase
	{
		implement_rtti_serializable_base_class( tHeightFieldMaterialGen, 0xB14F552D );
	public:
		tHeightFieldMaterialGen( );
		virtual const char* fGetMaterialName( ) const { return "HeightField"; }
		virtual Gfx::tMaterial* fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned ) { return fCreateHeightFieldGfxMaterial( lipFileCreator, skinned, false ); }
		virtual Gfx::tMaterial* fCreateHeightFieldGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned, b32 compressedVerts );
		virtual tFilePathPtr fMaterialFilePath( ) const;
		virtual void fGenerateMaterialFileWii( tPlatformId pid );
		virtual void fGenerateMaterialFilePcDx9( tPlatformId pid );
		virtual void fGenerateMaterialFilePcDx10( tPlatformId pid );
		virtual void fGenerateMaterialFileXbox360( tPlatformId pid );
		virtual void fGenerateMaterialFilePs3Ppu( tPlatformId pid );
	};

}

#endif//__tHeightFieldMaterialGen__
