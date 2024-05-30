#if defined( platform_ios )
#ifndef __tScreen_ios__
#define __tScreen_ios__
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
	public:

		tScreen( const tDevicePtr& device, const tSceneGraphPtr& sceneGraph, tScreenCreationOptions& createOpts );
		~tScreen( );
		virtual void fOnDeviceLost( tDevice* device ) { }
		virtual void fOnDeviceReset( tDevice* device ) { }
		void fResize( u32 newBackBufferWidth, u32 newBackBufferHeight );
		void fBeginCaptureDump( const tFilePathPtr& folder );
		void fEndCaptureDump( );
	private:
		void fReleaseSwapChainResources( );
		void fCaptureFrame( const tFilePathPtr& directory, u32 frameNum, b32 saveAsPng ) const;
	};

}}

#endif//__tScreen_ios__
#endif//#if defined( platform_ios )

