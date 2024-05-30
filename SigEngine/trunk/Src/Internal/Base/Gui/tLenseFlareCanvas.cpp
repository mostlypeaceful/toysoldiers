#include "BasePch.hpp"
#include "tLenseFlareCanvas.hpp"
#include "tTexturedQuad.hpp"
#include "tProjectFile.hpp"
#include "Gfx/tTextureFile.hpp"
#include "Gfx/tViewport.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gfx/tScreen.hpp"
#include "tSceneGraph.hpp"
#include "tGameAppBase.hpp"

using namespace Sig::Math;

namespace Sig { namespace Gui
{
	devvar( f32, Renderer_LensFlare_FadeRate, 4.0f );

	namespace
	{
		struct tLenseRayCastCallback
		{
		public:
			mutable Math::tRayCastHit	mHit;

			inline void operator()( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const
			{
				if( i->fQuickRejectByFlags( ) )
					return;

				tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i->fOwner( ) );
				if( i->fQuickRejectByBox( ray ) )
					return;

				Math::tRayCastHit hit;
				spatial->fRayCast( ray, hit );
				if( hit.fHit( ) && hit.mT < mHit.mT )
					mHit = hit;
			}
		};
	}

	tLenseFlareCanvas::tLenseFlareCanvas( )
		: mCurrentAlpha( 0.f )
	{
	}

	void tLenseFlareCanvas::fSetLenseFlare( u32 key )
	{
		const tProjectFile::tLenseFlare* lf = tProjectFile::fFindItemByKey( key, tProjectFile::fInstance( ).mEngineConfig.mRendererSettings.mLenseFlares );
		if( lf )
		{
			const Gfx::tLenseFlareData& data = lf->mData;
			mFlares.fSetCount( data.mFlares.fCount( ) );

			for( u32 i = 0; i < mFlares.fCount( ); ++i )
			{
				mFlares[ i ] = tFlare( data.mFlares[ i ] );
				mFlares[ i ].mTexture->fSetInvisible( false );
				fAddChild( mFlares[ i ].mTexturePtr );
			}
		}
		else
			log_warning( "Could not find lense flare with key: " << key );
	}

	void tLenseFlareCanvas::fOnTickCanvas( f32 dt ) 
	{ 
		if( !mTrackingEnt )
			return;

		const tVec3f lightWorldPos = mTrackingEnt->fObjectToWorld( ).fGetTranslation( );
		tVec3f screenPos;
		b32 onScreen = mViewport->fProjectToScreenClamped( lightWorldPos, screenPos );
		onScreen = onScreen && mViewport->fComputeRect( ).fContains( screenPos.fXY( ) );

		if( onScreen )
		{
			tRayf ray;
			ray.mOrigin = mViewport->fRenderCamera( ).fGetTripod( ).mEye;
			ray.mExtent = lightWorldPos - ray.mOrigin;

			tLenseRayCastCallback cb;
			mViewport->fScreen( ).fSceneGraph( )->fRayCastAgainstRenderable( ray, cb );
			if( cb.mHit.fHit( ) )
				onScreen = false;
		}

		const f32 targetAlpha = onScreen ? 1.f : 0.f;

#ifdef target_tools
		mCurrentAlpha = targetAlpha;
#else
		// fade in and out, as visible.
		const f32 cMaxChange = Renderer_LensFlare_FadeRate * dt;
		const f32 delta = fClamp( targetAlpha - mCurrentAlpha, -cMaxChange, cMaxChange );
		mCurrentAlpha += delta;
#endif

		fSetAlpha( mCurrentAlpha );

		if( mCurrentAlpha > 0.f )
		{
			fSetInvisible( false );

			tVec3f dir = fWorldPosition( ) - screenPos;
			dir.z = 0;

			for( u32 i = 0; i < mFlares.fCount( ); ++i )
			{
				tFlare& flare = mFlares[ i ];
				tVec3f halfSize( flare.mTexture->fTextureDimensions( ) * flare.mFlare.mScale * 0.5f, 0 );
				flare.mTexture->fSetPosition( dir * flare.mFlare.mPosition - halfSize );
			}
		}
		else
			fSetInvisible( true );
	}

	void tLenseFlareCanvas::fSetViewport( const Gfx::tViewport& vp )
	{
		mViewport.fReset( &vp );
		fSetPosition( tVec3f( mViewport->fComputeRect( ).fCenter( ), 0 ) );
	}


	tLenseFlareCanvas::tFlare::tFlare( const Gfx::tLenseFlareData::tFlare& flare )
		: mFlare( flare )
		, mTexture( NEW tTexturedQuad( Gfx::tDefaultAllocators::fInstance( ) ) )
		, mTexturePtr( mTexture )
	{
		sigassert( Gfx::tDefaultAllocators::fInstance( ).mResourceDepot );

		Gfx::tRenderState renderState = Gfx::tRenderState::cDefaultColorTransparent;
		renderState.fSetSrcBlendMode( Gfx::tRenderState::cBlendSrcAlpha );
		renderState.fSetDstBlendMode( Gfx::tRenderState::cBlendOne );
		mTexture->fSetRenderState( renderState );

		if( mFlare.mTexture.fLength( ) )
		{
#if target_tools
			Math::tVec2f dims;
			Gfx::tTextureReference tex = Gfx::tDefaultAllocators::fInstance( ).mResourceDepot->fToolsTextureQuery( mFlare.mTexture, dims );
			mTexture->fSetTexture( tex, dims );
#else
			mTexture->fSetTexture( Gfx::tDefaultAllocators::fInstance( ).mResourceDepot->fQuery( tResourceId::fMake<Gfx::tTextureFile>( mFlare.mTexture ) ) );
#endif
		}

		mTexture->fSetRgbaTint( mFlare.mColor );
		mTexture->fSetScale( mFlare.mScale );
	}

	tLenseFlareCanvas::tFlare::tFlare( )
	{
	}

}}

namespace Sig { namespace Gui
{
	void tLenseFlareCanvas::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tLenseFlareCanvas, tCanvasFrame, Sqrat::NoCopy<tLenseFlareCanvas> > classDesc( vm.fSq( ) );

		classDesc
			;

		vm.fNamespace(_SC("Gui")).Bind(_SC("LenseFlareCanvas"), classDesc);
	}
}}
 