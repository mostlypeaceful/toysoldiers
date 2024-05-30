#ifndef __ClassRegistration__
#define __ClassRegistration__ 1 // define as 1 so we can use in sig_static_assert

#ifdef throw
#	undef throw
#endif

#include "tLoadInPlaceDeserializer.hpp"

// for register_rtti_factory
#include "tMeshEntity.hpp"
#include "tHeightFieldMeshEntity.hpp"
#include "tAttachmentEntity.hpp"
#include "tPathEntity.hpp"
#include "tShapeEntity.hpp"
#include "t3dGridEntity.hpp"
#include "tPathDecalEntityDef.hpp"
#include "tNavGraphEntity.hpp"
#include "Gfx/tDefaultMaterial.hpp"
#include "Gfx/tSolidColorMaterial.hpp"
#include "Gfx/tFullBrightMaterial.hpp"
#include "Gfx/tPhongMaterial.hpp"
#include "Gfx/tFontMaterial.hpp"
#include "Gfx/tHeightFieldMaterial.hpp"
#include "Gfx/tParticleMaterial.hpp"
#include "Gfx/tShadeMaterial.hpp"
#include "tTileEntity.hpp"


// for implement_lip_load_unload_callbacks
#include "tFilePackageFile.hpp"
#include "tLocalizationFile.hpp"
#include "tDataTableFile.hpp"
#include "Scripts/tScriptFile.hpp"
#include "tSkeletonFile.hpp"
#include "tAnimPackFile.hpp"
#include "tSceneGraphFile.hpp"
#include "Gui/tFont.hpp"
#include "Gfx/tGeometryFile.hpp"
#include "Gfx/tMaterialFile.hpp"
#include "Gfx/tTextureFile.hpp"
#include "FX/tFxFile.hpp"
#include "tXmlFile.hpp"

namespace Sig
{
	tStaticFunctionCall gStaticLogIntializer( &Log::fInitializeSystem );

	register_rtti_factory( tMeshEntityDef, true )
	register_rtti_factory( tHeightFieldMeshEntityDef, true )
	register_rtti_factory( tAttachmentEntityDef, true )
	register_rtti_factory( tPathEntityDef, true )
	register_rtti_factory( tShapeEntityDef, true )
	register_rtti_factory( t3dGridEntityDef, true )
	register_rtti_factory( tPathDecalEntityDef, true )
	register_rtti_factory( tNavGraphEntityDef, true )
	register_rtti_factory( tTileEntityDef, true )

	implement_lip_load_unload_callbacks( tFilePackageFile )
	implement_lip_load_unload_callbacks( tLocalizationFile )
	implement_lip_load_unload_callbacks( tDataTableFile )
	implement_lip_load_unload_callbacks( tScriptFile )
	implement_lip_load_unload_callbacks( tSkeletonFile )
	implement_lip_load_unload_callbacks( tAnimPackFile )
	implement_lip_load_unload_callbacks( tSceneGraphFile )
	implement_lip_load_unload_callbacks( tXmlFile )

	namespace Gui
	{
		implement_lip_load_unload_callbacks( tFont )
	}

	namespace Gfx
	{
		register_rtti_factory( tDefaultMaterial, true );
		register_rtti_factory( tSolidColorMaterial, true );
		register_rtti_factory( tFullBrightMaterial, true );
		register_rtti_factory( tPhongMaterial, true );
		register_rtti_factory( tFontMaterial, true );
		register_rtti_factory( tHeightFieldMaterial, true );
		register_rtti_factory( tParticleMaterial, true );
		register_rtti_factory( tShadeMaterial, true );

		implement_lip_load_unload_callbacks( tGeometryFile )
		implement_lip_load_unload_callbacks( tMaterialFile )
		implement_lip_load_unload_callbacks( tTextureFile )
	}

	namespace FX
	{
		implement_lip_load_unload_callbacks( tFxFile )
	}

	void fGlobalClassRegistration( )
	{
		// this function is required simply to avoid the contents of this file being stripped out by the linker
	}
}
#endif//__ClassRegistration__
