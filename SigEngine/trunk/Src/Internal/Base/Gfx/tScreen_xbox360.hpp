#if defined( platform_xbox360 )
#ifndef __tScreen_xbox360__
#define __tScreen_xbox360__
#ifndef __tScreen__
#error This file must be included from tScreen.hpp!
#endif//__tScreen__
#include "tDeviceResource.hpp"

namespace Sig { namespace Gfx
{
	struct tScreenCreationOptions;

	class base_export tScreen : public tScreenPlatformBase, public tDeviceResource
	{
		friend class tViewport;
		friend class tScreenPlatformBase;
	private:

		D3DPRESENT_PARAMETERS	mLastPresentParams;
		IDirect3DTexture9*		mPlatformWhiteTexture;
		IDirect3DTexture9*		mPlatformBlackTexture;
	    IDirect3DTexture9*		mFrontBufferTexture;
		tRenderToTexturePtr		mPostProcessDepthTexture;

	public:

		tScreen( const tDevicePtr& device, const tSceneGraphPtr& sceneGraph, tScreenCreationOptions& createOpts );
		~tScreen( );

		virtual void fOnDeviceLost( tDevice* device );
		virtual void fOnDeviceReset( tDevice* device );

		const tRenderToTexturePtr&  fPostProcessDepthTexture( ) const { return mPostProcessDepthTexture; }

		void fResize( u32 newBackBufferWidth, u32 newBackBufferHeight );

		void fRenderWorldOpaqueBegin( );
		void fRenderWorldOpaqueEnd( );

		void fBeginCaptureDump( const tFilePathPtr& folder );
		void fEndCaptureDump( );

	private:

		void fReleaseSwapChainResources( );
		void fCreateSwapChain( D3DPRESENT_PARAMETERS& d3dpp );
		void fCaptureFrame( const tFilePathPtr& directory, u32 frameNum, b32 saveAsPng = false ) const;
	};

}}

#endif//__tScreen_xbox360__
#endif//#if defined( platform_xbox360 )

