#ifndef __tRadialMenu__
#define __tRadialMenu__
#include "Gui/tScriptedControl.hpp"
#include "tUser.hpp"

namespace Sig { namespace Gui
{
	class tRadialMenu : public tScriptedControl
	{
	public:
		explicit tRadialMenu( const tResourcePtr& scriptResource, const tUserPtr& user );
		~tRadialMenu( );
		u32 fInputFilterLevel( ) const { return mInputLevel; }
		const Input::tGamepad& fGamepad( ) const { return mUser->fFilteredGamepad( mInputLevel ); }
		Input::tGamepad* fGamepadFromScript( ) { return ( Input::tGamepad* )&mUser->fFilteredGamepad( mInputLevel ); }
		void fFadeIn( );
		void fFadeOut( );
		b32  fTryHotKeys( const Input::tGamepad& gamepad );
		b32  fHighlightByAngle( f32 angle, f32 magnitude );
		b32  fSelectActiveIcon( );
		b32  fIsActive( ) { return mActive; }
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	private:
		tUserPtr mUser;
		u32 mInputLevel;
		b32 mActive;
	};

	typedef tRefCounterPtr< tRadialMenu > tRadialMenuPtr;

}}

#endif//__tRadialMenu__
