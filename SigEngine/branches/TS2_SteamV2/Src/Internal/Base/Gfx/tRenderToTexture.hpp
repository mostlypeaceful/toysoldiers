#ifndef __tRenderToTexture__
#define __tRenderToTexture__
#include "tRenderTarget.hpp"
#include "tTextureReference.hpp"

namespace Sig { namespace Gfx
{
	class tScreen;

	class tRenderToTexturePlatformBase : public tUncopyable, public tRefCounter, public tTextureReference
	{
	protected:
		tRenderTargetPtr mRt;
		tRenderTargetPtr mDt;
		const u32 mNumLayers;
	public:
		explicit tRenderToTexturePlatformBase( u32 numLayers );
		const tRenderTargetPtr& fRenderTarget( ) const { return mRt; }
		const tRenderTargetPtr& fDepthTarget( ) const { return mDt; }
		b32 fFailed( ) const { return mRt.fNull( ) || mRt->fFailed( ) || mDt.fNull( ) || mDt->fFailed( ); }
		void fApply( tScreen& screen, u32 index = 0 );
		void fSetClipBox( tScreen& screen, const Math::tVec4f& viewportTLBR ) const;
		u32 fWidth( ) const { return mRt.fNull( ) ? 0 : mRt->fWidth( ); }
		u32 fHeight( ) const { return mRt.fNull( ) ? 0 : mRt->fHeight( ); }
		u32 fLayerCount( ) const { return mNumLayers; } // i.e., greater than 1 means array texture
		b32 fCanRenderToSelf( ) const;
		b32 fResolveClearsTarget( ) const;
	};

	class tRenderToTexture;

	define_smart_ptr( base_export, tRefCounterPtr, tRenderToTexture );
}}



#if defined( platform_pcdx9 )
#	include "tRenderToTexture_pcdx9.hpp"
#elif defined( platform_xbox360 )
#	include "tRenderToTexture_xbox360.hpp"
#elif defined( platform_ios )
#	include "tRenderToTexture_ios.hpp"
#else
#	error Invalid platform for tRenderToTexture defined!
#endif

#endif//__tRenderToTexture__

