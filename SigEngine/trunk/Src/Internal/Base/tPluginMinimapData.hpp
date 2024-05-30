#ifndef __tPluginMinimapData__
#define __tPluginMinimapData__

#include "tEntityData.hpp"

namespace Sig
{
	/*
		Engine level plugin example.
	*/

	class base_export tPluginMiniMapData : public tEntityData
	{
		declare_reflector( );
		define_dynamic_cast( tPluginMiniMapData, tEntityData );
		implement_rtti_serializable_base_class( tPluginMiniMapData, 0xBE6247BC );
	public:

		tPluginMiniMapData( );
		tPluginMiniMapData( tNoOpTag );

		tLoadInPlaceResourcePtr*	mTexture;
		Math::tVec2f				mWorldOrigin; // In pixel coordinates.
		f32							mUnitScale;   // pixels / world unit.
	};

}

#endif//__tPluginMinimapData__
