#include "BasePch.hpp"
#include "tPluginMinimapData.hpp"

namespace Sig
{

	//------------------------------------------------------------------------------
	// tPluginMiniMapData
	//------------------------------------------------------------------------------
	tPluginMiniMapData::tPluginMiniMapData( )
		: mTexture( NULL )
	{
	}

	//------------------------------------------------------------------------------
	tPluginMiniMapData::tPluginMiniMapData( tNoOpTag )
		: tEntityData( cNoOpTag )
		, mWorldOrigin( cNoOpTag )
	{
	}

} //Sig
