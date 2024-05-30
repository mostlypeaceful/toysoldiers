#include "ToolsPch.hpp"
#include "tExporterToolbox.hpp"
#include "tExporterGuiFactory.hpp"
#include "tProjectFile.hpp"

namespace Sig
{

	void tExporterToolbox::fNameAnimationSources( u32 i, std::string & out )
	{
		std::stringstream ss;
		ss << fNameAnimationSources( );
		ss << i;
		out = ss.str( );
	}

	BEGIN_EVENT_TABLE( tExporterToolbox, wxFrame )
		EVT_CLOSE( tExporterToolbox::fOnClose )
	END_EVENT_TABLE()

	tExporterToolbox::tExporterToolbox( 
		const char* windowTitle, 
		const tStrongPtr<tExporterGuiFactory>& factory )
		: tWxSlapOnDialog( windowTitle )
	{
		SetBackgroundColour( wxColour( 0x55, 0x55, 0x55 ) );
		SetForegroundColour( wxColour( 0xff, 0xff, 0xff ) );

		sigassert( factory );

		tWxSlapOnTabSet* tabSet = factory->fCreateTabSet( this );

		fSetupMainTab( factory.fGetRawPtr( ), factory->fCreatePanel( tabSet, "Main" ) );
		fSetupGeometryTab( factory.fGetRawPtr( ), factory->fCreatePanel( tabSet, "Geometry" ) );
		fSetupAnimationTab( factory.fGetRawPtr( ), factory->fCreatePanel( tabSet, "Animation" ) );
		fSetupMaterialTab( factory.fGetRawPtr( ), factory->fCreatePanel( tabSet, "Material" ) );

		//
		// add status bar...
		//

		const int statusWidths[] = { 0, -1 };
		const int statusStyles[] = { wxSB_FLAT, wxSB_FLAT };
		wxStatusBar* statusBar = CreateStatusBar( array_length(statusWidths), wxST_SIZEGRIP|wxFULL_REPAINT_ON_RESIZE, 0, wxT("Active Profile:") );
		statusBar->SetStatusStyles( array_length(statusStyles), statusStyles );
		SetStatusWidths( array_length(statusWidths), statusWidths );
		SetStatusText( "Nothing selected.", 1 );
		statusBar->SetBackgroundColour( GetBackgroundColour( ) );
		statusBar->SetForegroundColour( GetForegroundColour( ) );
	}

	void tExporterToolbox::fSetupMainTab( tExporterGuiFactory* factory, tWxSlapOnPanel* tab )
	{
		//factory->fCreateCheckBoxAttribute( tab, fNameIgnoreObject( ) );
		factory->fCreateCheckOutButton( tab, "Check-Out Maya File" );
		factory->fCreateTexturePathFixerButton( tab, "RePath Res Textures" );
		factory->fCreateQuickExport( tab, "Export File" );
	}

	void tExporterToolbox::fSetupGeometryTab( 
		tExporterGuiFactory* factory, 
		tWxSlapOnPanel* tab )
	{
		tGrowableArray<wxString> enumTypeNames;

		enumTypeNames.fSetCount( 0 );
		enumTypeNames.fPushBack( fNameGeomTypeNone( ) );
		enumTypeNames.fPushBack( fNameGeomTypeRenderMesh( ) );
		factory->fCreateChoiceAttribute( tab, fNameGeomType( ), enumTypeNames.fBegin( ), enumTypeNames.fCount( ), 1 );

		enumTypeNames.fSetCount( 0 );
		enumTypeNames.fPushBack( fNameStateState( ) );
		enumTypeNames.fPushBack( fNameStateTransition( ) );
		factory->fCreateChoiceAttribute( tab, fNameStateType( ), enumTypeNames.fBegin( ), enumTypeNames.fCount( ) );
		factory->fCreateSpinnerAttribute( tab, fNameStateIndex( ), -1, 254, 1, 0 );

		
		// Project specific entries
		{
			const tProjectFile & projectFile = tProjectFile::fGetCurrentProjectFileCached( );

			// Game flags
			const u32 flagCount = projectFile.mGameTags.fCount( );
			enumTypeNames.fSetCount( flagCount );
			for( u32 f = 0; f < flagCount; ++f )
				enumTypeNames[ f ] = projectFile.mGameTags[ f ].mName.c_str( );

			factory->fCreateMultiChoiceAttribute( 
				tab, fNameGameTags( ), enumTypeNames.fBegin( ), enumTypeNames.fCount( ) );
		}
		
		//factory->fCreateMaskAttribute( tab, fNameStateMask( ), 16, (1<<0) );

		//factory->fCreateSpinnerAttribute( tab, fNameLodSlot( ), 1, 10, 1, 0 );
	}

	void tExporterToolbox::fSetupAnimationTab( tExporterGuiFactory* factory, tWxSlapOnPanel* tab )
	{
		factory->fCreateCheckBoxAttribute( tab, fNameReferenceFrame( ) );
		factory->fCreateTextBoxAttribute( tab, fNameRootNodeName( ) );
		factory->fCreateCheckBoxAttribute( tab, fNameExcludeBone( ) );
		factory->fCreateCheckBoxAttribute( tab, fNameAdditiveBone( ) );
		factory->fCreateSpinnerAttribute( tab, fNameIKBonePriority( ), 0.f, 1000.f, 1, 0 );
		factory->fCreateAnimSourceSkeletonsListBox( tab, fNameAnimationSources( ) );
	}

	void tExporterToolbox::fSetupMaterialTab( tExporterGuiFactory* factory, tWxSlapOnPanel* tab )
	{
		factory->fCreateCheckBoxAttribute( tab, fNameTwoSided( ) );
		factory->fCreateCheckBoxAttribute( tab, fNameFlipBackFaceNormal( ) );
		factory->fCreateCheckBoxAttribute( tab, fNameTransparency( ) );
		factory->fCreateSpinnerAttribute( tab, fNameAlphaCutOut( ), 0, 255, 1, 0 );
		factory->fCreateCheckBoxAttribute( tab, fNameAdditive( ) );

		tGrowableArray<wxString> enumTypeNames;

		enumTypeNames.fSetCount( 0 );
		enumTypeNames.fPushBack( fNameZBufferTestDefault( ) );
		enumTypeNames.fPushBack( fNameZBufferTestForceOn( ) );
		enumTypeNames.fPushBack( fNameZBufferTestForceOff( ) );
		factory->fCreateChoiceAttribute( tab, fNameZBufferTest( ), enumTypeNames.fBegin( ), enumTypeNames.fCount( ) );

		enumTypeNames.fSetCount( 0 );
		enumTypeNames.fPushBack( fNameZBufferWriteDefault( ) );
		enumTypeNames.fPushBack( fNameZBufferWriteForceOn( ) );
		enumTypeNames.fPushBack( fNameZBufferWriteForceOff( ) );
		factory->fCreateChoiceAttribute( tab, fNameZBufferWrite( ), enumTypeNames.fBegin( ), enumTypeNames.fCount( ) );

		factory->fCreateCheckBoxAttribute( tab, fNameFaceX( ) );
		factory->fCreateCheckBoxAttribute( tab, fNameFaceY( ) );
		factory->fCreateCheckBoxAttribute( tab, fNameFaceZ( ) );

		factory->fCreateSpinnerAttribute( tab, fNameSortOffset( ), -10000.f, +10000.f, 0.1f, 1 );
	}

	void tExporterToolbox::fSaveLayout( )
	{
		tExporterToolboxLayout layout;
		layout.fFromWxWindow( this );

		if( layout.fIsInBounds( 2048 ) && layout != mLayout )
		{
			layout.fSave( );
			mLayout = layout;
		}
	}

	void tExporterToolbox::fLoadLayout( )
	{
		mLayout.fLoad( );

		mLayout.fToWxWindow( this );
	}

	void tExporterToolbox::fOnClose(wxCloseEvent& event)
	{
		Show( false );
	}

}
