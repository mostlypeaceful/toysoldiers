#ifndef __tFontMaterialGen__
#define __tFontMaterialGen__
#include "tMaterialGenBase.hpp"

namespace Sig
{
	namespace Gfx
	{
		class tFontMaterial;
	}

	///
	/// \brief Generates a Font material file.
	class tools_export tFontMaterialGen : public tMaterialGenBase
	{
		implement_rtti_serializable_base_class( tFontMaterialGen, 0x367BC994 );
	public:
		tFilePathPtr mFontMap;
		b32 mOutline;
	public:
		tFontMaterialGen( );
		Gfx::tFontMaterial* fCreateFontMaterial( tLoadInPlaceFileBase& lipFileCreator );
	private:
		virtual const char* fGetMaterialName( ) const { return "Font"; }
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

#endif//__tFontMaterialGen__
