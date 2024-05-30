#ifndef __tRadialMenu__
#define __tRadialMenu__
#include "Gui/tScriptedControl.hpp"
#include "tUser.hpp"
#include "tGameController.hpp"

namespace Sig { namespace Gui
{
	class tRadialMenu : public tScriptedControl
	{
	public:
		explicit tRadialMenu( const tResourcePtr& scriptResource, const tUserPtr& user, const tGameControllerPtr& gc );
		~tRadialMenu( );
		u32 fInputFilterLevel( ) const { return mInputLevel; }
		const tGameControllerPtr& fGameController( ) const;
		tGameController* fGameControllerFromScript( ) { return mGameController.fGetRawPtr( ); }
		void fFadeIn( );
		void fFadeOut( );
		b32  fTryHotKeys( const tGameControllerPtr& gc );
		b32  fHighlightByAngle( f32 angle, f32 magnitude );
		b32  fSelectActiveIcon( );
		b32  fIsActive( ) { return mActive; }
		b32  fHighlightByPosition( const Math::tVec2f &pos , f32 minRadius  );
		Math::tVec2f fGetEntryPosition( u32 entryIdx );
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	private:
		tUserPtr mUser;
		tGameControllerPtr mGameController;
		u32 mInputLevel;
		b32 mActive;
	};

	typedef tRefCounterPtr< tRadialMenu > tRadialMenuPtr;

}}

#endif//__tRadialMenu__
