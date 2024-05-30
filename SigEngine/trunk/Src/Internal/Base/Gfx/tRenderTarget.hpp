#ifndef __tRenderTarget__
#define __tRenderTarget__

namespace Sig { namespace Gfx
{
	class base_export tRenderTargetPlatformBase : public tUncopyable, public tRefCounter
	{
	public:
		// All formats are 32 bit unless noted
		enum tFormat
		{
			cFormatNull,
			cFormatXRGB8,
			cFormatRGBA8,
			cFormatRGBA16F, //64 bit format
			cFormatR32F,
			cFormatRG16F,

			cFormatDepthFormatsBegin = 512,
			cFormatD24S8 = cFormatDepthFormatsBegin,
			cFormatD24FS8,
		};
	private:
		tFormat mFormat;
		u32 mWidth;
		u32 mHeight;
		u32 mMultiSamplePower;
		b32 mIsDepth;

	public:
		tRenderTargetPlatformBase( u32 width, u32 height, b32 isDepth, tFormat format, u32 multiSamplePower );

		tFormat fFormat( ) const { return mFormat; }
		u32 fWidth( ) const { return mWidth; }
		u32 fHeight( ) const { return mHeight; }
		u32 fMultiSamplePower( ) const { return mMultiSamplePower; }
		b32 fIsDepthTarget( ) const { return mIsDepth; }
		b32 fFailed( ) const;
	};

	class tRenderTarget;

	define_smart_ptr( base_export, tRefCounterPtr, tRenderTarget );
}}



#if defined( platform_pcdx9 )
#	include "tRenderTarget_pcdx9.hpp"
#elif defined( platform_pcdx10 )
#	include "tRenderTarget_pcdx10.hpp" // N.B. doesn't exist at time of writing
#elif defined( platform_pcdx11 ) || defined( platform_metro )
#	include "tRenderTarget_dx11.hpp"
#elif defined( platform_xbox360 )
#	include "tRenderTarget_xbox360.hpp"
#elif defined( platform_ios )
#	include "tRenderTarget_ios.hpp"
#else
#	error Invalid platform for tRenderTarget defined!
#endif

#endif//__tRenderTarget__

