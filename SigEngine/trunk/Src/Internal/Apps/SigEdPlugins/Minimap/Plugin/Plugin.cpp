#include "SigEdPch.hpp"
#include "tAssetPluginDll.hpp"
#include "tEditableMiniMapProperties.hpp"

using namespace Sig;

dll_export u32 fGetPluginVersion( )
{
	return asset_plugin_version;
}

dll_export void fCreatePlugins( tAssetPluginDll::tPluginList& pluginsAddTo )
{
    static tMiniMapEditorPlugin gPlugin;
    pluginsAddTo.fPushBack( &gPlugin );
}

dll_export void fDestroyPlugins( const tAssetPluginDll::tPluginList& pluginsToFree )
{
}
