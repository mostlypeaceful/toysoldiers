#if defined( platform_xbox360 )
#ifndef __tRenderTarget_xbox360__
#define __tRenderTarget_xbox360__

#include "tEdramTilingDesc.hpp"

namespace Sig { namespace Gfx
{
	class tDevicePtr;

	class base_export tRenderTarget : public tRenderTargetPlatformBase
	{
		friend class tRenderTargetPlatformBase;
		define_class_pool_new_delete( tRenderTarget, 32 );
	private:
		IDirect3DSurface9*	mSurface;
		tEdramTilingDesc	mTiling;

		void fChooseTiling( u32 numTargets );
	public:
		static D3DFORMAT fConvertFormatType( tFormat format );
		static u32 fFormatBitsPerPixel( tFormat format );
	public:
		// Num targets is just to choose the correct tiling size
		tRenderTarget( const tDevicePtr& device, u32 width, u32 height, b32 isDepth, tFormat format, const tEdramTilingDesc& tiling, u32 multiSamplePower = 0, u32 edramStartPos = 0 );
		~tRenderTarget( );

		const tEdramTilingDesc& fTiling( ) const { return mTiling; }
		u32 fComputeEdramSize( ) const;

		void fApply( const tDevicePtr& device, u32 rtIndex ) const;
		static void fReset( const tDevicePtr& device, u32 rtIndex = 0 );
		IDirect3DSurface9* fGetSurface( ) const { return mSurface; }
	};

}}



#endif//__tRenderTarget_xbox360__
#endif//#if defined( platform_xbox360 )

