#include "BasePch.hpp"
#include "tApplication.hpp"
#include "tScreenSpaceFxSystem.hpp"
#include "Gfx/tDevice.hpp"
#include "Gfx/tScreen.hpp"

namespace Sig { namespace Gui
{

	tScreenSpaceFxSystem::tScreenSpaceFxSystem( )
		: mDelay( -1.f )
	{
		fCommonCtor( );
	}

	tScreenSpaceFxSystem::~tScreenSpaceFxSystem( )
	{

	}

	void tScreenSpaceFxSystem::fCommonCtor( )
	{
		Gfx::tDevice& gfxDevice = *Gfx::tDevice::fGetDefaultDevice( );
		fResetDeviceObjects( gfxDevice );
	}

	void tScreenSpaceFxSystem::fOnTickCanvas( f32 dt )
	{
		mDelay -= dt;
		if( mDelay < 0.f )
		{
			fSetPaused( false );
			mDelay = Math::cInfinity;
		}

		if( mFxSystem && mFxSystem->fReadyForDeletion( ) )
			fDeleteSelf( );

		tCanvas::fOnTickCanvas( dt );
	}

	void tScreenSpaceFxSystem::fAddDrawCalls( Gfx::tScreen& screen, tEntity* entity ) const
	{
		Gfx::tRenderableEntity* renderable = entity->fDynamicCast< Gfx::tRenderableEntity >( );
		if( renderable )
			screen.fAddScreenSpaceDrawCall( renderable->fGetDrawCall( mFxSystem->fObjectToWorld( ).fGetTranslation( ).z ) );
		
		for( u32 i = 0; i < entity->fChildCount( ); ++i )
			fAddDrawCalls( screen, entity->fChild( i ).fGetRawPtr( ) );
	}


	void tScreenSpaceFxSystem::fOnRenderCanvas( Gfx::tScreen& screen ) const
	{
		fAddDrawCalls( screen, mFxSystem.fGetRawPtr( ) );
	}

	void tScreenSpaceFxSystem::fResetDeviceObjects( Gfx::tDevice& device )
	{
		fRegisterWithDevice( &device );
	}	

	void tScreenSpaceFxSystem::fSetPosition( f32 x, f32 y, f32 z )
	{
		if( mFxSystem )
		{
			Math::tMat3f xform = Math::tMat3f::cIdentity;
			xform.fXAxis( 1.f * Math::tVec3f::cXAxis );
			xform.fYAxis( -1.f * Math::tVec3f::cYAxis );
			xform.fZAxis( 1.f * Math::tVec3f::cZAxis );
			xform.fSetTranslation( Math::tVec3f( x, y, z ) );
			mFxSystem->fMoveTo( xform );
		}
	}

	void tScreenSpaceFxSystem::fSetSystem( const tFilePathPtr& path, s32 playcount, b32 local )
	{
		mFxSystem.fReset( tApplication::fInstance( ).fSceneGraph( )->fRootEntity( ).fSpawnFxChild( path, playcount, local ) );
		mFxSystem->fSetAsScreenSpaceParticleSystem( true );
		mFxSystem->fSetInvisible( true );
		fSetPaused( true );
	}

	void tScreenSpaceFxSystem::fSetPaused( b32 paused )
	{
		if( mFxSystem )
			mFxSystem->fPause( paused );
	}
	
	void tScreenSpaceFxSystem::fFadeOutSystems( f32 fadeOutTime )
	{
		if( mFxSystem )
			mFxSystem->fFadeEmissionRateOutOverTime( fadeOutTime );
	}

	void tScreenSpaceFxSystem::fOverrideSystemAlphas( f32 newAlpha )
	{
		if( mFxSystem )
			mFxSystem->fOverrideSystemAlpha( newAlpha );
	}

	void tScreenSpaceFxSystem::fFastForward( f32 amount )
	{
		if( mFxSystem )
			mFxSystem->fFastForward( amount );
	}

	void tScreenSpaceFxSystem::fSetEmissionRates( f32 rate )
	{
		if( mFxSystem )
			mFxSystem->fSetEmissionPercent( rate );
	}
}}


namespace Sig { namespace Gui
{

	void tScreenSpaceFxSystem::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tScreenSpaceFxSystem, tCanvas, Sqrat::NoCopy<tScreenSpaceFxSystem> > classDesc( vm.fSq( ) );

		classDesc
			.Func( _SC( "SetSystem" ),															&tScreenSpaceFxSystem::fSetSystem)
			.Func( _SC( "SetPaused" ),															&tScreenSpaceFxSystem::fSetPaused)
			.Func( _SC( "SetPosition" ),														&tScreenSpaceFxSystem::fSetPosition)
			.Func( _SC( "SetDelay" ),															&tScreenSpaceFxSystem::fSetDelay)
			.Func( _SC( "FadeOutSystems" ),														&tScreenSpaceFxSystem::fFadeOutSystems)
			.Func( _SC( "OverrideSystemAlphas" ),												&tScreenSpaceFxSystem::fOverrideSystemAlphas)
			.Func( _SC( "FastForward" ),														&tScreenSpaceFxSystem::fFastForward)
			.Func( _SC( "SetEmissionRates" ),													&tScreenSpaceFxSystem::fSetEmissionRates)
			;

		vm.fNamespace(_SC("Gui")).Bind(_SC("ScreenSpaceFxSystem"), classDesc);
	}
}}

