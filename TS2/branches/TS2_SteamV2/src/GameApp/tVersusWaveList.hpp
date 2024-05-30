/// \file   tVersusWaveList.hpp
/// \author Randall Knapp
/// \par    Email:\n rknapp\@signalstudios.net
/// \date   November 2, 2010 - 14:11
/// \par    Copyright:\n &copy; Signal Studios 2010-2011
/// \brief  Wave list for versus mode. It shows both players offensive waves but ignores the repeating infantry waves.
#ifndef __tVersusWaveList__
#define __tVersusWaveList__
#include "tSinglePlayerWaveList.hpp"
#include "tUser.hpp"

namespace Sig { namespace Gui
{

	class tVersusWaveList : public tSinglePlayerWaveList
	{
	public:
		explicit tVersusWaveList( const tResourcePtr& scriptResource, const tUserPtr& user );
		~tVersusWaveList( );
		virtual void fFinalEnemyWave( );
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	typedef tRefCounterPtr< tVersusWaveList > tVersusWaveListPtr;
} }

#endif //__tVersusWaveList__