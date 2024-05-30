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

		void fPaintScrub( wxAutoBufferedPaintDC* dc );
		void fPaintLine( wxAutoBufferedPaintDC* dc );

		b32 fOnMouseLeftButtonDown( wxMouseEvent& event );
		void fOnMouseLeftButtonUp( wxMouseEvent& event );
		void fOnMouseMove( wxMouseEvent& event );

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

		void fOnPaint( wxPaintEvent& event );
		void fOnEraseBackground( wxEraseEvent& event );

		void fOnMouseMove( wxMouseEvent& event );
		void fOnMouseLeftButtonDown( wxMouseEvent& event );
		void fOnMouseLeftButtonUp( wxMouseEvent& event );

		void fOnTick( f32 dt );

		u32 fTickSpace( ) const { return 48; }

		void fOnTimelineLengthChanged( wxCommandEvent& event );

		f32 fLifetime( );
		void fSetLifetime( const f32 lifetime );

		void fOnPlayButtonClicked( wxCommandEvent& event );
		void fOnPlayButtonFocused( wxFocusEvent& event );

		void fOnSize( wxSizeEvent& event );
		void fOnMouseLeaveWindow( wxMouseEvent& event );
		void fOnMouseEnterWindow( wxMouseEvent& event );

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

		wxPoint mLastMousePosition;
		wxChoice* mChoice;

		wxMemoryDC*				mBackgroundDC;
		b32						mDirty;

		wxButton* mPlay;

		wxPoint mStartingPosition;
		wxSize mTimelineDrawSize;

		b32 mContinueMouseInput;
		b32 mForceMouseInput;
		b32 mUpdateSkeletonToScrub;

		b32 mLastTimelineState;	//paused or playing
		f32 mLastTimelineLength;
		f32 mCurrentTimelineLength;

		Anipk::tAnimationMetaData* mAnimMetaData;
	};


}

#endif // __tSigAnimTimeline__