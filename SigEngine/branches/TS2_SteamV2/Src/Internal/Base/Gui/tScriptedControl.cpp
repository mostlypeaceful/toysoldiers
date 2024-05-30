//------------------------------------------------------------------------------
// \file tScriptedControl.cpp - 10 Aug 2010
// \author Max Wagner
// \author Josh Wittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tScriptedControl.hpp"


namespace Sig { namespace Gui
{
	//------------------------------------------------------------------------------
	tScriptedControl::tScriptedControl( const tResourcePtr& scriptResource )
		: mScriptResource( scriptResource )
		, mLetCanvasPersist( false )
	{
		if( mScriptResource )
			mScriptResource->fLoadDefault( this );
	}

	//------------------------------------------------------------------------------
	tScriptedControl::~tScriptedControl( )
	{
		if( !mLetCanvasPersist )
			mCanvas.fDeleteSelf( );
		if( mScriptResource )
			mScriptResource->fUnload( this );
	}

	//------------------------------------------------------------------------------
	void tScriptedControl::fAttachCanvasToFrame( Gui::tCanvasFrame& frame )
	{
		if( mCanvas.fIsNull( ) )
			return;

		frame.fAddChild( mCanvas );
	}

	//------------------------------------------------------------------------------
	b32 tScriptedControl::fQueryEntryPoint( 
		const tStringPtr& entryPointName, 
		Sqrat::Function & entryPoint )
	{
		for( ;; )
		{
			// No script, break
			if( !mScriptResource )
				break;
			
			// Failed script, break
			if( mScriptResource->fLoadFailed( ) )
				break;

			// Find the entry func
			entryPoint = mScriptResource->fCast< tScriptFile >( )->fFindExportedFunction( entryPointName );
			
			// Invalid entry point, break
			if ( entryPoint.IsNull( ) )
			{
				log_warning( Log::cFlagScript, "Script file [" 
								 << mScriptResource->fGetPath( ) 
								 << "] doesn't contain entry point [" 
								 << entryPointName );
				break;
			}

			// Success!
			return true;
		}

		// Fail!
		return false;
	}

	//------------------------------------------------------------------------------
	void tScriptedControl::fUnloadAndReleaseScript( )
	{
		if( !mScriptResource )
			return;

		mScriptResource->fUnload( this );
		mScriptResource.fRelease( );
	}

}}
