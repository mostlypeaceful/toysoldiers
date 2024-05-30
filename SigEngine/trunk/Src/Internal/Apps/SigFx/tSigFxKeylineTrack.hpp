#ifndef __tSigFxKeylineTrack__
#define __tSigFxKeylineTrack__

#include "tSigFxKeyline.hpp"
#include "tSigFxGraph.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "Editor/tEditorAction.hpp"
#include "wx/dcbuffer.h"


namespace Sig
{
	class tSigFxKeyframe;

	class tSigFxKeylineTrack
	{
	public:
		tSigFxKeylineTrack( tSigFxKeyline* Keyline, tGraphPtr graph,
			const wxString& keyframeText, const f32 trackHeight, const wxColour& normal,
			const wxColour& highlight, const wxColour& selected );

		virtual ~tSigFxKeylineTrack( );

		void fDeleteAllSelectedKeyframes( );
		b32 fHasKeyframe( tSigFxKeyframe* key ) const;
		b32 fHasKeyframe( tKeyframePtr key ) const;
		
		void fAddNewKeyframe( tSigFxKeyframe* key );

		void fRemoveKeyframe( tSigFxKeyframe* key );
		
		void fUpdateGraphValues( );

		void fDeleteKeyframe( tSigFxKeyframe* key );
		
		u32 fNumKeyframes( ) const;

		tSigFxKeyframe* fKeyframe( const u32 idx );

		void fOnSize( wxSizeEvent& event );

		void fOnPaint1( wxAutoBufferedPaintDC* dc );
		void fOnPaint2( wxAutoBufferedPaintDC* dc );

		b32 fOnMouseMove( wxMouseEvent& event );
		void fOnMouseLeftButtonDown( wxMouseEvent& event );
		void fOnMouseLeftButtonUp( wxMouseEvent& event );
		b32 fOnMouseLeftDoubleClick ( wxMouseEvent& event );

		void fSelectKeyframesWithinBounds( const wxRect& selectionRectangle );
		void fSelectKeyframe( tKeyframePtr rawkey );
			
		tSigFxKeyline*	fTheKeyline( ) const { return mTheKeyline; }

		tKeyframePtr fGetRawKeyframe( u32 idx ) const;
		tKeyframePtr fNewRawKeyframe( tSigFxKeyframe* key );

		tSigFxKeyframe* fCreateNewKeyframe( u32 xpos );

		void fToggleShrink( );

		void fDeselectAll( );

	private:

		tSigFxKeyline*			mTheKeyline;
		tGraphPtr				mGraph;
		
		f32						mTrackHeight;
		b32						mTrackSelected;
		b32						mPlacingNewKeyframe;
		b32						mKeyframesShrunk;

		wxPoint					mY;
		wxPoint					mLastMousePos;

		wxColour mNormalColour;
		wxColour mHighlightedColour;
		wxColour mSelectedColour;
		wxString mKeyframeText;

		tGrowableArray< tSigFxKeyframe* > mKeyframes;
		tGrowableArray< tSigFxKeyframe* > mDelayedAddList;
		tGrowableArray< tSigFxKeyframe* > mDelayedRemoveList;
	};

}

#endif	// __tSigFxKeylineTrack__