#ifndef __tParticleMaterialGen__
#define __tParticleMaterialGen__
#include "tMaterialGenBase.hpp"

namespace Sig
{

	///
	/// \brief Generates a Particle material file.
	class tools_export tParticleMaterialGen : public tMaterialGenBase
	{
		implement_rtti_serializable_base_class( tParticleMaterialGen, 0x671488E4 );
	private:
		virtual const char* fGetMaterialName( ) const { return "Particle"; }
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

#endif//__tParticleMaterialGen__
