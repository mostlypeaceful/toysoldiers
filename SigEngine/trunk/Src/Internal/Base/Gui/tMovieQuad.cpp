#include "BasePch.hpp"
#include "tMovieQuad.hpp"
#include "Gfx/tDevice.hpp"
#include "Gfx/tScreen.hpp"
#include "Audio/tSystem.hpp"


namespace Sig { namespace Gui
{
	tMovieQuad::tMovieQuad( )
		: mFinished( false )
	{
		fCommonCtor( );
	}

	void tMovieQuad::fCommonCtor( )
	{
		mFinished = false;
		fSetPositionAndSize( );
	}

	tMovieQuad::~tMovieQuad( )
	{
		mPlayer->fCancel( );
	}

	void tMovieQuad::fResetDeviceObjects( Gfx::tDevice& device )
	{
		fRegisterWithDevice( &device );
	}

	void tMovieQuad::fSetMovie( const tFilePathPtr& moviePath, b32 looping )
	{
		if( mPlayer )
			mPlayer->fCancel( );

		Gfx::tDevice& gfxDevice = *Gfx::tDevice::fGetDefaultDevice( );
		Audio::tSystem& audioSystem = *Audio::tSystem::fGetDefaultSystem( );
		fResetDeviceObjects( gfxDevice );
		mPlayer.fReset( NEW tMoviePlayer( gfxDevice, audioSystem, moviePath, 0, 0, 100, 100, looping, false ) );
		fSetPositionAndSize( );
	}

	void tMovieQuad::fSetRect( f32 w, f32 h )
	{
		fSetRect( Math::tVec2f( w, h ) );
	}

	void tMovieQuad::fSetRect( const Math::tVec2f& widthHeight )
	{
		fSetBounds( Math::tRect( Math::tVec2f::cZeroVector, widthHeight ) );
		fSetPositionAndSize( );
	}

	void tMovieQuad::fPause( b32 pause )
	{
		mPlayer->fPause( pause );
	}

	b32 tMovieQuad::fPaused( ) const
	{
		return mPlayer->fPaused( );
	}

	b32 tMovieQuad::fFinished( ) const
	{
		return mPlayer->fFinishedPlaying( );
	}

	void tMovieQuad::fOnMoved( )
	{
		tRenderableCanvas::fOnMoved( );
		fSetPositionAndSize( );
	}

	void tMovieQuad::fOnParentMoved( )
	{
		tRenderableCanvas::fOnParentMoved( );
		fSetPositionAndSize( );
	}

	void tMovieQuad::fSetPositionAndSize( )
	{
		const Math::tVec3f screenPos = fCalculateScreenPosition( );
		const Math::tVec2f size = fLocalRect( ).fWidthHeight( );
		fRefreshVideoRect( fRound<u32>( screenPos.x ), fRound<u32>( screenPos.y ), fRound<u32>( size.x ), fRound<u32>( size.y ) );
	}

	void tMovieQuad::fRefreshVideoRect( u32 l, u32 t, u32 w, u32 h )
	{
		if( mPlayer )
			mPlayer->fRefreshVideoRect( t, l, t + h, l + w );
	}

	void tMovieQuad::fOnRenderCanvas( Gfx::tScreen& screen ) const
	{
		const b32 vpIsVirtual = false;//( !fUser( ) || !fUser( )->fViewport( )->fIsVirtual( ) );
		const b32 disabled = fDisabled( ) || fInvisible( );
		if( !disabled && !vpIsVirtual )
			screen.fQueueMoviePlayer( mPlayer );
	}

	void tMovieQuad::fOnTickCanvas( f32 dt )
	{
		if( mPlayer && !mFinished && !mOnFinished.IsNull( ) && mPlayer->fFinishedPlaying( ) )
		{
			mFinished = true;
			Sqrat::Function f = mOnFinished; //keep a reference to the function object so script is safe to call fClearOnFinished( )
			f.Execute( );
		}
		tRenderableCanvas::fOnTickCanvas( dt );
	}

	void tMovieQuad::fClearOnFinished( )
	{
		//This is required to prevent Sqrat from holding onto a hard-ref to our squirrel class.
		// And this is especially problematic if a circular ref is formed (which was the case
		//  so that's why this function was written.)
		mOnFinished = Sqrat::Function( );
	}

}}

namespace Sig { namespace Gui
{
	void tMovieQuad::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tMovieQuad, tRenderableCanvas, Sqrat::NoCopy<tMovieQuad> > classDesc( vm.fSq( ) );

		classDesc
			.Overload<void (tMovieQuad::*)(const Math::tVec2f&)>( _SC( "SetRect" ),	&tMovieQuad::fSetRect )
			.Overload<void (tMovieQuad::*)(f32,f32)>( _SC( "SetRect" ),				&tMovieQuad::fSetRect )
			.Func( _SC( "RefreshVideoRect" ),										&tMovieQuad::fRefreshVideoRect )
			.Func( _SC( "SetMovie" ),												&tMovieQuad::fSetMovie)
			.Func( _SC( "Pause" ),													&tMovieQuad::fPause)
			.Var( _SC( "OnFinished" ),												&tMovieQuad::mOnFinished)
			.Func(_SC("ClearOnFinished"),											&tMovieQuad::fClearOnFinished)
			;

		vm.fNamespace(_SC("Gui")).Bind(_SC("MovieQuad"), classDesc);
	}
}}
