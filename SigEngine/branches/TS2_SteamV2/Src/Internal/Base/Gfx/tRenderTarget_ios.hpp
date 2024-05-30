#if defined( platform_ios )
#ifndef __tRenderTarget_ios__
#define __tRenderTarget_ios__

namespace Sig { namespace Gfx
{
	class tDevicePtr;

	class base_export tRenderTarget : public tRenderTargetPlatformBase
	{
		friend class tRenderTargetPlatformBase;
		define_class_pool_new_delete( tRenderTarget, 32 );
	public:
		tRenderTarget( const tDevicePtr& device, u32 width, u32 height, tFormat format, u32 multiSamplePower = 0, u32 edramSize = 0 );
		~tRenderTarget( );
		void fApply( const tDevicePtr& device, u32 rtIndex = 0 ) const;
	};

}}



#endif//__tRenderTarget_ios__
#endif//#if defined( platform_ios )

