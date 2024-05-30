#if defined( platform_ios )
#ifndef __tRenderToTexture_ios__
#define __tRenderToTexture_ios__

namespace Sig { namespace Gfx
{

	class tRenderToTexture : public tRenderToTexturePlatformBase
	{
		friend class tRenderToTexturePlatformBase;
		define_class_pool_new_delete( tRenderToTexture, 32 );
	private:
		tRenderToTexturePtr mDepthResolveTexture;
	public:
		tRenderToTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, tRenderTarget::tFormat depthFormat, u32 multiSamplePower, u32 numLayers = 1 );
		tRenderToTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, const tRenderTargetPtr& depthTarget, u32 multiSamplePower, u32 numLayers = 1 );
		tRenderToTexture( const tDevicePtr& device, const tRenderTargetPtr& renderTarget, const tRenderTargetPtr& depthTarget, const tRenderTarget::tFormat* textureFormatOverride = 0, u32 numLayers = 1 );
		~tRenderToTexture( );
		void fSetDepthResolveTexture( const tRenderToTexturePtr& rtt ) { mDepthResolveTexture = rtt; }
		void fApply( tScreen& screen, u32 index = 0 );
		void fResolve( tScreen& screen, void* targetOverride = 0, u32 slice = 0 );
	protected:
		void fCreateTextureAndRenderTarget( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, u32 multiSamplePower );
		void fCreateTextureFromRenderTarget( const tDevicePtr& device, const tRenderTargetPtr& renderTarget, const tRenderTarget::tFormat* textureFormatOverride = 0 );
		void fCreateTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, u32 numLayers );
		void fReleaseAll( );
	};

}}

#endif//__tRenderToTexture_ios__
#endif//#if defined( platform_ios )
