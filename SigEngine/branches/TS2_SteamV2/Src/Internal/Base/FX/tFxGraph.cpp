#include "BasePch.hpp"
#include "tFxGraph.hpp"


namespace Sig
{
namespace FX
{
	
	register_rtti_factory( tFxGraphF32, false );
	register_rtti_factory( tFxGraphV2f, false );
	register_rtti_factory( tFxGraphV3f, false );
	register_rtti_factory( tFxGraphV4f, false );

	tBinaryGraph* fCreateNewGraph( tGraphPtr original )
	{
		u32 id = original->fGetID( );

		if( id == Rtti::fGetClassId< f32 >( ) )
			return NEW tBinaryF32Graph( );
		else if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
			return NEW tBinaryV2Graph( );
		else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
			return NEW tBinaryV3Graph( );
		else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
			return NEW tBinaryV4Graph( );
		return 0;
	}

}
}

