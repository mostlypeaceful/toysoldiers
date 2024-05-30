#ifndef __tDeferredShadingMaterialGen__
#define __tDeferredShadingMaterialGen__
#include "tMaterialGenBase.hpp"

namespace Sig
{

	///
	/// \brief Generates a HeightField material file.
	class tools_export tDeferredShadingMaterialGen : public tMaterialGenBase
	{
		implement_rtti_serializable_base_class( tDeferredShadingMaterialGen, 0xDD5611DD );
	public:
		tDeferredShadingMaterialGen( );
		virtual const char* fGetMaterialName( ) const { return "DeferredShading"; }
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

		
		static void fPackFunctions( std::stringstream& ss );
		static void fUnPackFunctions( std::stringstream& ss );

		static const u32 cEmissiveFactor = 1;
		static const u32 cDiffuseFactor = 0;
	};

}

#endif//__tDeferredShadingMaterialGen__
