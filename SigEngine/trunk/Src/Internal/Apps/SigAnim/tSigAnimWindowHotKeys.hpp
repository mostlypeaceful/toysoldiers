#ifndef __tSigAnimWindowHotKeys__
#define __tSigAnimWindowHotKeys__

#include "tEditorHotKey.hpp"
#include "tSigAnimMainWindow.hpp"

namespace Sig
{
	


	class tSigAnimFrameForwardHotKey : public tEditorHotKey
	{
		tSigAnimMainWindow* mEditorWindow;
	public:
		tSigAnimFrameForwardHotKey( tEditorHotKeyTable& table, tSigAnimMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonRight, 0 ), mEditorWindow( editorWindow ) { }

		virtual void fFire( ) const
		{
			mEditorWindow->fNextFrame( );			
		}
	};

	class tSigAnimFrameBackwardHotKey : public tEditorHotKey
	{
		tSigAnimMainWindow* mEditorWindow;
	public:
		tSigAnimFrameBackwardHotKey( tEditorHotKeyTable& table, tSigAnimMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonLeft, 0 ), mEditorWindow( editorWindow ) { }

		virtual void fFire( ) const
		{
			mEditorWindow->fPreviousFrame( );
		}
	};



	class tSigAnimAnimEventHotKey : public tEditorHotKey
	{
		tSigAnimMainWindow* mEditorWindow;
	public:
		tSigAnimAnimEventHotKey( tEditorHotKeyTable& table, tSigAnimMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonS, 0 ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const
		{
			mEditorWindow->fToggleAnimEd( );
		}
	};
	
	

}

#endif // __tSigAnimWindowHotKeys__