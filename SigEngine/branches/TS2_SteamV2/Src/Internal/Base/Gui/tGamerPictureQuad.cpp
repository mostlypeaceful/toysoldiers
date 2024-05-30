//------------------------------------------------------------------------------
// \file tGamerPictureQuad.cpp - 01 Mar 2011
// \author jwittner, rknapp
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tGamerPictureQuad.hpp"
#include "tTexturedQuad.hpp"

namespace Sig { namespace Gui
{
	//------------------------------------------------------------------------------
	tGamerPictureQuad::tGamerPictureQuad( )
		: mState( cStateNull )
		, mUserPicTexture( NULL )
	{
	}

	//------------------------------------------------------------------------------
	tGamerPictureQuad::~tGamerPictureQuad( )
	{
		fClearChildren( );
	}

	//------------------------------------------------------------------------------
	void tGamerPictureQuad::fOnTickCanvas( f32 dt )
	{
		if( mUserPic )
			mUserPic->fTickState( );

		if( mUserPic )
		{
			if( mUserPicTexture != mUserPic->fTexture( ) )
				mState = cStateWaitingForRead;

			if( mState == cStateWaitingForRead && mUserPic->fDone( ) )
				fSetTextureFromUserPic( );
		}

		tCanvasFrame::fOnTickCanvas( dt );
	}

	//------------------------------------------------------------------------------
	
	void tGamerPictureQuad::fSetTexture( u32 requesterHwIndex, tUser* user, b32 smallPic )
	{
		if( !user )
		{
			log_warning( 0, this << " Null user passed for gamer picture query" );
			return;
		}

		mState = cStateNull;
		mUserPic = user->fRequestUserPic( requesterHwIndex, smallPic );

		if( mUserPic->fDone( ) )
		{
			fSetTextureFromUserPic( );
		}
		else if( mUserPic->fNeedsRead( ) )
		{
			mUserPic->fSetTexture( requesterHwIndex, user->fPlatformId( ), smallPic );
			mState = cStateWaitingForRead;
		}
		else if( mUserPic->fReading( ) )
		{
			mState = cStateWaitingForRead;
		}
	}

	void tGamerPictureQuad::fSetTextureFromUserPic( )
	{
		fResetQuad( );
		mState = cStateNull;
	}

	//------------------------------------------------------------------------------
	void tGamerPictureQuad::fUnsetTexture( )
	{
		fClearChildren( );
		mUserPic.fRelease( );
		mUserPicTexture = NULL;
	}

	//------------------------------------------------------------------------------
	void tGamerPictureQuad::fResetQuad( )
	{
		fClearChildren( );
		if( mUserPic )
		{
			tTexturedQuad* quad = NEW tTexturedQuad( );
			quad->fSetTexture( (Gfx::tTextureReference::tPlatformHandle)mUserPic->fTexture( ), mUserPic->fTextureDims( ) );
			fAddChild( tCanvasPtr( quad ) );
			mUserPicTexture = mUserPic->fTexture( );
		}
	}

	//------------------------------------------------------------------------------
	void tGamerPictureQuad::fSetTextureFromScript( tUser * requester, tUser * gamer, bool smallPic )
	{
		fSetTexture( requester->fLocalHwIndex( ), gamer, smallPic );
	}

}}


namespace Sig { namespace Gui
{

	//------------------------------------------------------------------------------
	void tGamerPictureQuad::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tGamerPictureQuad, tCanvasFrame, Sqrat::NoCopy<tGamerPictureQuad> > classDesc( vm.fSq( ) );

		classDesc
			.Func( _SC("SetTexture"), &tGamerPictureQuad::fSetTextureFromScript)
			.Func( _SC("UnsetTexture"), &tGamerPictureQuad::fUnsetTexture)
			;

		vm.fNamespace(_SC("Gui")).Bind(_SC("GamerPictureQuad"), classDesc);
	}

}}
