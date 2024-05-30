#include "BasePch.hpp"
#include "tSupportMapping.hpp"

using namespace Sig::Math;

namespace Sig { namespace Physics
{

#ifdef sig_devmenu
	tFixedArray< u32, cSupportCount > tSupportMapping::gCounts;
#endif

	tSupportMapping::tSupportMapping( f32 extraRadius, u32 type )
		: mWorldXform( Math::tMat3f::cIdentity )
		, mExtraRadius( extraRadius )
	{
		if_devmenu( mType = type; ++gCounts[ type ]; )
	}

	tSupportMapping::~tSupportMapping( )
	{ 
		if_devmenu( sigassert( gCounts[ mType ] ); --gCounts[ mType ]; )
	}

} }