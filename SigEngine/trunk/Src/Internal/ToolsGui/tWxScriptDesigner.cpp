#include "ToolsGuiPch.hpp"
#include "..\Tools\Editor\tWxTextEditor.hpp"
#include "tWxScriptDesigner.hpp"
#include <wx/spinctrl.h>
#include "tWxSlapOnSpinner.hpp"

namespace Sig
{	
	namespace 
	{
		const s32 cColumns = 4;
		const wxColour cFontColorGroup( 255, 255, 255 );
		const wxColour cFontColor( 200, 200, 200 );
		const f32 cFloatIncrement = 0.01f;
		const s32 cFloatPrecision = 2;
	}

	tWxScriptDesigner::tWxScriptDesigner( 
		wxWindow* parent,
		tWxTextEditor* associatedText,
		wxWindowID id,
		const wxPoint& pos,
		const wxSize& size,
		long style,
		const wxString& name )
		: wxScrolledWindow( parent, id, pos, size, style, name )
		, mAssociatedText( associatedText )
	{
		const s32 vGap = 1;
		const s32 hGap = 5;
		const s32 labelWidth = 50;
		const s32 dataWidth = 100;
		wxFlexGridSizer *sizer = new wxFlexGridSizer( cColumns, vGap, hGap );
		SetSizer( sizer );
		sizer->AddGrowableCol( cColumns - 1, 1 );
		SetScrollbars( 1, 20, 1, 50 );
	}

	void tWxScriptDesigner::fParseScript( )
	{
		tFilePathPtr filename = mAssociatedText->fGetFilePath( );
		wxString text = mAssociatedText->GetText( );
		tDynamicBuffer scriptString;
		scriptString.fInsert( 0, (byte*)text.c_str( ), text.Length( ) );
		scriptString.fPushBack( 0 );

		const u32 fileBasedUniqueId = tScriptFileConverter::fGenerateUniqueFileBasedId( filename );
		const std::string fileBasedUniqueTag = tScriptFileConverter::fGenerateUniqueFileBasedTag( fileBasedUniqueId );

		tGrowableArray< std::string > groups;
		s32 currentGroup = -1;

		Freeze( );

		GetSizer( )->Clear( true );

		const s32 topMargin = 3;
		for( s32 i = 0; i < cColumns; ++i )
			GetSizer( )->AddSpacer( topMargin );

		mVariables.fSetCount( 0 );
		if( tScriptFileConverter::fParseAndReplaceSigvars( NULL, scriptString, fileBasedUniqueTag, mVariables, groups, true ) )
		{
			//populate controls
			mTextControls.fSetCount( mVariables.fCount( ) );

			if( mVariables.fCount( ) == 0 )
			{
				fPushGroup( "No designer data found" );
			}
			else
			{
				for( u32 i = 0; i < mVariables.fCount( ); ++i )
				{
					tScriptFileConverter::tExportedVariable& variable = mVariables[ i ];
					if( variable.mGroupIndex != currentGroup )
					{
						currentGroup = variable.mGroupIndex;
						fPushGroup( groups[ currentGroup ] );
					}

					switch( variable.mType )
					{
					case tScriptFileConverter::cVariableTypeEnum:	fPushComboBox( variable, i );	break;
					case tScriptFileConverter::cVariableTypeFloat:	fPushFloatSpin( variable, i );	break;
					case tScriptFileConverter::cVariableTypeInt:	fPushIntSpin( variable, i );	break;
					case tScriptFileConverter::cVariableTypeString: fPushTextField( variable, i );	break;
					case tScriptFileConverter::cVariableTypePath:	fPushFilename( variable, i );	break;
					case tScriptFileConverter::cVariableTypeVector:	fPushVector( variable, i );	break;
					}
				}
			}
		}

		GetSizer( )->Layout( );
		Thaw( );
	}

	void tWxScriptDesigner::fPushPad( s32 columnsUsed )
	{
		for( s32 i = columnsUsed; i < cColumns; ++i )
			GetSizer( )->AddSpacer( 10 );
	}

	void tWxScriptDesigner::fPushGroup( const wxString& groupName )
	{
		fPushPad( 0 );

		wxStaticText* groupLabel = new wxStaticText( this, wxID_ANY, std::string( " " ) + groupName );
		wxFont font = groupLabel->GetFont( );
		font.SetPointSize( 14 );
		groupLabel->SetFont( font );
		groupLabel->SetForegroundColour( cFontColorGroup );
		groupLabel->SetWindowStyle( wxALIGN_LEFT );

		wxSizerItem* item = GetSizer( )->Add( groupLabel, 1, wxEXPAND | wxALL );

		fPushPad( 1 );
	}

	wxStaticText* tWxScriptDesigner::fMakeLabel( const std::string& description )
	{
		wxStaticText* label = new wxStaticText( this, wxID_ANY, std::string( "       " ) + description + ":" );
		//label->SetWindowStyle( wxALIGN_RIGHT );
		label->SetForegroundColour( cFontColor );

		return label;
	}

	void tWxScriptDesigner::fPushTextField( const tScriptFileConverter::tExportedVariable& variable, u32 varIndex )
	{
		wxStaticText* label = fMakeLabel( variable.mDescription );

		std::string value = variable.mCurrentValue;
		wxTextCtrl* text = new wxTextCtrl( this, wxID_ANY, StringUtil::fStripQuotes( value ) );
		mTextControls[ varIndex ] = text;

		GetSizer( )->Add( label, 1, wxEXPAND | wxALL );
		GetSizer( )->Add( text, 1, wxEXPAND | wxALL );
		fPushPad( 2 );

		text->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( tWxScriptDesigner::fTextChanged ), NULL, this );
		text->SetClientData( (void*)varIndex );
	}

	void tWxScriptDesigner::fPushIntSpin( const tScriptFileConverter::tExportedVariable& variable, u32 varIndex )
	{
		tGrowableArray<std::string> options;
		StringUtil::fSplit( options, variable.mPotentialValues.c_str( ), ":" );

		if( options.fCount( ) != 2 )
		{
			log_warning( "Invalid range for int variable: " << variable.mExportedName );
			return;
		}

		s32 value = atoi( variable.mCurrentValue.c_str( ) );
		s32 min = atoi( options[0].c_str( ) );
		s32 max = atoi( options[1].c_str( ) );

		wxStaticText* label = fMakeLabel( variable.mDescription );
		GetSizer( )->Add( label, 1, wxEXPAND | wxALL );

		wxSpinCtrl* spin = new wxSpinCtrl( this, wxID_ANY );
		spin->SetMin( min );
		spin->SetMax( max );
		spin->SetValue( value );

		GetSizer( )->Add( spin, 1, wxEXPAND | wxALL );
		fPushPad( 2 );

		spin->Connect( wxEVT_SCROLL_THUMBTRACK, wxSpinEventHandler(tWxScriptDesigner::fIntChanged), NULL, this );
		spin->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(tWxScriptDesigner::fIntChanged), NULL, this );
		spin->SetClientData( (void*)varIndex );
	}

	void tWxScriptDesigner::fPushFloatSpin( const tScriptFileConverter::tExportedVariable& variable, u32 varIndex )
	{
		tGrowableArray<std::string> options;
		StringUtil::fSplit( options, variable.mPotentialValues.c_str( ), ":" );

		if( options.fCount( ) != 2 )
		{
			log_warning( "Invalid range for float variable: " << variable.mExportedName );
			return;
		}

		float value = atof( variable.mCurrentValue.c_str( ) );
		float min = atof( options[0].c_str( ) );
		float max = atof( options[1].c_str( ) );

		wxStaticText* label = fMakeLabel( variable.mDescription );
		GetSizer( )->Add( label, 1, wxEXPAND | wxALL );

		tWxSlapOnSpinner* spin = new tWxSlapOnSpinner( this, NULL, min, max, cFloatIncrement, cFloatPrecision );
		spin->fSetValue( value );

		//tWxSlapOnSpinner adds itself to sizer.
		fPushPad( 2 );

		spin->Connect( wxEVT_SCROLL_THUMBTRACK, wxSpinEventHandler( tWxScriptDesigner::fFloatChanged ), NULL, this );
		spin->SetClientData( (void*)varIndex );
	}

	void tWxScriptDesigner::fPushComboBox( const tScriptFileConverter::tExportedVariable& variable, u32 varIndex )
	{
		tGrowableArray<std::string> values;
		StringUtil::fSplit( values, variable.mPotentialValues.c_str( ), "," );
		s32 valueIndex = -1;

		for( u32 i = 0; i < values.fCount( ); ++i )
		{
			values[ i ] = StringUtil::fEatWhiteSpace( values[ i ] );
			if( values[ i ] == variable.mCurrentValue ) valueIndex = i;
		}

		wxStaticText* label = fMakeLabel( variable.mDescription );
		GetSizer( )->Add( label, 1, wxEXPAND | wxALL );

		wxComboBox* combo = new wxComboBox( this, wxID_ANY );
		for( u32 i = 0; i < values.fCount( ); ++i )
			combo->Append( values[ i ] );

		combo->Select( valueIndex );

		GetSizer( )->Add( combo, 1, wxEXPAND | wxALL );
		fPushPad( 2 );

		combo->Connect( wxEVT_COMMAND_COMBOBOX_SELECTED, wxCommandEventHandler(tWxScriptDesigner::fComboSelected), NULL, this );
		combo->SetClientData( (void*)varIndex );
	}

	void tWxScriptDesigner::fPushFilename( const tScriptFileConverter::tExportedVariable& variable, u32 varIndex )
	{
		wxStaticText* label = fMakeLabel( variable.mDescription );
		wxButton* button = new wxButton( this, wxID_ANY, "Browse" );

		std::string value = variable.mCurrentValue;
		wxTextCtrl* text = new wxTextCtrl( this, wxID_ANY, StringUtil::fStripQuotes( value ) );
		mTextControls[ varIndex ] = text;

		GetSizer( )->Add( label, 1, wxEXPAND | wxALL );
		GetSizer( )->Add( text, 1, wxEXPAND | wxALL );
		GetSizer( )->Add( button, 1, wxEXPAND | wxALL );
		fPushPad( 3 );

		button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(tWxScriptDesigner::fOnFileBrowse), NULL, this );
		button->SetClientData( (void*)varIndex );

		text->Connect( wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( tWxScriptDesigner::fTextChanged ), NULL, this );
		text->SetClientData( (void*)varIndex );
	}

	void tWxScriptDesigner::fPushVector( tScriptFileConverter::tExportedVariable& variable, u32 varIndex )
	{
		static const std::string vectorComponent[4] = { "X", "Y", "Z", "W" };
		variable.mUIElements.fSetCount( 0 );

		tGrowableArray<std::string> options;
		StringUtil::fSplit( options, variable.mPotentialValues.c_str( ), ":" );

		if( options.fCount( ) != 2 )
		{
			log_warning( "Invalid range for float variable: " << variable.mExportedName );
			return;
		}

		tGrowableArray<float> values = variable.fValues( );

		float min = atof( options[0].c_str( ) );
		float max = atof( options[1].c_str( ) );

		u32 count = fMin( values.fCount( ), 4u );
		for( u32 i = 0; i < count; ++i )
		{
			wxStaticText* label = fMakeLabel( variable.mDescription + " " + vectorComponent[ i ] );
			GetSizer( )->Add( label, 1, wxEXPAND | wxALL );

			tWxSlapOnSpinner* spin = new tWxSlapOnSpinner( this, NULL, min, max, cFloatIncrement, cFloatPrecision );
			spin->fSetValue( values[ i ] );

			//tWxSlapOnSpinner adds itself to sizer.
			fPushPad( 2 );

			spin->Connect( wxEVT_SCROLL_THUMBTRACK, wxSpinEventHandler( tWxScriptDesigner::fVectorChanged ), NULL, this );
			spin->SetClientData( (void*)varIndex );
			variable.mUIElements.fPushBack( (void*) spin );
		}
	}

	void tWxScriptDesigner::fOnFileBrowse( wxCommandEvent& event )
	{
		wxButton* button = static_cast<wxButton*>( event.GetEventObject( ) );
		u32 index = (u32)button->GetClientData( );
		tScriptFileConverter::tExportedVariable& variable = mVariables[ index ];

		std::string path = variable.mCurrentValue;
		StringUtil::fStripQuotes( path );

		tFilePathPtr totalPath = ToolsPaths::fMakeResAbsolute( tFilePathPtr( path.c_str( ) ) );
		std::string folder = StringUtil::fDirectoryFromPath( totalPath.fCStr( ) );
		path = StringUtil::fNameFromPath( totalPath.fCStr( ) );

		wxFileDialog *diag = new wxFileDialog( this, "Browse..", folder, path, variable.mPotentialValues, wxFD_OPEN );

		if( diag->ShowModal( ) == wxID_OK )
		{
			path = std::string( ToolsPaths::fMakeResRelative( tFilePathPtr( diag->GetPath( ).c_str( ) ) ).fCStr( ) );
			path = StringUtil::fReplaceAllOf( path, "\\", "/" );
			mTextControls[ index ]->SetValue( path );			
		}

		delete diag;
	}

	void tWxScriptDesigner::fTextChanged( wxCommandEvent& event )
	{
		u32 index = (u32)event.GetClientData( );
		tScriptFileConverter::tExportedVariable& variable = mVariables[ index ];
		variable.mCurrentValue = std::string( "\"" ) + static_cast<wxTextCtrl*>( event.GetEventObject( ) )->GetValue( ) + std::string( "\"" );

		fPostVariable( variable );
	}

	void tWxScriptDesigner::fComboSelected( wxCommandEvent& event )
	{
		u32 index = (u32)event.GetClientData( );
		tScriptFileConverter::tExportedVariable& variable = mVariables[ index ];
		variable.mCurrentValue = static_cast<wxComboBox*>( event.GetEventObject( ) )->GetValue( );

		fPostVariable( variable );
	}

	void tWxScriptDesigner::fIntChanged( wxSpinEvent& event )
	{
		wxSpinCtrl* spin = static_cast<wxSpinCtrl*>( event.GetEventObject( ) );
		tScriptFileConverter::tExportedVariable& variable = mVariables[ (u32)spin->GetClientData( ) ];
		variable.mCurrentValue = StringUtil::fToString( spin->GetValue( ) );

		fPostVariable( variable );
	}

	void tWxScriptDesigner::fIntChanged( wxCommandEvent& event )
	{
		wxSpinCtrl* spin = static_cast<wxSpinCtrl*>( event.GetEventObject( ) );
		tScriptFileConverter::tExportedVariable& variable = mVariables[ (u32)spin->GetClientData( ) ];
		variable.mCurrentValue = StringUtil::fToString( spin->GetValue( ) );

		fPostVariable( variable );
	}

	void tWxScriptDesigner::fFloatChanged( wxSpinEvent& event )
	{
		u32 index = (u32)event.GetClientData( );
		tScriptFileConverter::tExportedVariable& variable = mVariables[ index ];
		variable.mCurrentValue = StringUtil::fToString( static_cast<tWxSlapOnSpinner*>( event.GetEventObject( ) )->fGetValue( ) );

		fPostVariable( variable );
	}

	void tWxScriptDesigner::fVectorChanged( wxSpinEvent& event )
	{
		u32 index = (u32)event.GetClientData( );
		tScriptFileConverter::tExportedVariable& variable = mVariables[ index ];

		variable.mCurrentValue = "(";
		for( u32 i = 0; i < variable.mUIElements.fCount( ); ++i )
		{
			tWxSlapOnSpinner* spinner = static_cast<tWxSlapOnSpinner*>( variable.mUIElements[ i ] );
			variable.mCurrentValue += StringUtil::fToString( spinner->fGetValue( ) );
			if( i != variable.mUIElements.fCount( )-1 )
				variable.mCurrentValue += ",";
		}

		variable.mCurrentValue += ")";

		fPostVariable( variable );
	}

	void tWxScriptDesigner::fPostVariable( const tScriptFileConverter::tExportedVariable& variable )
	{
		std::string text = mAssociatedText->GetText( );

		std::string newLine = variable.fGenerateLine( );

		StringUtil::fReplaceLine( text, newLine.c_str( ), variable.mLineNumber );

		mAssociatedText->SetText( text );
	}

	BEGIN_EVENT_TABLE(tWxScriptDesigner, wxPanel)
		//EVT_STC_SAVEPOINTREACHED(	wxID_ANY,	tWxTextEditor::fSavepointReached )
		//EVT_STC_SAVEPOINTLEFT(		wxID_ANY,	tWxTextEditor::fSavepointLeft )
		//EVT_STC_MODIFIED(			wxID_ANY,	tWxTextEditor::fModified )
		//EVT_STC_CHARADDED(			wxID_ANY,	tWxTextEditor::fCharacterAdded )
	END_EVENT_TABLE()
}
