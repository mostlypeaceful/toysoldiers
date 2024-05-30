#ifndef __tRenderToTexture__
#define __tRenderToTexture__
#include "tRenderTarget.hpp"
#include "tTextureReference.hpp"

namespace Sig { namespace Gfx
{
	class tScreen;

	class tRenderToTexture;
	define_smart_ptr( base_export, tRefCounterPtr, tRenderToTexture );

	class tRenderToTexturePlatformBase : public tUncopyable, public tRefCounter
	{
	public:
		static const u32 cMaxTargets = 4;
		
		// Each target may have a different format as long as their bit size matches.
		struct tFormat
		{
			tFixedGrowingArray< tRenderTarget::tFormat, cMaxTargets > mFormats;

			tFormat( tRenderTarget::tFormat format )		{ mFormats.fPushBack( format ); }

			// Push in new formats for multi render targets
			tFormat& fNext( tRenderTarget::tFormat format ) { mFormats.fPushBack( format ); return *this; }

			u32 fCount( ) const { return mFormats.fCount( ); }
		};

		explicit tRenderToTexturePlatformBase( u32 numLayers );
		
		const tRenderTargetPtr& fRenderTarget( u32 index = 0 ) const { return mRts[ index ]; }
		const tRenderTargetPtr& fDepthTarget( ) const { return mDt; }
		
		b32 fFailed( ) const { return fRenderTarget( ).fNull( ) || fRenderTarget( )->fFailed( ) || mDt.fNull( ) || mDt->fFailed( ); }
		
		void fSetClipBox( tScreen& screen, const Math::tVec4f& viewportTLBR ) const;
		
		u32 fWidth( ) const { return fRenderTarget( ).fNull( ) ? 0 : fRenderTarget( )->fWidth( ); }
		u32 fHeight( ) const { return fRenderTarget( ).fNull( ) ? 0 : fRenderTarget( )->fHeight( ); }
		u32 fLayerCount( ) const { return mNumLayers; } // i.e., greater than 1 means array texture
		u32 fTargetCount( ) const { return mRts.fCount( ); }
		b32 fCanRenderToSelf( ) const;
		b32 fResolveClearsTarget( ) const;

		void fSetDepthResolveTexture( const tRenderToTexturePtr& rtt ) { mDepthResolveTexture = rtt; }
		tTextureReference& fTexture( u32 index = 0 ) { return mTextures[ index ]; }

	protected:
		tRenderTargetPtr mDt;
		tFixedGrowingArray<tRenderTargetPtr, cMaxTargets> mRts;
		tFixedArray<tTextureReference, cMaxTargets> mTextures;

		tFixedArray< IDirect3DBaseTexture9*, cMaxTargets >  mRealTexture;
		tRenderToTexturePtr mDepthResolveTexture;

		const u32 mNumLayers;
	};
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

