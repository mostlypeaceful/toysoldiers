#include "BasePch.hpp"
#include "tRenderToTextureAgent.hpp"
#include "tScreen.hpp"
#include "tSceneRefEntity.hpp"
#include "tApplication.hpp"

namespace Sig { namespace Gfx
{
	tRenderToTextureAgent::tRenderToTextureAgent( )
		: mOutTexture( 0 )
		, mRtt( 0 )
		, mClearColor( Math::tVec4f::cZeroVector )
	{ }

	tRenderToTextureAgent::~tRenderToTextureAgent( )
	{
		if( mLight )
		{
			mLight->fDelete( );
			mLight.fRelease( );
		}
		if( mTextureResource )
		{
			mTextureResource = tResourcePtr( );
		}
	}

	void tRenderToTextureAgent::fFromSigml( tScreen& screen, tEntity& sigml, const Math::tMat3f& camera, const tFilePathPtr& texture )
	{
		//setup root
		mRoot.fReset( &sigml );

		//create light
		tSceneRefEntity* sre = mRoot->fStaticCast<tSceneRefEntity>( );
		tSceneGraphFile* sgf = sre->fSgResource( )->fCast<tSceneGraphFile>( );
		mLight.fReset( tLightEntity::fCreateDefaultLight( screen, *sgf ) );

		//create output texture
		mTextureResource = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake<tTextureFile>( texture ) );
		sigassert( mTextureResource );
		mTextureResource->fLoadDefault( this );
		tTextureFile* texfile = mTextureResource->fCast<tTextureFile>( );
		sigassert( texfile );
		sigassert( texfile->mFormat == tTextureFile::cFormatA8R8G8B8 && "Format must match render target, or be Resolve compatible" );
		mOutTexture = texfile->fGetPlatformHandle( );

		//and finally make our tRenderToTexture obj
		const Gfx::tRenderToTexturePtr& rtt = screen.fScreenSpaceRenderToTexture( );
		const u32 multiSample = screen.fCreateOpts( ).mMultiSamplePower;
		mRtt.fReset( NEW tRenderToTexture( 
			screen.fGetDevice( ),
			texfile->mWidth, texfile->mHeight,
			rtt->fRenderTarget( )->fFormat( ),
			rtt->fDepthTarget( )->fFormat( ),
			multiSample ) );

		//setup our camera + other render shit
		const Math::tVec3f eye = camera.fGetTranslation( );
		const Math::tVec3f look = eye + camera.fZAxis( ).fNormalize( );
		const Math::tVec3f up = camera.fYAxis( ).fNormalize( );

		const f32 aspect = texfile->mWidth / (f32)texfile->mHeight;
		tLens lens;
		lens.fSetPerspective( 1.f, 1000.f, aspect, Math::cPiOver4 );

		mCamera = tCamera( lens, tTripod( eye, look, up ) );
	}
}}

