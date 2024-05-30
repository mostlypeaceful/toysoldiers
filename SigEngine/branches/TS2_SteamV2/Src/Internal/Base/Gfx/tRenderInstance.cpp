#include "BasePch.hpp"
#include "tRenderInstance.hpp"

namespace Sig { namespace Gfx
{
	tRenderInstance::tRenderInstance( )
		: mRgbaTint( Math::tVec4f::cOnesVector )
		, mRI_ObjectToWorld( &Math::tMat3f::cIdentity )
	{
	}

}}
