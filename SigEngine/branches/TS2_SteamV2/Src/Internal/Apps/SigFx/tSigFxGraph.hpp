#ifndef __tSigFxGraph__
#define __tSigFxGraph__

#include "FX/tFxGraph.hpp"
#include "wx/dcbuffer.h"
#include "tRefCounterPtr.hpp"

namespace Sig
{
	using namespace FX;
	class tSigFxMainWindow;
	class tGraphButton;

	class tSigFxGraph : public tRefCounter
	{
	public:
		tSigFxGraph( tGraphPtr* graph, tEntityPtr entity, const wxString& name, tSigFxMainWindow* guiApp );
		~tSigFxGraph( );

		u32 fNumKeyframes( ) { return (*mGraph)->fNumKeyframes( ); }
		tKeyframePtr fRawKeyframe( u32 idx ) { return (*mGraph)->fKeyframe( idx ); }
		
		void fBuildValues( ) { (*mGraph)->fBuildValues( ); }
		tGraphPtr* fRawGraph( ) const { return mGraph; }

		void fMouseMove( wxMouseEvent& event );
		void fMouseButtonDown( wxMouseEvent& event );
		void fOnMouseButtonDoubleClick( wxMouseEvent& event );
		void fMouseButtonUp( wxMouseEvent& event );
		void fOnMouseWheel( wxMouseEvent& event );
		void fOnKeyDown( wxKeyEvent& event );
		void fPaint( wxAutoBufferedPaintDC& dc, wxSize& panelSize, f32 KeylineDelta, const tGrowableArray< tGraphButton* >& graphButtons );
		void fFrame( );
		void fRefresh( );

		u32 fGetNumberIndices( );
		f32 fValue( tKeyframePtr key, u32 idx = 0 ) const;
		f32 fSampleGraph( f32 x, u32 idx = 0 ) const;

		void fSetValue( tKeyframePtr key, f32 val, u32 idx = 0 ) const;

		static void fSetOldGraph( tGraphPtr graph, tGraphPtr* old );

		f32 fMaximumNodeDifference( );
		f32 fGraphHeight( );

		void fClampHeightAtOne( b32 clamp ) { mHeightClamped = clamp; }
		void fAllowScrolling( b32 scroll ) { mAllowScroll = scroll; }
		void fShowScrub( b32 show ) { mShowScrub = show; }
		
		void fSetAsColorGraph( ) { mColorGraph = true; }
		b8 fColorGraph( ) const { return mColorGraph; }

		void fSetAsSpatialGraph( ) { mSpatialGraph = true; }
		b32 fSpatialGraph( ) const { return mSpatialGraph; }

		void fMarkAsDirty( ) { mDirty = true; }

		void fClear( );
		void fAddKeyframe( tKeyframePtr key );

		void fFlipX( );
		void fFlipY( );

		f32 fMaxY( ) const { return mMaxY; }
		void fSetGraphTotalMaximumValue( f32 maximumYValue ) { mGraphTotalMaximumValue = maximumYValue; }

		tSigFxMainWindow* fSigFxMainWindow( ) { return mMainWindow; }

	private:
		tGraphPtr*				mGraph;
		tGraphPtr				mOldGraph;
		tEntityPtr				mAssociatedEntity;
		wxString				mGraphName;
		tSigFxMainWindow*		mMainWindow;

		static wxFont			mFont;

		wxBitmap				mBackgroundBitmap;
		wxBitmap				mMaskBitmap;
		wxBitmap				mGraphBitmap;

		b32						mDirty;
		wxRect					mBounds;		

		f32	mMinX, mMinY, mMaxX, mMaxY;
		b8 mHeightClamped;
		b8 mAllowScroll;
		b8 mShowScrub;
		b8 mColorGraph;

		b32 mSpatialGraph;

		wxPoint mLastMousePos;
		b32 mDoSelection;
		wxRect mSelectionBox;
		wxPoint	mSelectionRectangleAnchor;

		enum
		{
			cMouseMoveNormal,
			cMouseMoveVertical,
			cMouseMoveHorizontal,
			cCount,
		} mMouseMovement;

		
		b32 mMultiSelect;
		b32 mMovingKeyframes;
		b32 mCopyKeyframes;
		f32 mElevatorOffset;
		f32 mGraphTotalMaximumValue;

		struct tNode
		{
			u32 mIdx;
			Math::tVec2f mPos;
			tKeyframePtr mKey;
			f32 mNodeRadius;
		};

		
		struct tDrawNodeSort
		{
			inline b32 operator( )( const tNode& a, const tNode& b ) const
			{
				if( a.mKey == b.mKey )
					return a.mIdx < b.mIdx;
				return a.mKey->fX( ) < b.mKey->fX( );
			}
			inline b32 operator( )( const tNode* a, const tNode* b ) const
			{
				if( a->mKey == b->mKey )
					return a->mIdx < b->mIdx;
				return a->mKey->fX( ) < b->mKey->fX( );
			}
		};


		tNode* mHighlightedNode;
		tGrowableArray< tNode* > mDrawNodePoints;	// this list is resonsible for deleting its tNodes.
		tGrowableArray< tNode* > mSelectedNodes;
		tGrowableArray< tNode* > mNewlySelectedNodes;
		tGrowableArray< tNode* > mNewKeys;
	};


	typedef tRefCounterPtr< tSigFxGraph > tSigFxGraphPtr;

}

#endif // __tSigFxGraph__

