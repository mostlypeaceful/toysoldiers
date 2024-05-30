#include "SigEdPch.hpp"
#include "tEditorAppWindow.hpp"
#include "tEditableObjectProperties.hpp"
#include "FileSystem.hpp"
#include "tProjectFile.hpp"
#include "tWxSlapOnGroup.hpp"
#include "tWxSelectStringDialog.hpp"
#include "tWxSlapOnTextBox.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "Editor/tEditablePropertyTypes.hpp"
#include "Sigml.hpp"
#include "Sklml.hpp"

namespace Sig
{
	namespace
	{
		static const wxString cPropertyTypeChoices[]=
		{
			"Tag",
			"Enum",
		};
		enum
		{
			cPropertyTypeTag,
			cPropertyTypeEnum,
		};
	}

	enum tEditableObjectPropsToolBarActions
	{
		cEOPAddProperty = 1
	};

	BEGIN_EVENT_TABLE(tEditableObjectProperties, wxFrame)
		EVT_MENU(		cEOPAddProperty,	tEditableObjectProperties::fOnAction )
	END_EVENT_TABLE()

	tEditableObjectProperties::tEditableObjectProperties( tEditorAppWindow* editorWindow, b32 globalProps )
		: tEditorDialog( editorWindow, globalProps ? "EditableGlobalPropertiesWindow" : "EditableObjectPropertiesWindow" )
		, mMainPanel( 0 )
		, mUserPropType( 0 )
		, mUserPropName( 0 )
		, mDoGlobalProps( globalProps )
	{
		// enforce width constraints on window
		const int width = 376;
		SetMinSize( wxSize( width, -1 ) );
		SetMaxSize( wxSize( width, -1 ) );
		SetSize( wxSize( width, -1 ) );

		// set property event callbacks
		mOnGlobalPropertyChanged.fFromMethod< tEditableObjectProperties, &tEditableObjectProperties::fNotifyGlobalPropertyChanged >( this );
		mGlobalProperties.mOnPropertyChanged.fAddObserver( &mOnGlobalPropertyChanged );
		mOnPropertyWantsRemoval.fFromMethod< tEditableObjectProperties, &tEditableObjectProperties::fRemoveProperty >( this );
		mCommonProps.mOnPropertyWantsRemoval.fAddObserver( &mOnPropertyWantsRemoval );
		mOnCommonPropertyChanged.fFromMethod< tEditableObjectProperties, &tEditableObjectProperties::fNotifyCommonPropertyChanged >( this );
		mCommonProps.mOnPropertyChanged.fAddObserver( &mOnCommonPropertyChanged );
		mBrowseForBoneName.fFromMethod< tEditableObjectProperties, &tEditableObjectProperties::fBrowseForBoneName >( this );
		mBrowseForSkeletonBindingName.fFromMethod< tEditableObjectProperties, &tEditableObjectProperties::fBrowseForSkeletonBindingName >( this );

		// set global filename browser filters
		tEditablePropertyFileNameString::fAddFilter( Sigml::tObjectProperties::fEditablePropScriptName( ), "*.nut" );
		tEditablePropertyFileNameString::fAddFilter( Sigml::tObjectProperties::fEditablePropSkeletonName( ), "*.sklml" );

		tEditablePropertyCustomString::tCallbackData customBrowse;
		customBrowse.mFunc = mBrowseForBoneName;
		tEditablePropertyCustomString::fAddCallback( Sigml::tObjectProperties::fEditablePropBoneAttachment( ), customBrowse );
		customBrowse.mFunc = mBrowseForSkeletonBindingName;
		tEditablePropertyCustomString::fAddCallback( Sigml::tObjectProperties::fEditablePropSkeletonBindingName( ), customBrowse );

		wxToolBar* toolBar = new wxToolBar( this, wxID_ANY );

		mUserPropType = new wxChoice( toolBar, wxID_ANY, wxDefaultPosition, wxDefaultSize, array_length( cPropertyTypeChoices ), cPropertyTypeChoices );
		mUserPropName = new wxChoice( toolBar, wxID_ANY, wxDefaultPosition, wxDefaultSize );

		toolBar->AddSeparator( );
		toolBar->AddControl( new wxStaticText( toolBar, wxID_ANY, "Property Type:" ) );
		toolBar->AddControl( mUserPropType );
		toolBar->AddSeparator( );
		toolBar->AddControl( new wxStaticText( toolBar, wxID_ANY, "Name:" ) );
		toolBar->AddControl( mUserPropName );
		toolBar->AddTool( cEOPAddProperty, "Add Tag", wxBitmap( "newdoc" ), wxNullBitmap, wxITEM_NORMAL, "Add property to selected objects" );
		mUserPropType->Select( 0 );
		toolBar->Realize( );

		mUserPropType->Connect( wxEVT_COMMAND_CHOICE_SELECTED, wxCommandEventHandler( tEditableObjectProperties::fOnAction ), NULL, this );

		mMainPanel = new wxScrolledWindow( this );

		fSetAppearance( mDoGlobalProps );

		SetSizer( new wxBoxSizer( wxVERTICAL ) );
		GetSizer( )->Add( toolBar, 0, wxEXPAND | wxALL, 0 );
		GetSizer( )->Add( mMainPanel, 1, wxEXPAND | wxALL, 0 );

		fClear( true );
		Layout( );
		Refresh( );
		fRefreshUserPropertyNames( );
	}

	void tEditableObjectProperties::fSetGlobalProperties( const tEditablePropertyTable& globalProps )
	{
		mGlobalProperties = globalProps;
		fEditorWindow( )->fGuiApp( ).fSyncDefaultLight( mGlobalProperties );

		// refresh bone names
		tEditablePropertyPtr* sklmlName = mGlobalProperties.fFind( Sigml::tObjectProperties::fEditablePropSkeletonName( ) );
		if( sklmlName && *sklmlName )
			fRefreshBoneNames( **sklmlName );

		fOnSelectionChanged( );
	}

	void tEditableObjectProperties::fOnSelectionChanged( )
	{
		mMainPanel->Freeze( );

		const tEditorSelectionList& selection = fEditorWindow( )->fGuiApp( ).fSelectionList( );
		const b32 nullSelection = ( selection.fCount( ) == 0 );
		fClear( );

		fSetAppearance( mDoGlobalProps );

		if( mDoGlobalProps )
		{
			mCommonProps = mGlobalProperties;
			mCommonProps.fCollectCommonPropertiesForGui( mGlobalProperties );
		}
		else if( nullSelection )
		{
			mMainPanel->SetScrollbars( 0, 0, 1, 1 );
			mMainPanel->Layout( );
			mMainPanel->Thaw( );
			return;
		}
		else
		{
			// get intersection of all properties
			tEditableObject* obj = selection[ 0 ]->fDynamicCast< tEditableObject >( );
			mCommonProps = obj->fGetEditableProperties( );
			for( u32 i = 0; i < selection.fCount( ); ++i )
				mCommonProps.fCollectCommonPropertiesForGui( selection[ i ]->fDynamicCast< tEditableObject >( )->fGetEditableProperties( ) );

			tWxSlapOnTextBox *layerName = new tWxSlapOnTextBox( mMainPanel, "Layer Name:" );
			layerName->fSetValue( obj->fLayer( ) );
			layerName->fDisableControl( );
		}

		tEditablePropertyTable::fAddPropsToWindow( mMainPanel, mCommonProps );

		mMainPanel->SetScrollbars( 0, 20, 1, 50 );
		mMainPanel->Layout( );
		mMainPanel->Thaw( );
	}

	void tEditableObjectProperties::fAddProperty( tEditablePropertyTable& table, int propType, const std::string& name )
	{
		tEditablePropertyPtr* find = table.fFind( name );
		if( !find )
		{
			switch( propType ) // this switch must match up with the array of strings passed to the wxChoice combo box in the constructor
			{
			case cPropertyTypeTag:
				table.fInsert( tEditablePropertyPtr( new tEditablePropertyProjectFileTag( name, true ) ) );
				break;
			case cPropertyTypeEnum:
				table.fInsert( tEditablePropertyPtr( new tEditablePropertyProjectFileEnum( name, 0, true ) ) );
				break;
			//case 1: table.fInsert( tEditablePropertyPtr( new tEditablePropertyString( name, true, false ) ) ); break;
			//case 2: table.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( name, 0.f, -99999.f, +99999.f, 0.01f, 2, true ) ) ); break;
			}
		}
	}

	void tEditableObjectProperties::fSetAppearance( b32 globalProps )
	{
		if( globalProps )
		{
			SetIcon( wxIcon( "globeicon" ) );
			SetTitle( "Global Properties" );
			mMainPanel->SetBackgroundColour( wxColour( 0x55, 0x55, 0x66 ) );
			mMainPanel->SetForegroundColour( wxColour( 0xff, 0xff, 0xff ) );
		}
		else
		{
			SetIcon( wxIcon( "appicon" ) );
			SetTitle( "Selected Object Properties" );
			mMainPanel->SetBackgroundColour( wxColour( 0xee, 0xee, 0xee ) );
			mMainPanel->SetForegroundColour( wxColour( 0x33, 0x33, 0x55 ) );
		}
	}

	void tEditableObjectProperties::fRemoveProperty( tEditableProperty& property )
	{
		tEditorSelectionList& selected = fEditorWindow( )->fGuiApp( ).fSelectionList( );
		if( mDoGlobalProps )
		{
			mGlobalProperties.fRemove( property.fGetName( ) );
		}
		else
		{
			for( u32 i = 0; i < selected.fCount( ); ++i )
			{
				tEditableObject* eo = selected[ i ]->fDynamicCast< tEditableObject >( );
				sigassert( eo );
				eo->fGetEditableProperties( ).fRemove( property.fGetName( ) );
			}
		}

		fOnSelectionChanged( );

		fEditorWindow( )->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
	}

	void tEditableObjectProperties::fClear( b32 refreshScrollBars )
	{
		mCommonProps.fClearGui( );
		mCommonProps = tEditablePropertyTable( );

		mMainPanel->DestroyChildren( );

		if( refreshScrollBars )
			mMainPanel->SetScrollbars( 0, 0, 1, 1 );
	}

	void tEditableObjectProperties::fOnAction(wxCommandEvent& event)
	{
		if( event.GetEventType( ) == wxEVT_COMMAND_CHOICE_SELECTED )
		{
			fRefreshUserPropertyNames( );
		}
		else if( event.GetId( ) == cEOPAddProperty )
		{
			tEditorSelectionList& selected = fEditorWindow( )->fGuiApp( ).fSelectionList( );

			const int type = mUserPropType->GetSelection( );
			const wxString propertyName = mUserPropName->GetString( mUserPropName->GetSelection( ) );
			sigassert( type >= 0 && type < array_length( cPropertyTypeChoices ) );

			wxString name;

			if( type == cPropertyTypeTag )
			{
				const tProjectFile::tGameTag* tag = tProjectFile::fGetCurrentProjectFileCached( ).fFindTagByName( std::string( propertyName ) );
				if( !tag )
				{
					wxMessageBox( "Game tag not found.", "Invalid Property Name", wxOK | wxCENTRE, this );
					return;
				}

				name = StringUtil::fToString( tag->mKey );
			}
			else if( type == cPropertyTypeEnum )
			{
				u32 key;

				if( !tProjectFile::fGetCurrentProjectFileCached( ).fFindEnumeratedTypeByName( std::string( propertyName ), &key ) )
				{
					wxMessageBox( "Enum type not found.", "Invalid Property Name", wxOK | wxCENTRE, this );
					return;
				}

				name = StringUtil::fToString( key );
			}

			if( name.length( ) == 0 )
			{
				wxMessageBox( "You can't have an empty string as a property name.", "Invalid Property Name", wxOK | wxCENTRE, this );
				return;
			}
			if( name[0] == '~' )
			{
				wxMessageBox( "Property names that begin with '~' are reserved for program use only.", "Invalid Property Name", wxOK | wxCENTRE, this );
				return;
			}

			std::string fullName;
			if( type == cPropertyTypeTag )
				fullName += Sigml::tObject::fEditablePropGameTagName( );
			else if( type == cPropertyTypeEnum )
				fullName += Sigml::tObject::fEditablePropGameEnumName( );
			fullName += name;

			if( mDoGlobalProps )
			{
				fAddProperty( mGlobalProperties, type, fullName );
			}
			else
			{
				for( u32 i = 0;  i < selected.fCount( ); ++i )
				{
					tEditableObject* eo = selected[ i ]->fDynamicCast< tEditableObject >( );
					sigassert( eo );
					fAddProperty( eo->fGetEditableProperties( ), type, fullName );
				}
			}

			fOnSelectionChanged( );

			fEditorWindow( )->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}
	}

	void tEditableObjectProperties::fNotifyGlobalPropertyChanged( tEditableProperty& property )
	{
		fEditorWindow( )->fGuiApp( ).fActionStack( ).fForceSetDirty( true );

		if( property.fGetName( ) == Sigml::tObjectProperties::fEditablePropSkeletonName( ) )
		{
			fRefreshBoneNames( property );
		}
		else if( Sigml::tFile::fIsPropertyDefaultLightRelated( property ) )
		{
			fEditorWindow( )->fGuiApp( ).fSyncDefaultLight( mGlobalProperties );
		}
		else if( property.fGetName( ) == Sigml::tObjectProperties::fEditablePropShowFogInEditor( ) )
		{
			b32 defaultData = true;
			if( property.fGetData( defaultData ) == true )
			{
				fEditorWindow( )->fGuiApp( ).fEnableFog( );
			}
			else
			{
				fEditorWindow( )->fGuiApp( ).fDisableFog( );
			}
		}
		else if( property.fGetName( ) == Sigml::tObjectProperties::fEditablePropShowGlobalLightDirection( ) )
		{
			b32 defaultData = true;
			if( property.fGetData( defaultData ) == true )
			{
				fEditorWindow( )->fGuiApp( ).fSetLightGizmoActive( true );
			}
			else
			{
				fEditorWindow( )->fGuiApp( ).fSetLightGizmoActive( false );
			}

			fEditorWindow( )->fGuiApp( ).fSyncDefaultLight( mGlobalProperties );
		}
	}
	void tEditableObjectProperties::fNotifyCommonPropertyChanged( tEditableProperty& property )
	{
		fEditorWindow( )->fRefreshLayers( );
	}
	void tEditableObjectProperties::fRefreshBoneNames( tEditableProperty& property )
	{
		// skeleton path has been updated, grab new set of bone names
		mBoneNames.fSetCount( 0 );

		std::string relPath;
		property.fGetData<std::string>( relPath );

		if( relPath.length( ) == 0 ) return;
		const tFilePathPtr absPath = ToolsPaths::fMakeResAbsolute( tFilePathPtr( relPath ) );
		if( !FileSystem::fFileExists( absPath ) ) return;

		Sklml::tFile sklml;
		if( sklml.fLoadXml( absPath ) )
			sklml.fQueryBoneNames( mBoneNames );
	}

	b32 tEditableObjectProperties::fBrowseForBoneName( tEditablePropertyCustomString& property, std::string& boneNameOut )
	{
		if( mBoneNames.fCount( ) == 0 )
		{
			wxMessageBox( "You have not associated a skeleton (.sklml) with this scene. To do so, browse for one from the Global Properties dialog.", "No Skeleton" );
			return false;
		}

		tWxSelectStringDialog dlg( this, "Select Bone Name", mBoneNames );
		if( dlg.ShowModal( ) == wxID_OK )
		{
			boneNameOut = dlg.fGetResultString( );
			return true;
		}

		return false;
	}

	b32 tEditableObjectProperties::fBrowseForSkeletonBindingName( tEditablePropertyCustomString& property, std::string& objectNameOut )
	{
		tGrowableArray<std::string> objectNames;

		const tEditableObjectContainer& eoc = fEditorWindow( )->fGuiApp( ).fEditableObjects( );
		tGrowableArray<tEditableObject*> allObjects;
		eoc.fCollectByType( allObjects );
		for( u32 i = 0; i < allObjects.fCount( ); ++i )
		{
			const std::string name = allObjects[ i ]->fGetName( );
			if( name.length( ) > 0 )
				objectNames.fPushBack( name );
		}

		if( objectNames.fCount( ) == 0 )
		{
			wxMessageBox( "There are no objects in this scene to bind the skeleton to.", "No Objects" );
			return false;
		}

		tWxSelectStringDialog dlg( this, "Select Skeleton Root", objectNames );
		if( dlg.ShowModal( ) == wxID_OK )
		{
			objectNameOut = dlg.fGetResultString( );
			return true;
		}

		return false;
	}

	void tEditableObjectProperties::fRefreshUserPropertyNames( )
	{
		mUserPropName->Clear( );

		const tProjectFile& projectFile = tProjectFile::fGetCurrentProjectFileCached( );

		const int type = mUserPropType->GetSelection( );
		if( type == cPropertyTypeTag )
		{
			for( u32 i = 0; i < projectFile.mGameTags.fCount( ); ++i )
				mUserPropName->Append( projectFile.mGameTags[ i ].mName );
		}
		else if( type == cPropertyTypeEnum )
		{
			for( u32 i = 0; i < projectFile.mGameEnumeratedTypes.fCount( ); ++i )
			{
				if( !projectFile.mGameEnumeratedTypes[i].mHide )
				{
					mUserPropName->Append( projectFile.mGameEnumeratedTypes[ i ].mName );
					for( u32 j = 0; j < projectFile.mGameEnumeratedTypes[ i ].mAliases.fCount( ); ++j )
						mUserPropName->Append( projectFile.mGameEnumeratedTypes[ i ].mAliases[ j ].mName );
				}
			}
		}
		else
			log_warning( 0, "Invalid property type." );

		if( mUserPropName->GetCount( ) == 0 )
			mUserPropName->Append( "" );
		mUserPropName->SetSelection( 0 );
	}

}

