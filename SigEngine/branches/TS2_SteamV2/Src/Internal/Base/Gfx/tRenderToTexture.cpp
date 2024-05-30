#include "BasePch.hpp"
#include "tRenderToTexture.hpp"
#include "tScreen.hpp"

namespace Sig { namespace Gfx
{
	tRenderToTexturePlatformBase::tRenderToTexturePlatformBase( u32 numLayers )
		: mNumLayers( numLayers )
	{
		sigassert( mNumLayers > 0 );
	}

	void tRenderToTexturePlatformBase::fApply( tScreen& screen, u32 index )
	{
		sigassert( mRt && mDt && !mDt->fFailed( ) );
		mRt->fApply( screen.fGetDevice( ), index );
		mDt->fApply( screen.fGetDevice( ), index );

		screen.mCurrentTarget.fReset( static_cast<tRenderToTexture*>( this ) );
	}

	void tRenderToTexturePlatformBase::fSetClipBox( tScreen& screen, const Math::tVec4f& viewportTLBR ) const
	{
		screen.fSetViewportClipBox( 
			fRound<u32>( viewportTLBR.x * mRt->fWidth( ) ), 
			fRound<u32>( viewportTLBR.y * mRt->fHeight( ) ), 
			fRound<u32>( mRt->fWidth( ) * ( viewportTLBR.z - viewportTLBR.x ) ), 
			fRound<u32>( mRt->fHeight( ) * ( viewportTLBR.w - viewportTLBR.y ) ) );
	}

}}

