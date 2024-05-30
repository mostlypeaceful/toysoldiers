#if defined( platform_pcdx9 )
#ifndef __tRenderToTexture_pcdx9__
#define __tRenderToTexture_pcdx9__

namespace Sig { namespace Gfx
{

	class base_export tRenderToTexture : public tRenderToTexturePlatformBase
	{
		friend class tRenderToTexturePlatformBase;
		define_class_pool_new_delete( tRenderToTexture, 32 );
	private:
		tFixedArray< IDirect3DSurface9*, cMaxTargets >  mSurfaces;
		tFixedArray< IDirect3DVolumeTexture9*, cMaxTargets > m3DTextures; //optional, if we need slices, have to actually return a volume for PCDx9
	public:
		tRenderToTexture( const tDevicePtr& device, u32 width, u32 height, const tFormat& format, tRenderTarget::tFormat depthFormat, u32 multiSamplePower, u32 numLayers = 1 );
		tRenderToTexture( const tDevicePtr& device, u32 width, u32 height, tRenderTarget::tFormat format, const tRenderTargetPtr& depthTarget, u32 multiSamplePower, u32 numLayers = 1 );
		tRenderToTexture( const tDevicePtr& device, const tRenderTargetPtr& renderTarget, const tRenderTargetPtr& depthTarget, u32 multiSamplePower, u32 numLayers = 1 );
		~tRenderToTexture( );

		void fApply( tScreen& screen );
		void fResolve( tScreen& screen, IDirect3DTexture9* targetOverride = 0, u32 slice = 0, b32 unApplyExtras = true );

		void fApplyDepthOnly( tScreen& screen );
		void fEndDepthOnly( tScreen& screen );
		void fResolveDepth( tScreen& screen, u32 slice = 0 );

	protected:
		void fCreateTextureAndRenderTarget( const tDevicePtr& device, u32 width, u32 height, const tFormat& format, u32 multiSamplePower, u32 numLayers );
		void fCreateTextureFromRenderTarget( const tDevicePtr& device, u32 index, u32 numLayers );
		void fCreateTexture( const tDevicePtr& device, u32 index, u32 width, u32 height, tRenderTarget::tFormat format, u32 numLayers );
		void fReleaseAll( );
	};

}}

#endif//__tRenderToTexture_pcdx9__
#endif//#if defined( platform_pcdx9 )
