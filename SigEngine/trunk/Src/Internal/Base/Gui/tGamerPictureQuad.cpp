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
		if( mState == cStateInProcess )
		{
			mProfilePic.fUpdateST( );

			Math::tVec2f dims;
			Gfx::tTextureReference texture;
			if( mProfilePic.fPic( ).fGetTexture( texture, &dims ) )
			{
				fClearChildren( );
				
				tTexturedQuad* quad = NEW tTexturedQuad( );
				quad->fSetTexture( texture, dims );

				fAddChild( tCanvasPtr( quad ) );

				mState = cStateReady;
			}
		}

		tCanvasFrame::fOnTickCanvas( dt );
	}

	//------------------------------------------------------------------------------
	void tGamerPictureQuad::fSetTexture( u32 requesterHwIndex, const tUser& user, b32 smallPic )
	{
		fSetTexture( requesterHwIndex, user.fPlatformId( ), smallPic );
	}

	//------------------------------------------------------------------------------
	void tGamerPictureQuad::fSetTexture( u32 requesterHwIndex, tPlatformUserId id, b32 smallPic )
	{
		fClearChildren( );
		mProfilePic.fReset( requesterHwIndex, id, smallPic );
		mState = cStateInProcess;
	}

	//------------------------------------------------------------------------------
	void tGamerPictureQuad::fUnsetTexture( )
	{
		fClearChildren( );
		mProfilePic.fRelease( );
		mState = cStateNull;
	}

	//------------------------------------------------------------------------------
	void tGamerPictureQuad::fSetTextureFromScript( tUser * requester, tUser * gamer, bool smallPic )
	{
#ifdef platform_xbox360
		fSetTexture( requester->fLocalHwIndex( ), *gamer, smallPic );
#endif
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
