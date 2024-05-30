#include "BasePch.hpp"
#include "tRenderToTexture.hpp"
#include "tScreen.hpp"

namespace Sig { namespace Gfx
{
	tRenderToTexturePlatformBase::tRenderToTexturePlatformBase( u32 numLayers )
		: mNumLayers( numLayers )
	{
		mRealTexture.fFill( NULL );
		sigassert( mNumLayers > 0 );
	}

	void tRenderToTexturePlatformBase::fSetClipBox( tScreen& screen, const Math::tVec4f& viewportTLBR ) const
	{
		const tRenderTarget& rt = *fRenderTarget( 0 );
		screen.fSetViewportClipBox( 
			fRound<u32>( viewportTLBR.x * rt.fWidth( ) ), 
			fRound<u32>( viewportTLBR.y * rt.fHeight( ) ), 
			fRound<u32>( rt.fWidth( ) * ( viewportTLBR.z - viewportTLBR.x ) ), 
			fRound<u32>( rt.fHeight( ) * ( viewportTLBR.w - viewportTLBR.y ) ) );
	}

}}

