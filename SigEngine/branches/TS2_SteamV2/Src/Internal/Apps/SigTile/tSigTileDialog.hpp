#ifndef __tSigTileDialog__
#define __tSigTileDialog__
#include "tWxSlapOnDialog.hpp"

namespace Sig
{
	class tSigTileMainWindow;

	class tSigTileDialog : public tWxSlapOnDialog
	{
	protected:
		tSigTileMainWindow*		mMainWindow;

	public:
		tSigTileMainWindow*		fMainWindow( ) const { return mMainWindow; }
		tSigTileDialog( tSigTileMainWindow* mainWindow, const char* regKeyName );
		void fOnTick( );
	};

}


#endif//__tSigTileDialog__
