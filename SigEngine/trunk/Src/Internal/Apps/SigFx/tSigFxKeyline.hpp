#ifndef __tSigFxKeyline__
#define __tSigFxKeyline__
#include "FX/tFxGraph.hpp"
#include "tSigFxGraph.hpp"
#include "tSigFxTimelineScrub.hpp"
#include "tSigFxFile.hpp"
#include "tTabPanel.hpp"
#include "Editor/tEditorAction.hpp"
#include "tSigFxSystem.hpp"

namespace Sig
{
	class tSigFxMainWindow;
	class tSigFxKeylineTrack;
	class tSigFxGraphline;
	
	class tSigFxKeyline : public tTabPanel
	{
		// main window
		tSigFxMainWindow* mMainWindow;
		tEditorActionPtr mUndoAction;
		wxPanel* mKeylineScrubPanel;

	public:

		void fOnSize( wxSizeEvent& event );
		void fOnPaint( wxPaintEvent& event );
		void fOnEraseBackground( wxEraseEvent& event );
		void fOnMouseMove( wxMouseEvent& event );
		void fOnMouseLeftButtonDown( wxMouseEvent& event );
		void fOnMouseLeftButtonUp( wxMouseEvent& event );
		void fOnMouseLeftDoubleClick ( wxMouseEvent& event );

		void fOnMouseLeaveWindow( wxMouseEvent& event );
		void fOnMouseEnterWindow( wxMouseEvent& event );

		void fOnKeyDown( wxKeyEvent& event );
		void fOnKeyUp( wxKeyEvent& event );
		
		wxBitmapButton mPlayButton;
		void fOnPlayButtonClicked( wxCommandEvent& event );
		void fOnButtonEraseBackground( wxEraseEvent& event );

		virtual void fBuildPageFromEntities( tEditorSelectionList& selected );
		void fBuildKeyline( );
		void fClearKeyline( );

		void fDeselectAllKeyframes( );
		tSigFxGraphline* fGraphline( ) { return mGraphline; }

	private:

		wxBitmap play_normal;
		wxBitmap play_hover;
		wxBitmap play_selected;
		
		wxBitmap pause_normal;
		wxBitmap pause_hover;
		wxBitmap pause_selected;

		
		FX::tSigFxSystem* mFxScene;
		tSigFxGraphline* mGraphline;

		wxSize		mInnerBorder;
		b32			mPaused;
		b32			mPreviouslyPaused;
		b32			mPausedByKeyframe;

		b32			mKeyframesMoved;
		b32			mDoRectangleSelection;
		b32			mCopyingKeyframes;
		b32			mHotKeyPaused;
		wxRect		mSelectionRectangle;
		wxPoint		mSelectionRectangleAnchor;
		
		b32 mForceMouseInput;
		b32	mContinueMouseInput;

		tGrowableArray< tSigFxKeylineTrack* > mKeylineTracks;
		tEntityPtr mSelectedEntity;

		tSigFxTimelineScrub* mScrub;

	public:
		tSigFxKeyline( wxNotebook* parent, tSigFxMainWindow* mainWindow, FX::tSigFxSystem* scene, wxPanel* KeylineScrubPanel );
		virtual ~tSigFxKeyline( );
		virtual void fOnTick( f32 dt );
		virtual void fSaveInternal( HKEY hKey ) { }
		virtual void fLoadInternal( HKEY hKey ) { }

		FX::tSigFxSystem* fFxScene( ) { return mFxScene; }

		void		fAdvanceTime( f32 dt );
		void		fSetGraphline( tSigFxGraphline* graphline ) { mGraphline = graphline; }

		void		fSetScrub( tSigFxTimelineScrub* scrub ) { mScrub = scrub; }

		wxSize		fGetInnerBorder( ) const { return mInnerBorder; }
		b32			fRectangleSelectionInProcess( ) const { return mDoRectangleSelection; }
		f32			fCurrentTime( ) const { return mFxScene->fCurrentTime( ); }
		
		f32			fLifetime( ) const { return mFxScene->fLifetime( ); }
		void		fSetLifetime( f32 len ) { mFxScene->fSetLifetime( len ); }

		f32			fDelta( ) const { return fCurrentTime( ) / fLifetime( ); }
		void		fSetCurrentTime( const f32 time );
		wxPoint		fGetKeylinePosition( );
		wxSize		fGetKeylineSize( );

		void		fImmediateUpdate( b32 forcekeyframeupdate = false );

		u32			fTickSpace( ) const { return 6; }

		const u32 fGetCurrentBarPos( );

		void fCopyAllSelectedKeyframes( );
		void fDeleteAllSelectedKeyframes( );

		void fSelectKeyframes( tKeyframePtr rawkey );

		void fSetHotKeyPaused( const b32 pause );
		void fClearHotKeyPausedState( ) { mHotKeyPaused = false; }
		void fSetPaused( const b32 pause );
		b32	 fPaused( ) const { return mPaused; }

		void fForceWholeSceneRefresh( );
		tSigFxMainWindow* fMainWindow( ) { return mMainWindow; }

		static wxPoint	mLastMouseClickPosition;
		static wxPoint	mLastMousePosition;
		
	};

}

#endif//__tSigFxKeyline__

