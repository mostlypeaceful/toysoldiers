#ifndef __tSolidColorMaterialGen__
#define __tSolidColorMaterialGen__
#include "tMaterialGenBase.hpp"

namespace Sig
{

	///
	/// \brief Generates a SolidColor material file.
	class tools_export tSolidColorMaterialGen : public tMaterialGenBase
	{
		implement_rtti_serializable_base_class( tSolidColorMaterialGen, 0x8C67E120 );
	private:
		virtual const char* fGetMaterialName( ) const { return "SolidColor"; }
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual Gfx::tMaterial* fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned );
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

#endif//__tSolidColorMaterialGen__
