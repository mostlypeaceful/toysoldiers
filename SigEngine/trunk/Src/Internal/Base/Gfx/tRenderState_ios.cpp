#include "BasePch.hpp"
#if defined( platform_ios )
#include "tRenderState.hpp"
#include "tDevice.hpp"
#include "tRenderContext.hpp"

namespace Sig { namespace Gfx
{
	devvar( bool, Renderer_Shadows_FlipFaces, false );

	namespace
	{
		inline b32 fCompareStateToLastApplied( u32 flags, const tRenderState& rs, const tRenderState& lastApplied, b32 lastStateInvalid )
		{
			return !lastStateInvalid && ( lastApplied.fQuery( flags ) == rs.fQuery( flags ) );
		}
	}
	
	void tRenderState::fEnableDisableColorWrites( const tDevicePtr& device, b32 enable )
	{

	}

	void tRenderState::fApplyInternal( const tDevicePtr& device, const tRenderContext& context ) const
	{

	}

}}
#endif//#if defined( platform_ios )

