#ifndef __tSigFxKeyframe__
#define __tSigFxKeyframe__

#include "FX/tFxKeyframe.hpp"
#include "FX/tFxKeyframe.hpp"
#include "tSigFxKeyline.hpp"
#include "wx/dcbuffer.h"

namespace Sig
{

	class tSigFxKeyframe : public tRefCounter
	{
	public:
		tSigFxKeyframe( tKeyframePtr key, 
			tSigFxKeyline* parent, 
			tSigFxKeylineTrack* track,
			tGraphPtr graph,
			const u32 ypos,
			const wxString& text, 
			const wxColour& normal, 
			const wxColour& highlite, 
			const wxColour& selected );
		
		~tSigFxKeyframe( );
		
		tSigFxKeyframe( const tSigFxKeyframe* keyframe, tKeyframePtr rawkey );

		void fOnPaint( wxAutoBufferedPaintDC* dc );

		b32	fOnMouseLeftButtonDown( wxMouseEvent& event );
		void fOnMouseLeftButtonUp( wxMouseEvent& event );
		b32 fOnMouseLeftButtonDoubleClick( wxMouseEvent& event );

		b32 fOnMouseMove( wxMouseEvent& event, b32 resetHighlight, b32 oneHighlight );

		void fOnParentResize( wxSizeEvent& event, const u32 ypos );

		b32	fSelected( ) const { return mSelected; }
		b32	fHighlighted( ) const { return mHighlighted; }
		
		void fSetSelected( const b32 selected ) { mSelected = selected; }
		void fSetHighlighted( const b32 highlighted ) { mHighlighted = highlighted; }

		void fSelect( );
		void fDeselect( b32 remove = true );

		void fHighlight( );
		void fUnHighlight( b32 remove = true );

		struct tSortKeyframe
		{
			inline b32 operator()( const tSigFxKeyframe* a, const tSigFxKeyframe* b ) const
			{
				if( a->fSelected( ) )
					return false;
				if( b->fSelected( ) )
					return true;
				
				return a->fX( ) < b->fX( );
			}
		};
		
		struct tSortKeyframeByXOnly
		{
			inline b32 operator()( const tSigFxKeyframe* a, const tSigFxKeyframe* b ) const
			{
				return a->fX( ) < b->fX( );
			}
		};

		void			fSetSize( wxSize size ) { mBounds.SetSize( size ); }
		void			fSetPosition( wxPoint pos ) { mBounds.SetX( pos.x ); mBounds.SetY( pos.y ); }

		const wxRect	fGetBounds( ) const { return mBounds; }
		const wxSize	fGetTextSize( ) const { return mTextSize; }
		f32 fX( ) const { return mKeyframe->fX( ); }
		void fSetX( f32 x );
		f32 fOldX( ) const { return mLastX; }

		void fCalculateSizes( );

		static void		fRemoveAllSelected( );
		static void		fRemoveAllHighlighted( );

		static tGrowableArray< tSigFxKeyframe* > mSelectedKeyframes;
		static tGrowableArray< tSigFxKeyframe* > mHighlightedKeyframes;

		tSigFxKeyline*		fTheKeyline( ) const { return mTheKeyline; }
		tSigFxKeylineTrack*fParentTrack( ) const { return mParentTrack; }

		void				fNormalSize( wxSize& size ) { mNormalSize = size; }
		void				fExtraSize( wxSize& size ) { mExtraSize = size; }

		const wxSize&		fNormalSize( ) const { return mNormalSize; }
		const wxSize&		fExtraSize( ) const { return mExtraSize; }

		wxString&			fText( ) { return mText; }
		wxFont&				fFont( ) { return mFont; }
		wxBrush&			fBrush( ) { return mSolidBrush; }

		const b32			fLocked( ) const { return mLocked; }
		void				fSetLocked( const b32 lock ) { mLocked = lock; }

		void				fSetShrink( const b32 shrink ) { mShrink = shrink; }
		b32					fShrink( ) const { return mShrink; }

		b32					fShrunk( ) const { return mShrink && mBounds.GetSize( ).y == 0; }
		void				fFastShrink( );
		wxPen&				fDashedLinePen( ) { return mDashedLinePen; }

		tKeyframePtr		fGetRawKeyframe( ) const { return mKeyframe; }

		static wxString			fGetKeyframeText( tKeyframePtr key );

	private:

		tKeyframePtr		mKeyframe;
		u32					mFrame;

		b32					mLocked;
		b32					mSelected;
		b32					mHighlighted;

		f32					mLastX;
		b32					mShrink;
		u32					mMiddleY;
		u32					mTopY;

		wxString			mText;
		wxSize				mTextSize;
		wxSize				mNormalSize;
		wxSize				mExtraSize;
		wxRect				mBounds;
		wxRect				mDrawRectangle;
		tSigFxKeyline*		mTheKeyline;
		tSigFxKeylineTrack*mParentTrack;
		tGraphPtr			mGraph;

		wxColour	mNormalColour;
		wxColour	mHighlightedColour;
		wxColour	mSelectedColour;

		wxFont		mFont;
		wxPen		mSolidPen;
		wxPen		mDashedLinePen;
		wxBrush		mSolidBrush;
	};


	typedef tRefCounterPtr< tSigFxKeyframe > tSigFxKeyframePtr;

}

#endif // __tSigFxKeyframe__