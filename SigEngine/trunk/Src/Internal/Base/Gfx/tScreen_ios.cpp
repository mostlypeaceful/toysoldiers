#include "BasePch.hpp"
#if defined( platform_ios )
#include "tScreen.hpp"

namespace Sig { namespace Gfx
{

	tScreen::tScreen( const tDevicePtr& device, const tSceneGraphPtr& sceneGraph, tScreenCreationOptions& createOpts )
		: tScreenPlatformBase( device, sceneGraph, createOpts )
	{
	}

	tScreen::~tScreen( )
	{
	}

	void tScreen::fResize( u32 newBackBufferWidth, u32 newBackBufferHeight )
	{
	}

	void tScreenPlatformBase::fSetViewportClipBox( u32 x, u32 y, u32 width, u32 height, f32 minZ, f32 maxZ )
	{
	}

	Math::tVec2u tScreenPlatformBase::fComputeGuiSafeEdge( ) const
	{
		const u32 tenPercentW = fRound<u32>( 0.05f * ( f32 )mCreateOpts.mBackBufferWidth );
		const u32 tenPercentH = fRound<u32>( 0.05f * ( f32 )mCreateOpts.mBackBufferHeight );
		return Math::tVec2u( tenPercentW, tenPercentH );
	}

	b32 tScreenPlatformBase::fBeginAllRendering( )
	{
		return false;
	}

	void tScreenPlatformBase::fEndAllRendering( )
	{
	}

	void tScreenPlatformBase::fClearCurrentRenderTargets( b32 clearColor, b32 clearDepth, const Math::tVec4f& rgbaClear, f32 zClear, u32 stencilClear )
	{
	}

	void tScreenPlatformBase::fDisableMSAA( )
	{
	}

	void tScreenPlatformBase::fReEnableMSAA( )
	{
	}

	void tScreen::fBeginCaptureDump( const tFilePathPtr& folder )
	{
		// unsupported on ios
	}

	void tScreen::fEndCaptureDump( )
	{
		// unsupported on ios
	}

	void tScreen::fReleaseSwapChainResources( )
	{
	}

	void tScreen::fCaptureFrame( const tFilePathPtr& directory, u32 frameNum, b32 saveAsPng ) const
	{
		// unsupported on ios
	}

}}
#endif//#if defined( platform_ios )

