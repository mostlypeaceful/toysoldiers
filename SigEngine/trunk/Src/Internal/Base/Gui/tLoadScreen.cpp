//------------------------------------------------------------------------------
// \file tLoadScreen.cpp - 10 Aug 2010
// \author Josh Wittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tLoadScreen.hpp"
#include "tApplication.hpp"
#include "tSwfFile.hpp"
#include "tGameAppBase.hpp"
#include "tFUIDataStore.hpp"

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
	{
	}

	//------------------------------------------------------------------------------
	void tLoadScreen::fBegin( )
	{
		sigassert( mState == cStateLoading );
		mState = cStateBeginning;

		if( !fCreateControlFromScript( cCanvasCreateLoadScreen, this ) )
			return;

		fAttachCanvasToFrame( tGameAppBase::fInstance( ).fStatusCanvas( ).fToCanvasFrame( ) ); // Always tick that shit
	}

	//------------------------------------------------------------------------------
	b32 tLoadScreen::fTryAdvanceToPlaying( )
	{
		sigassert( mState == cStateBeginning );

		if( !mScriptResource || mCanvasIsFadedIn )
		{
			mState = cStatePlaying;
			return true;
		}
		else
			return false;
	}

	//------------------------------------------------------------------------------
	b32 tLoadScreen::fCanProceedToNewLevel( ) const
	{
		sigassert( mState == cStatePlaying );

		if( !mScriptResource )
			return true;
		
		return mCanProceedToNewLevel;
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
		// Squirrel
		{
			Sqrat::Class<tLoadScreen,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
			classDesc
				.Func( _SC("SetCanvasIsFadedIn"), &tLoadScreen::fSetCanvasIsFadedIn )
				.Func( _SC("SetCanvasIsFadedOut"), &tLoadScreen::fSetCanvasIsFadedOut )
				.Func( _SC("SetCanProceedToNewLevel"), &tLoadScreen::fSetCanProceedToNewLevel )
				.Func( _SC("NewLevelIsLoaded"), &tLoadScreen::fNewLevelIsLoaded )
				.Prop( _SC("CustomData"), &tLoadScreen::fCustomDataFromScript )
				;
			vm.fNamespace(_SC("Gui")).Bind( _SC("LoadScreen"), classDesc );
		}

		// Fui
		{
			FUIRat::tDataProvider< tLoadScreen > bind;
			bind
				.Func( "SetCanvasIsFadedIn",		&tLoadScreen::fSetCanvasIsFadedInForFui )
				.Func( "SetCanvasIsFadedOut",		&tLoadScreen::fSetCanvasIsFadedOutForFui )
				.Func( "SetCanProceedToNewLevel",	&tLoadScreen::fSetCanProceedToNewLevelForFui )
				.Func( "NewLevelIsLoaded",			&tLoadScreen::fNewLevelIsLoaded )
				;
			bind.fRegister( );
		}
	}
}}


