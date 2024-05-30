// Personal Best UI
#ifndef __tPersonalBestUI__
#define __tPersonalBestUI__

#include "Gui/tScriptedControl.hpp"
#include "tPlayer.hpp"

namespace Sig { namespace Gui
{
	class tPersonalBestUI : public tScriptedControl
	{
	public:
		explicit tPersonalBestUI( const tPlayerPtr& player );
		~tPersonalBestUI( ) { }

		void fNewPersonalBest( u32 statID, f32 newValue );
		tPlayer* fPlayer( ) { return mPlayer.fGetRawPtr( ); }

		b32 fShown( ) const { return mShown; }

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tPlayerPtr mPlayer;
		b32 mShown;
	};

	typedef tRefCounterPtr< tPersonalBestUI > tPersonalBestUIPtr;
} }

#endif //__tPersonalBestUI__