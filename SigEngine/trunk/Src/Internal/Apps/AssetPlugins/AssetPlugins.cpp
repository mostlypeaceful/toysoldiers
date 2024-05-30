#include "ToolsPch.hpp"
#include "tAssetPluginDll.hpp"

// general
#include "tStandardAssetPlugin.hpp"

// exporters
#include "tSigmlAssetPlugin.hpp"
#include "tSklmlAssetPlugin.hpp"
#include "tAnimlAssetPlugin.hpp"
#include "tDermlAssetPlugin.hpp"
#include "tFxmlAssetPlugin.hpp"
#include "tGoamlAssetPlugin.hpp"
#include "tTiledbAssetPlugin.hpp"
#include "tXmlAssetPlugin.hpp"

// textures
#include "tTextureGen.hpp"

// fonts
#include "tFontGen.hpp"

// materials
#include "tDefaultMaterialGen.hpp"
#include "tSolidColorMaterialGen.hpp"
#include "tFullBrightMaterialGen.hpp"
#include "tPhongMaterialGen.hpp"
#include "tFontMaterialGen.hpp"
#include "tHeightFieldMaterialGen.hpp"
#include "tHeightFieldTransitionMaterialGen.hpp"
#include "tParticleMaterialGen.hpp"
#include "tPostEffectsMaterialGen.hpp"
#include "tDecalMaterialGen.hpp"
#include "tDeferredShadingMaterialGen.hpp"

// Animation
#include "tMotionMapAssetPlugin.hpp"
#include "tAnimapAssetPlugin.hpp"

// scripting
#include "tScriptFileConverter.hpp"

// data tables
#include "tDataTableAssetPlugin.hpp"

// localization
#include "tLocmlAssetPlugin.hpp"

// swfs (flash ui)
#include "tSwfAssetPlugin.hpp"

// ttf ( True Type Font )
#include "tTtfAssetPlugin.hpp"

using namespace Sig;


dll_export u32 fGetPluginVersion( )
{
	return asset_plugin_version;
}

dll_export void fCreatePlugins( tAssetPluginDll::tPluginList& pluginsAddTo )
{
	// general
	static tStandardAssetPlugin gStdPlugin;
	pluginsAddTo.fPushBack( &gStdPlugin );

	// exporters
	static tSigmlAssetPlugin gSigml;
	pluginsAddTo.fPushBack( &gSigml );
	static tSklmlAssetPlugin gSklml;
	pluginsAddTo.fPushBack( &gSklml );
	static tAnimlAssetPlugin gAniml;
	pluginsAddTo.fPushBack( &gAniml );
	static tDermlAssetPlugin gDerml;
	pluginsAddTo.fPushBack( &gDerml );
	static tGoamlAssetPlugin gGoaml;
	pluginsAddTo.fPushBack( &gGoaml );
	static tFxmlAssetPlugin gFxml;
	pluginsAddTo.fPushBack( &gFxml );
	static tTiledmlAssetPlugin gTiledml;
	pluginsAddTo.fPushBack( &gTiledml );
	static tXmlAssetPlugin gXml;
	pluginsAddTo.fPushBack( &gXml );	

	// textures
	static tTextureGen gTextureGen;
	pluginsAddTo.fPushBack( &gTextureGen );

	// fonts
	static tFontGen gFontGen;
	pluginsAddTo.fPushBack( &gFontGen );

	// materials
	static tDefaultMaterialGen gDefaultMtlGen;
	pluginsAddTo.fPushBack( &gDefaultMtlGen );
	static tSolidColorMaterialGen gSolidColorMtlGen;
	pluginsAddTo.fPushBack( &gSolidColorMtlGen );
	static tFullBrightMaterialGen gFullBrightMtlGen;
	pluginsAddTo.fPushBack( &gFullBrightMtlGen );
	static tPhongMaterialGen gPhongMaterialGen;
	pluginsAddTo.fPushBack( &gPhongMaterialGen );
	static tFontMaterialGen gFontMaterialGen;
	pluginsAddTo.fPushBack( &gFontMaterialGen );
	static tHeightFieldMaterialGen gHeightFieldMaterialGen;
	pluginsAddTo.fPushBack( &gHeightFieldMaterialGen );
	static tHeightFieldTransitionMaterialGen gHeightFieldTransitionMaterialGen;
	pluginsAddTo.fPushBack( &gHeightFieldTransitionMaterialGen );
	static tParticleMaterialGen gParticleMaterialGen;
	pluginsAddTo.fPushBack( &gParticleMaterialGen );
	static tPostEffectsMaterialGen gPostEffectsMaterialGen;
	pluginsAddTo.fPushBack( &gPostEffectsMaterialGen );
	static tDecalMaterialGen gDecalMaterialGen;
	pluginsAddTo.fPushBack( &gDecalMaterialGen );
	static tDeferredShadingMaterialGen gDeferredShadingMaterialGen;
	pluginsAddTo.fPushBack( &gDeferredShadingMaterialGen );

	// animation
	static tMotionMapAssetPlugin gMotionMapFileConverter;
	pluginsAddTo.fPushBack( &gMotionMapFileConverter );
	static tAnimapAssetPlugin gAnimapFileConverter;
	pluginsAddTo.fPushBack( &gAnimapFileConverter );	

	// scripting
	static tScriptFileConverter gScriptFileConverter;
	pluginsAddTo.fPushBack( &gScriptFileConverter );

	// data table
	static tDataTableAssetPlugin gDataTableAssetPlugin;
	pluginsAddTo.fPushBack( &gDataTableAssetPlugin );

	// localization
	static tLocmlAssetPlugin gLocmlAssetPlugin;
	pluginsAddTo.fPushBack( &gLocmlAssetPlugin );

	// swfs (flash UI)
	static tSwfAssetPlugin gSwfAssetPlugin;
	pluginsAddTo.fPushBack( &gSwfAssetPlugin );

	// ttf ( True Type Font )
	static tTtfAssetPlugin gTtfAssetPlugin;
	pluginsAddTo.fPushBack( &gTtfAssetPlugin );
}

dll_export void fDestroyPlugins( const tAssetPluginDll::tPluginList& pluginsToFree )
{
}
