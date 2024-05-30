#ifndef __tDialogBox__
#define __tDialogBox__
#include "Gui/tScriptedControl.hpp"
#include "tUser.hpp"

namespace Sig { namespace Gui
{
	class tDialogBox : public tScriptedControl
	{
	public:
		explicit tDialogBox( const tResourcePtr& scriptResource, const tUserPtr& user, b32 needsConfirmation );
		~tDialogBox( );
		u32 fInputFilterLevel( ) const { return mInputLevel; }
		const Input::tGamepad& fGamepad( ) const { return mUser->fFilteredGamepad( mInputLevel ); }
		Input::tGamepad* fGamepadFromScript( ) { return ( Input::tGamepad* )&mUser->fFilteredGamepad( mInputLevel ); }
		b32 fNeedsConfirmation( ) const { return mNeedsConfirmation; }
		void fFadeIn( );
		void fFadeOut( );
		void fSetText( const tStringPtr& text );
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	private:
		tUserPtr mUser;
		u32 mInputLevel;
		b32 mNeedsConfirmation;
	};

	typedef tRefCounterPtr< tDialogBox > tDialogBoxPtr;

}}

#endif//__tDialogBox__
