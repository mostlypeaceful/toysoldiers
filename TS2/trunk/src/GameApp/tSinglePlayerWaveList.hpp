#ifndef __tSinglePlayerWaveList__
#define __tSinglePlayerWaveList__
#include "Gui/tScriptedControl.hpp"
#include "tUser.hpp"

namespace Sig
{
	struct tWaveDesc;
	class tOffensiveWaveDesc;
	class tWaveList;
}

namespace Sig { namespace Gui
{
	
	class tSinglePlayerWaveList : public tScriptedControl
	{
	public:
		explicit tSinglePlayerWaveList( const tResourcePtr& scriptResource, const tUserPtr& user, const tStringPtr& layer );
		~tSinglePlayerWaveList( );
		void fSetup( tWaveList* wavelist );
		void fAddWaveIcon( tWaveDesc& wave );
		void fAddWaveIcon( tOffensiveWaveDesc& wave, u32 unitID );
		void fNextWave( );
		void fReadying( );
		void fCountdownTimer( f32 time );
		void fSurvivalTimer( f32 time );
		void fSurvivalRound( u32 round );
		void fLaunchStart( );
		void fLaunching( f32 dt );
		void fShow( b32 show );
		b32 fIsVisible( ) { return mIsVisible; }
		void fLooping( b32 looping );
		void fClear( );
		virtual void fFinalEnemyWave( );
		tUser* fUser( ) const { return mUser.fGetRawPtr( ); }
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	private:
		tUserPtr mUser;
		b32 mIsVisible;
		tStringPtr mLayer;
	};

	typedef tRefCounterPtr< tSinglePlayerWaveList > tSinglePlayerWaveListPtr;

}}

#endif//__tRadialMenu__
