#ifndef __ClassRegistration__
#define __ClassRegistration__ 1 // define as 1 so we can use in static_assert

#ifdef throw
#	undef throw
#endif

#include "tLoadInPlaceDeserializer.hpp"

// for register_rtti_factory
#include "tMeshEntity.hpp"
#include "tAttachmentEntity.hpp"
#include "tPathEntity.hpp"
#include "tShapeEntity.hpp"
#include "t3dGridEntity.hpp"
#include "tPathDecalEntityDef.hpp"
#include "tNavGraphEntity.hpp"
#include "tHeightFieldMeshEntity.hpp"
#include "Gfx/tCameraEntity.hpp"
#include "Gfx/tDefaultMaterial.hpp"
#include "Gfx/tSolidColorMaterial.hpp"
#include "Gfx/tFullBrightMaterial.hpp"
#include "Gfx/tPhongMaterial.hpp"
#include "Gfx/tFontMaterial.hpp"
#include "Gfx/tHeightFieldMaterial.hpp"
#include "Gfx/tHeightFieldTransitionMaterial.hpp"
#include "Gfx/tParticleMaterial.hpp"
#include "Gfx/tShadeMaterial.hpp"
#include "tTileEntity.hpp"
#include "tTileCanvas.hpp"
#include "tTilePackage.hpp"

// Plugin data
#include "tPluginMinimapData.hpp"


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
#include "tSwfFile.hpp"
#include "tXmlFile.hpp"
#include "tTtfFile.hpp"
#include "tMotionMapFile.hpp"
#include "tAniMapFile.hpp"

namespace Sig
{
	tStaticFunctionCall gStaticLogIntializer( &Log::fInitializeSystem );

	register_rtti_factory( tMeshEntityDef, true )
	register_rtti_factory( tAttachmentEntityDef, true )
	register_rtti_factory( tPathEntityDef, true )
	register_rtti_factory( tShapeEntityDef, true )
	register_rtti_factory( t3dGridEntityDef, true )
	register_rtti_factory( tPathDecalEntityDef, true )
	register_rtti_factory( tNavGraphEntityDef, true )
	register_rtti_factory( tCameraEntityDef, true )
	register_rtti_factory( tTileCanvasEntityDef, true )
	register_rtti_factory( tTileEntityDef, true );
	register_rtti_factory( tHeightFieldMeshEntityDef, true )

	implement_lip_load_unload_callbacks( tFilePackageFile )
	implement_lip_load_unload_callbacks( tLocalizationFile )
	implement_lip_load_unload_callbacks( tDataTableFile )
	implement_lip_load_unload_callbacks( tScriptFile )
	implement_lip_load_unload_callbacks( tSkeletonFile )
	implement_lip_load_unload_callbacks( tAnimPackFile )
	implement_lip_load_unload_callbacks( tSceneGraphFile )
	implement_lip_load_unload_callbacks( tSwfFile )
	implement_lip_load_unload_callbacks( tXmlFile )
	implement_lip_load_unload_callbacks( tTtfFile )
	implement_lip_load_unload_callbacks( tMotionMapFile )
	implement_lip_load_unload_callbacks( tAniMapFile )
	implement_lip_load_unload_callbacks( tTilePackage )

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
		register_rtti_factory( tHeightFieldTransitionMaterial, true );
		register_rtti_factory( tParticleMaterial, true );
		register_rtti_factory( tShadeMaterial, true );

		implement_lip_load_unload_callbacks_direct( tGeometryFile )
		implement_lip_load_unload_callbacks( tMaterialFile )
		implement_lip_load_unload_callbacks_direct( tTextureFile )
		
	}

	namespace FX
	{
		implement_lip_load_unload_callbacks( tFxFile )
	}

	// Engine plugin data
	register_rtti_factory( tEntityData, true );
	register_rtti_factory( tPluginMiniMapData, true );

	void fGlobalClassRegistration( ); // Squelch GCC compiler warning
	void fGlobalClassRegistration( )
	{
		// this function is required simply to avoid the contents of this file being stripped out by the linker
	}
}
#endif//__ClassRegistration__
