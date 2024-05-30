//------------------------------------------------------------------------------
// \file tSaveUI.cpp - 10 Aug 2010
// \author Josh Wittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tSaveUI.hpp"
#include "tApplication.hpp"

namespace Sig { namespace Gui {

	namespace { 
		static const tStringPtr cCanvasCreate( "CanvasCreateSaveUI" ); 
		static const float cMinimumScreenTime = 3.f;
	}

	void tSaveUI::tGameStorageSaveInstance::fBegin( )
	{
		mStarted = true;
		mWriter.fGiveBuffer( mData );
		mWriter.fBeginWriting( mUser, mStorageDesc );
	}
	
	b32 tSaveUI::tGameStorageSaveInstance::fFinished( )
	{
		mErrored = mWriter.fIsErrored( );
		return mWriter.fIsFinished( );
	}


	//------------------------------------------------------------------------------
	tSaveUI::tSaveUI( const tResourcePtr& scriptResource, tCanvasFrame& frame ) 
		: tScriptedControl( scriptResource )
		, mState( cStateLoading )
		, mLifetime( cMinimumScreenTime )
		, mUIShown( false )
		, mParentCanvas( &frame )
		, mFrame( &frame )
	{
	}

	//------------------------------------------------------------------------------
	tSaveUI::~tSaveUI( )
	{
		// We must wait for the writer to finish
		fWait(false);
	}

	//------------------------------------------------------------------------------
	b32 tSaveUI::fBeginSaving( ) 
	{
		sigassert( mState == cStateLoading );
		mState = cStateSaving;

		// We proceed with saves regardless of invalid control
		fOnTick( 0 );
		return true;
	}

	void tSaveUI::fShowUI( )
	{
		if( !mUIShown )
		{
			mUIShown = true;

			// Try to create the control
			if( fCreateControlFromScript( cCanvasCreate, this ) )
				fAttachCanvasToFrame( *mFrame );
		}
	}

	//------------------------------------------------------------------------------
	void tSaveUI::fOnTick( float dt )
	{
		sigassert( mState == cStateSaving );

		if( mLifetime > 0 )
			mLifetime -= dt;

		if( mSaveInstances.fNumItems( ) > 0 )
		{
			tSaveInstancePtr& save = mSaveInstances.fFirst( );

			if( !save->mStarted )
			{
				if( save->fWantsUI( ) )
					fShowUI( );

				save->fBegin( );
				sigassert( save->mStarted );
			}
			else if( save->fFinished( ) )
			{
				tSaveInstancePtr save;
				mSaveInstances.fTryPopFirst( save );
				save->fPostErrors( );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tSaveUI::fWait( b32 processRemaining ) 
	{
		// If we're still loading
		if( mState == cStateLoading ) 
		{
			// If we wanna save all then begin  saving
			if( processRemaining )
			{
				// Finish loading the script
				if( mScriptResource && mScriptResource->fLoading( ) )
					mScriptResource->fBlockUntilLoaded( );

				fBeginSaving( );
			}

			// Otherwise we can just clear and return
			else
			{
				fClear( );
				return;
			}
		}

		// If we're here, then we're saving

		// Process all buffered saves
		if( processRemaining )
		{
			// While we have saves to process
			// sleep and advance
			while( !fIsFinishedSaving( ) )
			{
				fSleep( 1 );
				fOnTick( 0.001f );
			}
		}

		// Otherwise clear buffered saves and wait on the writer
		else 
		{
			fClear( true );
		}
	}

	//------------------------------------------------------------------------------
	void tSaveUI::fAddSave( const tSaveInstancePtr& save ) 
	{
		sigassert( save );

		// Grow if necessary
		mSaveInstances.fReserve( 1 );
		mSaveInstances.fPushLast( save );
	}

	//------------------------------------------------------------------------------
	void tSaveUI::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tSaveUI,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Func( _SC("IsFinishedSaving"), &tSaveUI::fIsFinishedSaving )
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("SaveUI"), classDesc );
	}

	//------------------------------------------------------------------------------
	void tSaveUI::fClear( b32 wait ) 
	{
		tSaveInstancePtr save;
		while( mSaveInstances.fTryPopFirst( save ) )
			save->fFinish( wait );
	}

}} 
