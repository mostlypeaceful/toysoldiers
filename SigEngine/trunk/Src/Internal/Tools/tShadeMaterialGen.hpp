#ifndef __tShadeMaterialGen__
#define __tShadeMaterialGen__
#include "tMaterialGenBase.hpp"
#include "Derml.hpp"

namespace Sig
{

	///
	/// \brief Generates a tShadeMaterial material.
	class tools_export tShadeMaterialGen : public tMaterialGenBase
	{
		implement_rtti_serializable_base_class( tShadeMaterialGen, 0xA5518570 );
	public:
		Derml::tMtlFile mMtlFile;
		tFilePathPtrList mTextureDependencies;

	public:
		tShadeMaterialGen( );
		b32 fFromDermlMtlFile( const Derml::tMtlFile& dermlMtlFile );

	private:
		virtual const char* fGetMaterialName( ) const { return "Shade"; }
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual Gfx::tMaterial* fCreateGfxMaterial( tLoadInPlaceFileBase& lipFileCreator, b32 skinned );
		virtual b32	 fIsEquivalent( const Sigml::tMaterial& other ) const;
		virtual void fGetTextureResourcePaths( tFilePathPtrList& resourcePathsOut );

		// this type doesn't actually generate a material file, that part is handled in tDermlAssetPlugin
		virtual tFilePathPtr fMaterialFilePath( ) const { return tFilePathPtr( ); }
		virtual void fGenerateMaterialFileWii( tPlatformId pid ) { }
		virtual void fGenerateMaterialFilePcDx9( tPlatformId pid ) { }
		virtual void fGenerateMaterialFilePcDx10( tPlatformId pid ) { }
		virtual void fGenerateMaterialFileXbox360( tPlatformId pid ) { }
		virtual void fGenerateMaterialFilePs3Ppu( tPlatformId pid ) { }
	};
}


#endif//__tShadeMaterialGen__
