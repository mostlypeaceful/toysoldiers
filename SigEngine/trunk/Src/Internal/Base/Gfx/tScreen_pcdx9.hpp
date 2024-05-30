#if defined( platform_pcdx9 )
#ifndef __tScreen_pcdx9__
#define __tScreen_pcdx9__
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

		IDirect3DSwapChain9*	mSwapChain;
		D3DPRESENT_PARAMETERS	mLastPresentParams;
		IDirect3DTexture9*		mPlatformWhiteTexture;
		IDirect3DTexture9*		mPlatformBlackTexture;
		IDirect3DTexture9*		mCaptureTextureScene;

	public:

		tScreen( const tDevicePtr& device, const tSceneGraphPtr& sceneGraph, tScreenCreationOptions& createOpts );
		~tScreen( );

		void fRenderMiniMap( tRenderToTexture& tex, const Math::tAabbf& box );

		virtual void fOnDeviceLost( tDevice* device );
		virtual void fOnDeviceReset( tDevice* device );

		void fResize( u32 newBackBufferWidth, u32 newBackBufferHeight );

		void fBeginCaptureDump( const tFilePathPtr& folder );
		void fEndCaptureDump( );

	private:

		void fReleaseSwapChainResources( );
		void fCreateSwapChain( D3DPRESENT_PARAMETERS& d3dpp );
		void fCaptureFrame( const tFilePathPtr& directory, u32 frameNum, b32 saveAsPng = false ) const;
	};

}}

#endif//__tScreen_pcdx9__
#endif//#if defined( platform_pcdx9 )
