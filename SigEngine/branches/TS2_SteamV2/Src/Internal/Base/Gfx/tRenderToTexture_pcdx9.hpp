#if defined( platform_pcdx9 )
#ifndef __tRenderToTexture_pcdx9__
#define __tRenderToTexture_pcdx9__

namespace Sig { namespace Gfx
{

	class tRenderToTexture : public tRenderToTexturePlatformBase
	{
		friend class tRenderToTexturePlatformBase;
		define_class_pool_new_delete( tRenderToTexture, 32 );
	private:
		IDirect3DTexture9* mTexture;
		IDirect3DSurface9* mSurface;
	public:
		tRenderToTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, tRenderTarget::tFormat depthFormat );
		tRenderToTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, const tRenderTargetPtr& depthTarget );
		tRenderToTexture( const tDevicePtr& device, const tRenderTargetPtr& renderTarget, const tRenderTargetPtr& depthTarget );
		~tRenderToTexture( );
		void fResolve( tScreen& screen, IDirect3DTexture9* targetOverride = 0, u32 slice = 0 );
		IDirect3DBaseTexture9* fGetTexture( ) const { return mTexture; }
		IDirect3DSurface9* fGetSurface( ) const { return mSurface; }
	protected:
		void fCreateTextureAndRenderTarget( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format );
		void fCreateTextureFromRenderTarget( const tDevicePtr& device, const tRenderTargetPtr& renderTarget );
		void fCreateTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format );
		void fReleaseAll( );
	};

}}

#endif//__tRenderToTexture_pcdx9__
#endif//#if defined( platform_pcdx9 )
