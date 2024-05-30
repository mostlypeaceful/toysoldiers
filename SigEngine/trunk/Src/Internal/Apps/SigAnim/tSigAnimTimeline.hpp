#ifndef __tSigAnimTimeline__
#define __tSigAnimTimeline__

#include "wx/dcbuffer.h"
#include "tKeyFrameAnimation.hpp"
#include "Anipk.hpp"

namespace Sig
{
	class tSigAnimTimeline;
	class tEntityControlPanel;

	class tSigAnimTimelineScrub
	{
	public:
		tSigAnimTimelineScrub( tSigAnimTimeline* timeline );
		~tSigAnimTimelineScrub( );

		b32 fOnMouseLeftButtonDown( wxMouseEvent& event );
		void fOnMouseLeftButtonUp( wxMouseEvent& event );
		void fOnMouseMove( wxMouseEvent& event );

		void fPaintScrub( wxAutoBufferedPaintDC* dc );

		f32  fGetX( ) const { return mX; }

		f32  fGetTime( ) const;
		void fSetXThruTime( const f32 time );

	private:
		tSigAnimTimeline* mParentTimeline;

		f32 mX;
		f32 mTime;
		b32	mMouseOver;
		b32	mMouseHookedOn;
		wxPoint mPoints[ 3 ];
		wxPoint mLastMousePosition;
	};




	class tSigAnimTimeline : public wxPanel
	{
	public:
		tSigAnimTimeline( wxPanel* parent, tEntityControlPanel* entityControlPanel );
		~tSigAnimTimeline( );

		void fDrawTickPos( wxDC* dc, f32 x, f32 height, f32 yOffset = 0 );
		void fDrawTickTime( wxDC* dc, f32 t, f32 height );

		void fDrawTextPos( wxDC* dc, f32 x, f32 t, f32 height );

		u32 fTickSpace( ) const { return 48; }

		f32 fLifetime( );
		void fSetLifetime( const f32 lifetime );

		void fOnTick( f32 dt );

		wxPoint fGetStartingPosition( ) const { return mStartingPosition; }
		wxSize fGetTimelineDrawSize( ) const { return mTimelineDrawSize; }

		tSigAnimTimelineScrub* fScrub( ) { return mScrub; }

		void fSetUpdateSkeletonToScrub( ) { mUpdateSkeletonToScrub = true; }
		b32 fUpdateSkeletonToScrub( ) { return mUpdateSkeletonToScrub; }
	
		void fSetAnimEvents( const std::string& animPkLabel, const std::string& animName, Anipk::tFile& anipkFile, const tKeyFrameAnimation* kfAnim );

		void fMarkTimelineDirty( ) { mDirty = true; }

	private:
		tSigAnimTimelineScrub* mScrub;
		tEntityControlPanel* mEntityControlPanel;

		wxButton* mPlay;

		wxMemoryDC* mBackgroundDC;
		b32 mDirty;

		wxPoint mLastMousePosition;
		wxPoint mStartingPosition;
		wxSize mTimelineDrawSize;

		b32 mContinueMouseInput;
		b32 mForceMouseInput;
		b32 mUpdateSkeletonToScrub;

		b32 mLastTimelineState;	//paused or playing
		f32 mLastTimelineLength;
		f32 mCurrentTimelineLength;

		// Selection/multiselection
		b32 mShiftHeld;
		b32 mSelectingNewRange;
		b32	mExpandingLeft;
		b32 mExpandingRight;
		b32 mMovingRange;
		Math::tVec2f mSelectionRange;
		tGrowableArray<u32> mMultiselectedEvents;
		u32 mSingleSelectedEvent;
		u32 mHoveredEvent;

		// Cut/copy/paste
		tGrowableArray< Anipk::tKeyFrameTag > mEventClipboard;

		Anipk::tAnimationMetaData* mAnimMetaData;

		tEditorContextActionList mContextActions;

		f32 fToT( f32 timelinePos ) const;

		void fOnPaint( wxPaintEvent& event );
		void fOnEraseBackground( wxEraseEvent& event );

		void fOnMouseMove( wxMouseEvent& event );
		void fOnMouseLeftButtonDown( wxMouseEvent& event );
		void fOnMouseLeftButtonUp( wxMouseEvent& event );
		void fOnRightClick( wxMouseEvent& event );
		void fOnAction( wxCommandEvent& event );

		void fOnPlayButtonClicked( wxCommandEvent& event );
		void fOnPlayButtonFocused( wxFocusEvent& event );

		void fOnSize( wxSizeEvent& event );
		void fOnMouseLeaveWindow( wxMouseEvent& event );
		void fOnMouseEnterWindow( wxMouseEvent& event );

		void fSyncPauseButton( );

		b32 fRangeIsSelected( ) const { return mSelectionRange != Math::tVec2f::cZeroVector; }
		void fClearSelectionRange( );
		Math::tVec2f fGetOrderedRange( ) const;
		void fSetSelectionRange( f32 l, f32 r );

		void fCopySelectedEvents( );
		void fDeleteSelectedEvents( );
		void fPasteEvents( f32 pasteAnchorPos );

		class tCutCopyPasteAction : public tEditorContextAction
		{
		public:
			// Also taking care of delete for now.
			tCutCopyPasteAction( tSigAnimTimeline* timeline );
			virtual b32 fAddToContextMenu( wxMenu& menu );
			virtual b32	fHandleAction( u32 actionId );

		private:
			tSigAnimTimeline* mTimeline;
			u32 mCut, mCopy, mPaste, mDelete;
		};
	};


}

#endif // __tSigAnimTimeline__