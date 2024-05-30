//------------------------------------------------------------------------------
// \file tNetUI.cpp - 31 May 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "GameAppPch.hpp"
#include "tNetUI.hpp"
#include "tGameApp.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateNetUI( "CanvasCreateNetUI" ); }

	//------------------------------------------------------------------------------
	tNetUI::tNetUI( )
		: tScriptedControl( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptNetUI) )
		, mLagUITimer( 0.f )
		, mLaggedUIActive( false )
		, mNetWaitUIActive( false )
		, mDesyncUIActive( false )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateNetUI, this );
		sigassert( !mCanvas.fIsNull( ) );
	}

	//------------------------------------------------------------------------------
	tNetUI::~tNetUI( )
	{

	}

	//------------------------------------------------------------------------------
	void tNetUI::fShowLagUI( f32 delta )
	{
		if( mLaggedUIActive )
			return;

		mLagUITimer += delta;

		if( mLagUITimer > 0.5f )
		{
			mLaggedUIActive = true;
			Sqrat::Function( mCanvas.fScriptObject( ), "ShowLagUI" ).Execute( );
		}
	}

	//------------------------------------------------------------------------------
	void tNetUI::fHideLagUI( )
	{
		mLagUITimer = 0.f;

		if( !mLaggedUIActive )
			return;
		
		mLaggedUIActive = false;
		Sqrat::Function( mCanvas.fScriptObject( ), "HideLagUI" ).Execute( );
	}

	//------------------------------------------------------------------------------
	void tNetUI::fShowNetWaitUI( )
	{
		if( mNetWaitUIActive )
			return;

		mNetWaitUIActive = true;

		Sqrat::Function( mCanvas.fScriptObject( ), "ShowNetWaitUI" ).Execute( );
	}

	//------------------------------------------------------------------------------
	void tNetUI::fHideNetWaitUI( )
	{
		if( !mNetWaitUIActive )
			return;

		mNetWaitUIActive = false;

		Sqrat::Function( mCanvas.fScriptObject( ), "HideNetWaitUI" ).Execute( );
	}

	//------------------------------------------------------------------------------
	void tNetUI::fShowDesyncUI( )
	{
		if( mDesyncUIActive )
			return;

		mDesyncUIActive = true;

		Sqrat::Function( mCanvas.fScriptObject( ), "ShowDesyncUI" ).Execute( );
	}

	//------------------------------------------------------------------------------
	void tNetUI::fHideDesyncUI( )
	{
		if( !mDesyncUIActive )
			return;

		mDesyncUIActive = false;

		Sqrat::Function( mCanvas.fScriptObject( ), "HideDesyncUI" ).Execute( );
	}

} }

namespace Sig { namespace Gui
{
	void tNetUI::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tNetUI, Sqrat::NoConstructor > classDesc( vm.fSq( ) );

		vm.fNamespace( _SC( "Gui" ) ).Bind( _SC( "NetUI" ), classDesc );
	}

} }
