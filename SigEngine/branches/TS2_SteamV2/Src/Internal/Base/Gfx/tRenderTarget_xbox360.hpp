#if defined( platform_xbox360 )
#ifndef __tRenderTarget_xbox360__
#define __tRenderTarget_xbox360__

namespace Sig { namespace Gfx
{
	class tDevicePtr;

	class base_export tRenderTarget : public tRenderTargetPlatformBase
	{
		friend class tRenderTargetPlatformBase;
		define_class_pool_new_delete( tRenderTarget, 32 );
	private:
		IDirect3DSurface9* mSurface;
	public:
		static D3DFORMAT fConvertFormatType( tFormat format );
	public:
		tRenderTarget( const tDevicePtr& device, u32 width, u32 height, tFormat format, u32 multiSamplePower = 0, u32 edramSize = 0 );
		tRenderTarget( IDirect3DSurface9* rt, u32 width, u32 height, tFormat format, u32 multiSamplePower = 0 );
		~tRenderTarget( );
		u32 fComputeEdramSize( ) const;
		void fApply( const tDevicePtr& device, u32 rtIndex = 0 ) const;
		IDirect3DSurface9* fGetSurface( ) const { return mSurface; }
	};

}}



#endif//__tRenderTarget_xbox360__
#endif//#if defined( platform_xbox360 )

