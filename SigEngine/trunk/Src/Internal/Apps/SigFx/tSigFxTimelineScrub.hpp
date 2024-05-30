#ifndef __tSigFxTimelineScrub__
#define __tSigFxTimelineScrub__

#include "wx/dcbuffer.h"

namespace Sig
{
	class tSigFxKeyline;
	class tSigFxGraphline;
	class tSigFxTimeline;


	class tSigFxTimelineScrub
	{
	public:
		tSigFxTimelineScrub( tSigFxTimeline* timeline, tSigFxKeyline* parent );
		~tSigFxTimelineScrub( );

		void fPaintScrub( wxAutoBufferedPaintDC* dc );
		void fPaintLine( wxAutoBufferedPaintDC* dc );

		b32 fOnMouseLeftButtonDown( wxMouseEvent& event );
		void fOnMouseLeftButtonUp( wxMouseEvent& event );
		void fOnMouseMove( wxMouseEvent& event );

		void fSetX( f32 x );
		f32  fGetX( ) const { return mX; }

		f32  fGetTime( ) const;
		void fSetXThruTime( const f32 time );

	private:

		tSigFxTimeline* mParentTimeline;
		tSigFxKeyline* mParentKeyline;
		

		f32 mX;
		f32 mTime;
		b32	mMouseOver;
		b32	mMouseHookedOn;
		wxPoint mPoints[ 3 ];
		wxPoint mLastMousePosition;
	};




	class tSigFxTimeline : public wxPanel
	{
	public:
		tSigFxTimeline( wxPanel* parent );
		~tSigFxTimeline( );

		void fOnPaint( wxPaintEvent& event );
		void fOnEraseBackground( wxEraseEvent& event );

		void fOnMouseMove( wxMouseEvent& event );
		void fOnMouseLeftButtonDown( wxMouseEvent& event );
		void fOnMouseLeftButtonUp( wxMouseEvent& event );

		void fOnTick( f32 dt );

		void fSetKeyline( tSigFxKeyline* keyline ) { mKeyline = keyline; }

		u32 fTickSpace( ) const { return 48; }

		void fOnTimelineLengthChanged( wxCommandEvent& event );

		f32 fLifetime( );
		void fSetLifetime( const f32 lifetime );

		void fOnPlayButtonClicked( wxCommandEvent& event );
		void fOnPlayButtonFocused( wxFocusEvent& event );

		void fOnSize( wxSizeEvent& event );
		void fOnMouseLeaveWindow( wxMouseEvent& event );
		void fOnMouseEnterWindow( wxMouseEvent& event );

	private:

		tSigFxKeyline* mKeyline;
		tSigFxTimelineScrub* mScrub;
		wxPoint mLastMousePosition;
		wxChoice* mChoice;

		wxMemoryDC*				mBackgroundDC;
		b32						mDirty;

		wxButton* mPlay;

		b32 mContinueMouseInput;
		b32 mForceMouseInput;

		b32 mLastTimelineState;	//paused or playing
		f32 mLastTimelineLength;
	};


}

#endif // __tSigFxTimelineScrub__