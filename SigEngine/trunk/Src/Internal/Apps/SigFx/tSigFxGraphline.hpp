#ifndef __tSigFxGraphline__
#define __tSigFxGraphline__

#include "tTabPanel.hpp"
#include "FX/tFxGraph.hpp"
#include "FX/tParticleSystem.hpp"
#include "tSigFxGraph.hpp"
#include "tSigFxFile.hpp"
#include "wx/dcbuffer.h"
#include "tSigFxKeyline.hpp"
#include "tWxSlapOnSpinner.hpp"


namespace Sig
{

	class tGraphButton
	{
	public:
		
		tGraphButton( );
		tGraphButton( wxPoint pos, wxSize size, wxString text, wxColour col, wxColour hil, wxColour sel );

		void fPaint( wxAutoBufferedPaintDC& dc );
		b32 fMouseOver( wxMouseEvent& event );
		b32 fMouseClick( wxMouseEvent& event );

		wxString fGetText( ) const { return mText; }

		tSigFxGraph* fGraph( ) const { return mGraph; }
		void fSetGraph( tSigFxGraph* g ) { mGraph = g; }

		void fSetSelected( b32 sel ) { mSelected = sel; }

	private:

		b32 mMouseOver;
		b32 mSelected;
		wxPoint mPosition;
		wxSize mSize;
		wxString mText;
		wxColour mColor;
		wxColour mHighlight;
		wxColour mSelectedCol;

		static wxFont mFontNom;
		static wxFont mFontHil;
		static wxFont mFontSel;

		tSigFxGraph* mGraph;
	};





	class tRandomnessSpinner : public tWxSlapOnSpinner
	{
	public:
		tRandomnessSpinner( wxWindow* parent, const std::string& label, f32 min, f32 max, f32 increment, u32 precision )
			: tWxSlapOnSpinner( parent, label.c_str( ), min, max, increment, precision )
		{

		}

		virtual void fOnControlUpdated( )
		{
			
		}
	};
	

	class tRandomnessWindow : public wxDialog
	{
		tSigFxGraph* mGraph;

		tRandomnessSpinner* mMinSpinner;
		tRandomnessSpinner* mMaxSpinner;

	public:

		tRandomnessWindow( wxWindow* parent, tSigFxGraph* graph, wxPoint pos )
			: wxDialog( parent, wxID_ANY, wxString( "Sample Randomness" ), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE	| wxNO_3D )
			, mGraph( graph )
		{
			SetPosition( wxPoint( pos.x, pos.y - GetSize( ).y/2 ) );

			wxBoxSizer* main = new wxBoxSizer( wxVERTICAL );

			wxStaticText* header = new wxStaticText(  this, wxID_ANY, wxString( "Randomness" ) );
			main->Add( header, 0, wxEXPAND | wxALL, 8 );

			wxPanel* group = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
			mMaxSpinner = new tRandomnessSpinner( group, "Maximum:", -1000, 1000, 0.01f, 2 );
			mMinSpinner = new tRandomnessSpinner( group, "Minimum:", -1000, 1000, 0.01f, 2 );
			main->Add( group, 0, wxEXPAND | wxALL, 8 );

			wxButton* butt = new wxButton( this, wxID_ANY, wxString( "Okay" ) );
			main->Add( butt, 0, wxEXPAND | wxALL, 8 );
			
			Connect( wxEVT_KEY_DOWN, wxKeyEventHandler( tRandomnessWindow::fOnKeyDown ) );
			butt->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tRandomnessWindow::fOnOkay ), NULL, this );

			SetSizer( main );
			
			main->SetSizeHints( this );

			f32 minRandomness = ( *mGraph->fRawGraph( ) )->fMinRandomness( );
			f32 maxRandomness = ( *mGraph->fRawGraph( ) )->fMaxRandomness( );

			mMinSpinner->fSetValue( minRandomness );
			mMaxSpinner->fSetValue( maxRandomness );
		}

		void fOnKeyDown( wxKeyEvent& event )
		{
			const int keyCode = event.GetKeyCode( );
			if( keyCode == WXK_ESCAPE )
				Close( );
		}

		void fOnOkay( wxCommandEvent& event )
		{
			f32 min = mMinSpinner->fGetValue( );
			f32 max = mMaxSpinner->fGetValue( );
			if( min > max )
				fSwap( min, max );

			( *mGraph->fRawGraph( ) )->fSetMinRandomness( min );
			( *mGraph->fRawGraph( ) )->fSetMaxRandomness( max );
			( *mGraph->fRawGraph( ) )->fBuildValues( );

			mGraph->fFrame( );
			Close( );
		}

	};


	class tGenerateSpinMenu : public wxMenu
	{
		tSigFxGraph* mGraph;
		u32 mMenuId;

	public:

		enum
		{
			cSpinOptions = 10,
		};

		static u32 mSpinCounts[ cSpinOptions ];

		tGenerateSpinMenu( tSigFxGraph* graph, u32 id )
			: wxMenu( wxString( "" ) )
			, mGraph( graph )
			, mMenuId( id )
		{
			for( u32 i = 0; i < cSpinOptions; ++i )
			{
				Append( mMenuId+i, wxString::Format( "%i", mSpinCounts[ i ] ) );
			}
		}	
	};



	class tGraphlineContextMenu : public wxMenu
	{
		tSigFxGraph* mGraph;
		wxPoint mMouseClick;

		enum
		{
			cFrame,
			cSetRandomness,
			cAllOnes,
			cAllZeroes,
			cEvenSpacing,
			cXAxis,
			cYAxis,
			cZAxis,
			cWAxis,
			cNormalize,
			cGenerateXLine,
			cGenerateYLine,
			cGenerateZLine,
			cFlipX,
			cFlipY,
			cXSpin,
			cYSpin = cXSpin + tGenerateSpinMenu::cSpinOptions,
			cZSpin = cYSpin + tGenerateSpinMenu::cSpinOptions,
			cCopyGraph = cZSpin + tGenerateSpinMenu::cSpinOptions,
			cPasteGraph,
		};

	public:
		tGraphlineContextMenu( tSigFxGraph* graph, wxPoint clickPos );
		void fOnMenuItemSelected( wxCommandEvent& event );
	};

	class tSigFxGraphline : public tTabPanel
	{
	public:
		tSigFxGraphline( wxNotebook* parent, tSigFxKeyline* Keyline );
		virtual ~tSigFxGraphline( );

		virtual void fOnTick( f32 dt );
			
		virtual void fSaveInternal( HKEY hKey ) { }
		virtual void fLoadInternal( HKEY hKey ) { }

		void fBuildPageFromEntities( tEditorSelectionList& selected );

		void fOnSize( wxSizeEvent& event );
		void fOnPaint( wxPaintEvent& event );
		void fOnEraseBackground( wxEraseEvent& event );
		void fOnMouseMove( wxMouseEvent& event );
		void fOnMouseButtonDown( wxMouseEvent& event );
		void fOnMouseButtonDoubleClick( wxMouseEvent& event );

		void fOnRightClick( wxMouseEvent& event );
		void fOnKeyDown( wxKeyEvent& event );
		
		void fOnMouseButtonUp( wxMouseEvent& event );
		void fOnMouseWheel( wxMouseEvent& event );

		void fOnMouseLeaveWindow( wxMouseEvent& event );
		void fOnMouseEnterWindow( wxMouseEvent& event );

		void fErasePreviousGraphs( );
		void fRefreshNodes( );

		tSigFxGraph* fActiveGraph( ) { return mActiveGraph; }
		void fSetActiveGraph( tGraphPtr graph );

		void fFrame( );
	private:

		s32 fGraphIndex( const tGraphPtr& graph );

		tEntityPtr mSelectedEntity;

		tGraphButton* mSelectedGraphButton;
		tGrowableArray< tGraphButton* > mGraphButtons;

		tSigFxKeyline* mTheKeyline;

		tSigFxGraph* mActiveGraph;
		tGrowableArray< tSigFxGraph* > mGraphs;

		b32 mForceMouseInput;
		b32	mContinueMouseInput;

		b32 mPreviousGraphIndexMode;
		u32 mPreviousGraphIndex;
	};



}


#endif // __tSigFxGraphline__