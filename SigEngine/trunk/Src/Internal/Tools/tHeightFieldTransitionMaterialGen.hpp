#ifndef __tHeightFieldTransitionMaterialGen__
#define __tHeightFieldTransitionMaterialGen__
#include "tHeightFieldMaterialGenBase.hpp"

namespace Sig
{

	///
	/// \brief Generates a HeightField material file.
	class tools_export tHeightFieldTransitionMaterialGen : public tHeightFieldMaterialGenBase
	{
		implement_rtti_serializable_base_class( tHeightFieldTransitionMaterialGen, 0x7C21E21D );
	public:
		tHeightFieldTransitionMaterialGen( );
		virtual const char* fGetMaterialName( ) const { return "HeightFieldTransition"; }
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

#endif//__tHeightFieldTransitionMaterialGen__
