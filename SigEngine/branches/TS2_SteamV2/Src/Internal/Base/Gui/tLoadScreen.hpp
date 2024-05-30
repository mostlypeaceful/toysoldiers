#ifndef __tLoadScreen__
#define __tLoadScreen__
#include "tScriptedControl.hpp"

namespace Sig { namespace Gui
{
	class tLoadScreen : public tScriptedControl
	{
	public:
		enum tState
		{
			cStateLoading,
			cStateBeginning,
			cStatePlaying,
			cStateEnding,
			cStateCount
		};
	public: // interface for tGameLoadAppState or similar
		explicit tLoadScreen( const tResourcePtr& scriptResource );
		tState fState( ) const { return mState; }
		b32 fLoadComplete( ) const { return fScriptLoadCompleted(); }
		void fBegin( ); // i.e., will begin fading in presumably
		b32  fTryAdvanceToPlaying( ) const; // i.e., the load screen is now fully faded in, covering the screen, so the old level can now be unloaded
		void fSetNewLevelIsLoaded( ) { mNewLevelIsLoaded = true; }
		b32  fCanProceedToNewLevel( ) const; // i.e., if there was a necessary sequence to the load screen as in a tutorial, it is now finished
		b32  fCanSpawn( ) const { return mCanSpawn; }
		void fEnd( ); // i.e., begin fading out
		b32  fIsComplete( ) const; // the load screen is completely done and you can go on with playing the new level
		Sqrat::Table& fCustomData( ) { return mCustomData; }
	public: // interface for scripted canvas to inform load screen of events
		void fSetCanvasIsFadedIn( ) { mCanvasIsFadedIn = true; }
		void fSetCanvasIsFadedOut( ) { mCanvasIsFadedOut = true; }
		void fSetCanProceedToNewLevel( ) { mCanProceedToNewLevel = true; }
		void fSetCanSpawn( b32 canSpawn ) { mCanSpawn = canSpawn; }
		b32  fNewLevelIsLoaded( ) const { return mNewLevelIsLoaded; }
		Sqrat::Object fCustomDataFromScript( ) const { return Sqrat::Object( mCustomData ); }
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	private:
		mutable tState	mState;
		b32				mCanvasIsFadedIn;
		b32				mCanvasIsFadedOut;
		b32				mNewLevelIsLoaded;
		b32				mCanProceedToNewLevel;
		b32				mCanSpawn;
		Sqrat::Table	mCustomData;
	};

	typedef tRefCounterPtr<tLoadScreen> tLoadScreenPtr;
}}

#endif//__tLoadScreen__
