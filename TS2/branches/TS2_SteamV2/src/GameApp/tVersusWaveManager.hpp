/// \file   tVersusWaveManager.hpp
/// \author Randall Knapp
/// \par    Email:\n rknapp\@signalstudios.net
/// \date   November 2, 2010 - 17:45
/// \par    Copyright:\n &copy; Signal Studios 2010-2011
/// \brief  Wave manager with the versus functionality
#ifndef __tVersusWaveManager__
#define __tVersusWaveManager__

#include "tWaveManager.hpp"
#include "tRtsCursorLogic.hpp"

namespace Sig {

	class tOffensiveWaveDesc
	{
	public:
		tOffensiveWaveDesc( );

		tStringPtr	mName;
		tStringPtr	mDesc;
		u32			mCost;
		u32			mCountry;
		tStringPtr	mWaveID;
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	class tVersusWaveManager : public tWaveManager
	{
	public:
		tVersusWaveManager( );
		virtual ~tVersusWaveManager( );

		void fUpdate( f32 dt );

		void fSetupOffensiveWaveMenu( Gui::tRadialMenuPtr& radialMenu, tPlayer* player );
		void fAddOffensiveWave( const tOffensiveWaveDesc& waveDesc );
		b32  fLaunchOffensiveWave( tOffensiveWaveDesc& waveDesc, tPlayer* player );
		virtual void fOnLevelUnloadBegin( );

		const tFixedArray< tWaveListPtr, 2 >& fWaveLists( ) const { return mOffensiveWaveLists; }

		void fRelaunchOffensiveWave( const tStringPtr& waveID, u32 country );
		void fReactivateWaveLists( const tGrowableArray< tSaveGameData::tWaveID >& waveData );
		tStringPtr fGetDescWaveID( u32 waveIndex, u32 country ); 
	private:
		// Versus
		tFixedArray< tWaveListPtr, 2 > mOffensiveWaveLists;
		tFixedArray< Gui::tSinglePlayerWaveListPtr, 2 > mVersusUi;
		tFixedArray< Gui::tSinglePlayerWaveListPtr, 2 > mNetVersusUi;

		tGrowableArray< tOffensiveWaveDesc > mOffensiveWaveDescriptions;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tVersusWaveManager );
}

#endif //__tVersusWaveManager__