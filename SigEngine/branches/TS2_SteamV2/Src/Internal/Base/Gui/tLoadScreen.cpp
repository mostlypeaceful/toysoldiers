//------------------------------------------------------------------------------
// \file tLoadScreen.cpp - 10 Aug 2010
// \author Josh Wittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tLoadScreen.hpp"
#include "tApplication.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateLoadScreen( "CanvasCreateLoadScreen" ); }

	//------------------------------------------------------------------------------
	tLoadScreen::tLoadScreen( const tResourcePtr& scriptResource )
		: tScriptedControl( scriptResource )
		, mState( cStateLoading )
		, mCanvasIsFadedIn( false )
		, mCanvasIsFadedOut( false )
		, mNewLevelIsLoaded( false )
		, mCanProceedToNewLevel( false )
		, mCanSpawn( false )
	{
	}

	//------------------------------------------------------------------------------
	void tLoadScreen::fBegin( )
	{
		sigassert( mState == cStateLoading );
		mState = cStateBeginning;
		if( !fCreateControlFromScript( cCanvasCreateLoadScreen, this ) )
			return;

		fAttachCanvasToFrame( tApplication::fInstance( ).fRootCanvas( ) );
	}

	//------------------------------------------------------------------------------
	b32 tLoadScreen::fTryAdvanceToPlaying( ) const
	{
		sigassert( mState == cStateBeginning );

		if( !mScriptResource )
		{
			mState = cStatePlaying;
			return true;
		}

		if( !mCanvasIsFadedIn )
			return false;

		mState = cStatePlaying;
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tLoadScreen::fCanProceedToNewLevel( ) const
	{
		sigassert( mState == cStatePlaying );

		if( !mScriptResource )
			return true;

		Sqrat::Function canProceedFunc( fCanvas( ).fScriptObject( ), "CanProceedToNewLevel" );
		if( !canProceedFunc.IsNull( ) )
			return canProceedFunc.Evaluate< bool >( );

		return true;
	}

	//------------------------------------------------------------------------------
	void tLoadScreen::fEnd( )
	{
		sigassert( mState == cStatePlaying );
		mState = cStateEnding;
	}

	//------------------------------------------------------------------------------
	b32 tLoadScreen::fIsComplete( ) const
	{
		if( !mScriptResource )
			return true;

		return mCanvasIsFadedOut;
	}
}}

namespace Sig { namespace Gui
{
	//------------------------------------------------------------------------------
	void tLoadScreen::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tLoadScreen,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Func( _SC("SetCanvasIsFadedIn"), &tLoadScreen::fSetCanvasIsFadedIn )
			.Func( _SC("SetCanvasIsFadedOut"), &tLoadScreen::fSetCanvasIsFadedOut )
			.Func( _SC("SetCanProceedToNewLevel"), &tLoadScreen::fSetCanProceedToNewLevel )
			.Func( _SC("NewLevelIsLoaded"), &tLoadScreen::fNewLevelIsLoaded )
			.Prop( _SC("CustomData"), &tLoadScreen::fCustomDataFromScript )
			.Func( _SC("CanSpawn"), &tLoadScreen::fCanSpawn )
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("LoadScreen"), classDesc );
	}
}}


