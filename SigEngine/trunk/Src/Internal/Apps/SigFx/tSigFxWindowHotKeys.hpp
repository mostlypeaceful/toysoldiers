#ifndef __tSigFxWindowHotKeys__
#define __tSigFxWindowHotKeys__

#include "tEditorHotKey.hpp"
#include "tSigFxKeyline.hpp"
#include "tSigFxMainWindow.hpp"

namespace Sig
{
	class tSigFxSwitchTabControlHotKey: public tEditorHotKey
	{
		tTabControl* mTabControl;
		u32 mPage;
	public:
		tSigFxSwitchTabControlHotKey( tEditorHotKeyTable& table, tTabControl* tabControl, u32 page, Sig::Input::tKeyboard::tButton key ) 
			: tEditorHotKey( table, key, 0 )
			, mTabControl( tabControl )
			, mPage( page )
		{
		}

		virtual void fFire( ) const
		{
			mTabControl->SetSelection( mPage );
		}
	};

	class tSigFxFrameSelectedHotKey : public tEditorHotKey
	{
		tToolsGuiMainWindow* mEditorWindow;
	public:
		tSigFxFrameSelectedHotKey( tEditorHotKeyTable& table, tToolsGuiMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonF, 0 ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fFrameSelection( ); }
	};
	
	class tSigFxPlayPauseHotKey : public tEditorHotKey
	{
		tSigFxKeyline* mKeyline;
	public:
		tSigFxPlayPauseHotKey( tEditorHotKeyTable& table, tSigFxKeyline* Keyline ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonSpace, 0 ), mKeyline( Keyline ) { }
		virtual void fFire( ) const { mKeyline->fSetHotKeyPaused( !mKeyline->fPaused( ) ); }
	};

	class tSigFxResetSceneHotKey : public tEditorHotKey
	{
		tSigFxKeyline* mKeyline;
	public:
		tSigFxResetSceneHotKey( tEditorHotKeyTable& table, tSigFxKeyline* Keyline ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonEnter, 0 ), mKeyline( Keyline ) { }
		virtual void fFire( ) const
		{
			mKeyline->fSetCurrentTime( 0.f );
			mKeyline->fFxScene( )->fRefreshScene( );
		}
	};


	class tSigFxFrameForwardHotKey : public tEditorHotKey
	{
		tSigFxKeyline* mKeyline;
	public:
		tSigFxFrameForwardHotKey( tEditorHotKeyTable& table, tSigFxKeyline* Keyline ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonRight, 0 ), mKeyline( Keyline ) { }

		virtual void fFire( ) const
		{
			mKeyline->fSetPaused( true );

			const f32 delta = mKeyline->fLifetime( ) * 0.0125f;
			const f32 ticks = ( mKeyline->fCurrentTime( ) / delta );
			f32 curTime = ( u32 ) ticks * delta;
			curTime += delta;
			if( curTime > mKeyline->fLifetime( ) )
				curTime = 0.f;
			mKeyline->fSetCurrentTime( curTime );
		}
	};

	class tSigFxFrameBackwardHotKey : public tEditorHotKey
	{
		tSigFxKeyline* mKeyline;
	public:
		tSigFxFrameBackwardHotKey( tEditorHotKeyTable& table, tSigFxKeyline* Keyline ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonLeft, 0 ), mKeyline( Keyline ) { }

		virtual void fFire( ) const
		{
			mKeyline->fSetPaused( true );

			const f32 delta = mKeyline->fLifetime( ) * 0.0125f;
			const f32 ticks = ( mKeyline->fCurrentTime( ) / delta );
			f32 curTime = ( u32 ) ticks * delta;
			curTime -= delta;
			if( curTime < 0.f )
				curTime = 0.f;
			mKeyline->fSetCurrentTime( curTime );
		}
	};



	class tSigFxSBuildHotKey : public tEditorHotKey
	{
		tSigFxMainWindow* mEditorWindow;
	public:
		tSigFxSBuildHotKey( tEditorHotKeyTable& table, tSigFxMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonB, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fBuild( ); }
	};
	
	class tSigFxSaveHotKey : public tEditorHotKey
	{
		tSigFxMainWindow* mEditorWindow;
	public:
		tSigFxSaveHotKey( tEditorHotKeyTable& table, tSigFxMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonS, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fGuiApp( ).fSaveDoc( false ); }
	};

	class tSigFxOpenHotKey : public tEditorHotKey
	{
		tSigFxMainWindow* mEditorWindow;
	public:
		tSigFxOpenHotKey( tEditorHotKeyTable& table, tSigFxMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonO, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fOpenDoc( ); }
	};

	class tSigFxUndoHotKey : public tEditorHotKey
	{
		tSigFxMainWindow* mEditorWindow;
	public:
		tSigFxUndoHotKey( tEditorHotKeyTable& table, tSigFxMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonZ, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fUndo( ); }
	};

	class tSigFxRedoHotKey : public tEditorHotKey
	{
		tSigFxMainWindow* mEditorWindow;
	public:
		tSigFxRedoHotKey( tEditorHotKeyTable& table, tSigFxMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonY, tEditorHotKey::cOptionCtrl ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fRedo( ); }
	};

	class tSigFxCenterSelectedObjectsHotKey : public tEditorHotKey
	{
		tSigFxMainWindow* mEditorWindow;
	public:
		tSigFxCenterSelectedObjectsHotKey( tEditorHotKeyTable& table, tSigFxMainWindow* editorWindow ) 
			: tEditorHotKey( table, Input::tKeyboard::cButtonC, 0 ), mEditorWindow( editorWindow ) { }
		virtual void fFire( ) const { mEditorWindow->fCenterSelectedObjects( ); }
	};

}

#endif // __tSigFxWindowHotKeys__