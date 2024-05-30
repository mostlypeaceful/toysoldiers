//------------------------------------------------------------------------------
// \file tGamerPictureQuad.hpp - 01 Mar 2011
// \author jwittner, rknapp
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tGamerPictureQuad__
#define __tGamerPictureQuad__

#include "tCanvas.hpp"
#include "Gfx\tWorldSpaceQuads.hpp"
#include "tUserPicServices.hpp"

namespace Sig { namespace Gui 
{
	///
	/// \class tGamerPictureQuad
	/// \brief 
	class tGamerPictureQuad : public tCanvasFrame 
	{
		define_dynamic_cast( tGamerPictureQuad, tCanvasFrame );

	public:
		tGamerPictureQuad( );
		~tGamerPictureQuad( );

		void fSetTexture( u32 requesterHwIndex, const tUser & user, b32 smallPic );
		void fSetTexture( u32 requesterHwIndex, tPlatformUserId id, b32 smallPic );
		
		void fUnsetTexture( );

		// Inherited
		virtual void fOnTickCanvas( f32 dt );

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		void fSetTextureFromScript( tUser * requester, tUser * gamer, bool smallPic );

	private:
		enum { cStateNull, cStateInProcess, cStateReady };

	private:
		u32						mState;
		tUserProfilePicRef		mProfilePic;
	};

} }

#endif//__tGamerPictureQuad__
