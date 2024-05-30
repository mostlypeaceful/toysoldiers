#ifndef __tBombDropOverlay__
#define __tBombDropOverlay__

#include "Gui/tScriptedControl.hpp"
#include "tUser.hpp"
#include "tGamePostEffectMgr.hpp"

namespace Sig { 

	class tPlayer;

namespace Gui
{
	class tBombDropOverlay : public tScriptedControl
	{
	public:
		explicit tBombDropOverlay( const tUserPtr& user );
		~tBombDropOverlay( );

		void fShow( b32 show, tPlayer* player );
		void fSetAngle( f32 angle );
		tUser* fUser( ) const { return mUser.fGetRawPtr( ); }
		tGamePostEffectsData* fPostEffectData( ) { return &mPostEffectData; }

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tUserPtr mUser;
		tGamePostEffectsData mPostEffectData;
	};

	typedef tRefCounterPtr< tBombDropOverlay > tBombDropOverlayPtr;
} }

#endif //__tBombDropOverlay__