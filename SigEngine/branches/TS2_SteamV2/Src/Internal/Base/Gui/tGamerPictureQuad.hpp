//------------------------------------------------------------------------------
// \file tGamerPictureQuad.hpp - 01 Mar 2011
// \author jwittner, rknapp
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tGamerPictureQuad__
#define __tGamerPictureQuad__

#include "tCanvas.hpp"
#include "tUser.hpp"

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

		void fSetTexture( u32 requesterHwIndex, tUser* user, b32 smallPic );
		void fUnsetTexture( );

		// Inherited
		virtual void fOnTickCanvas( f32 dt );

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		void fSetTextureFromScript( tUser * requester, tUser * gamer, bool smallPic );
		void fSetTextureFromUserPic( );
		void fResetQuad( );

	private:
		enum { cStateNull, cStateWaitingForRead };

	private:
		u32					mState;
		tUserPicPtr			mUserPic;
		void*				mUserPicTexture; //used to diff against the user pic to see if it has changed
	};

} }

#endif//__tGamerPictureQuad__
