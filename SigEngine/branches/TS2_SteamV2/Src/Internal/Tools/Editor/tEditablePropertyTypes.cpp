#include "ToolsPch.hpp"
#include "tEditablePropertyTypes.hpp"
#include "WxUtil.hpp"
#include "tEditScriptSnippetDialog.hpp"
#include "Threads/tProcess.hpp"
#include "Sigml.hpp"
#include "FileSystem.hpp"

namespace Sig
{

	///
	/// \section tEditablePropertyBool
	///

	register_rtti_factory( tEditablePropertyBool, false )

	tEditablePropertyBool::tEditablePropertyBool( const std::string& name, b32 initState )
		: tRawDataEditableProperty<b32>( name, initState )
		, mCheckBox( 0 )
	{
	}
	void tEditablePropertyBool::fCreateGui( wxWindow* parent, const std::string& label )
	{
		mCheckBox = new tCheckBox( this, parent, label.c_str( ) );
		mCheckBox->fSetToolTip( fDisplayOptions( ).mToolTip );
		fRefreshGui( );
	}
	void tEditablePropertyBool::fRefreshGui( )
	{
		if( fHasConflict( ) )
			mCheckBox->fSetValue( tWxSlapOnCheckBox::cGray );
		else
			mCheckBox->fSetValue( mRawData ? tWxSlapOnCheckBox::cTrue : tWxSlapOnCheckBox::cFalse );
	}
	void tEditablePropertyBool::fClearGui( )
	{
		mCheckBox = 0;
	}
	tEditableProperty* tEditablePropertyBool::fClone( ) const
	{
		tEditablePropertyBool* o = new tEditablePropertyBool( fGetName( ), mRawData );
		return o;
	}

	///
	/// \section tEditablePropertyMask
	///

	register_rtti_factory( tEditablePropertyMask, false )

	tEditablePropertyMask::tEditablePropertyMask( const std::string& name, u32 initState )
		: tRawDataEditableProperty<u32>( name, initState )
		, mUI( NULL )
	{
	}
	void tEditablePropertyMask::fCreateGui( wxWindow* parent, const std::string& label )
	{
		mUI = new tMask( this, parent, label.c_str( ) );
		mUI->fSetToolTip( fDisplayOptions( ).mToolTip );

		//if this were a flex sizer. but dunno how to enumerate one of those lazily like above
		//sizer->fGrid( )->SetItemSpan( mAllButton->fGetSizer( ), wxGBSpan( 1, cColumns ) );

		fRefreshGui( );
	}
	void tEditablePropertyMask::fRefreshGui( )
	{
		if( fHasConflict( ) )
		{
			mUI->fGrayOut( true );
		}
		else
		{
			mUI->fSetValue( mRawData, false );
		}
	}
	void tEditablePropertyMask::tMask::fOnControlUpdated( )
	{
		mOwnerProp->fSetData<u32>( fGetValue( ) );
	}
	void tEditablePropertyMask::fClearGui( )
	{
		mUI->fSetValue( 0, false );
	}
	tEditableProperty* tEditablePropertyMask::fClone( ) const
	{
		tEditablePropertyMask* o = new tEditablePropertyMask( fGetName( ), mRawData );
		return o;
	}

	///
	/// \section tEditablePropertyEnum
	///

	tEditablePropertyEnum::tComboBox::tComboBox( tEditablePropertyEnum* ownerProp, wxWindow* parent, const char* label, const tDynamicArray<wxString>& enumNames, u32 defChoice )
		: tWxSlapOnChoice( parent, label, enumNames.fBegin( ), enumNames.fCount( ), defChoice )
		, mOwnerProp( ownerProp )
	{
	}

	register_rtti_factory( tEditablePropertyEnum, false )

	tEditablePropertyEnum::tEditablePropertyEnum( const std::string& name, const tDynamicArray<std::string>& enumNames, u32 initState, b32 allowDelete )
		: tRawDataEditableProperty<u32>( name, initState )
		, mComboBox( 0 )
		, mAllowDelete( allowDelete )
		, mEnumNames( enumNames )
	{
	}
	void tEditablePropertyEnum::fCreateGui( wxWindow* parent, const std::string& label )
	{
		tDynamicArray<wxString> choices( mEnumNames.fCount( ) );
		for( u32 i = 0; i < choices.fCount( ); ++i )
			choices[ i ] = mEnumNames[ i ];
		mComboBox = new tComboBox( this, parent, label.c_str( ), choices, mRawData );
		mComboBox->fSetToolTip( fDisplayOptions( ).mToolTip );
		fRefreshGui( );
		if( mAllowDelete )
			fAddRemoveButton( parent, *mComboBox );
	}
	void tEditablePropertyEnum::fRefreshGui( )
	{
		if( fHasConflict( ) )
			mComboBox->fSetValue( ~0 );
		else
			mComboBox->fSetValue( mRawData );
	}
	void tEditablePropertyEnum::fClearGui( )
	{
		mComboBox = 0;
	}
	tEditableProperty* tEditablePropertyEnum::fClone( ) const
	{
		tEditablePropertyEnum* o = new tEditablePropertyEnum( fGetName( ), mEnumNames, mRawData );
		o->mAllowDelete = mAllowDelete;
		return o;
	}

	///
	/// \section tEditablePropertyProjectFileEnum
	///

	tEditablePropertyProjectFileEnum::tComboBox::tComboBox( tEditablePropertyProjectFileEnum* ownerProp, wxWindow* parent, const char* label, const tDynamicArray<wxString>& enumNames, u32 defChoice )
		: tWxSlapOnChoice( parent, label, enumNames.fBegin( ), enumNames.fCount( ), defChoice )
		, mOwnerProp( ownerProp )
	{
	}

	register_rtti_factory( tEditablePropertyProjectFileEnum, false )

		tEditablePropertyProjectFileEnum::tEditablePropertyProjectFileEnum( const std::string& name, u32 initState, b32 allowDelete )
		: tRawDataEditableProperty<u32>( name, initState )
		, mComboBox( 0 )
		, mAllowDelete( allowDelete )
	{
	}
	u32 tEditablePropertyProjectFileEnum::fEnumTypeKey( ) const
	{
		std::stringstream ss;
		ss <<  mName.substr( mName.find_last_of( '.' ) + 1 );
		u32 key = 0;
		ss >> key;
		return key;
	}
	const tProjectFile::tGameEnumeratedType* tEditablePropertyProjectFileEnum::fEnumType( ) const
	{
		return tProjectFile::fGetCurrentProjectFileCached( ).fFindEnumeratedTypeByKey( fEnumTypeKey( ) );
	}
	void tEditablePropertyProjectFileEnum::fCreateGui( wxWindow* parent, const std::string& label )
	{
		const tProjectFile& projectFile = tProjectFile::fGetCurrentProjectFileCached( );
		const tProjectFile::tGameEnumeratedType* enumType = fEnumType( );

		std::string name;
		tDynamicArray<wxString> choices;

		if( enumType )
		{
			choices.fNewArray( enumType->mValues.fCount( ) );
			for( u32 i = 0; i < choices.fCount( ); ++i )
				choices[ i ] = enumType->mValues[ i ].mName;

			name = enumType->mName;
		}
		else
			name = "Error";

		mComboBox = new tComboBox( this, parent, name.c_str( ), choices, mRawData );
		mComboBox->fSetToolTip( fDisplayOptions( ).mToolTip );
		fRefreshGui( );
		if( mAllowDelete )
			fAddRemoveButton( parent, *mComboBox );
	}
	void tEditablePropertyProjectFileEnum::fRefreshGui( )
	{
		if( fHasConflict( ) )
			mComboBox->fSetValue( ~0 );
		else
		{
			const tProjectFile::tGameEnumeratedType* enumType = fEnumType( );
			u32 index = ~0;
			if( enumType ) index = enumType->fFindValueIndexByKey( mRawData );

			mComboBox->fSetValue( index );
		}
	}
	void tEditablePropertyProjectFileEnum::fClearGui( )
	{
		mComboBox = 0;
	}
	tEditableProperty* tEditablePropertyProjectFileEnum::fClone( ) const
	{
		tEditablePropertyProjectFileEnum* o = new tEditablePropertyProjectFileEnum( fGetName( ), mRawData );
		o->mAllowDelete = mAllowDelete;
		return o;
	}

	///
	/// \section tEditablePropertyProjectFileTag
	///

	tEditablePropertyProjectFileTag::tTextBox::tTextBox( tEditablePropertyProjectFileTag* ownerProp, wxWindow* parent, const char* label )
		: tWxSlapOnTextBox( parent, label )
		, mOwnerProp( ownerProp )
	{
	}

	register_rtti_factory( tEditablePropertyProjectFileTag, false )

	tEditablePropertyProjectFileTag::tEditablePropertyProjectFileTag( const std::string& name, b32 allowDelete )
		: tRawDataEditableProperty<u32>( name, atoi( name.substr( name.find( '.' ) + 1 ).c_str( ) ) )
		, mTextBox( 0 )
		, mAllowDelete( allowDelete )
	{
	}
	void tEditablePropertyProjectFileTag::fCreateGui( wxWindow* parent, const std::string& label )
	{
		const tProjectFile::tGameTag* tag = tProjectFile::fGetCurrentProjectFileCached( ).fFindTagByKey( mRawData );

		std::string name;
		if( !tag ) name == "Error!";
		else name = tag->mName;

		mTextBox = new tTextBox( this, parent, name.c_str( ) );
		mTextBox->fSetToolTip( fDisplayOptions( ).mToolTip );
		fRefreshGui( );
		if( mAllowDelete )
			fAddRemoveButton( parent, *mTextBox );
	}
	void tEditablePropertyProjectFileTag::fRefreshGui( )
	{
		if( fHasConflict( ) )
			mTextBox->fSetValue( "" );
		else
		{
			const tProjectFile::tGameTag* tag = tProjectFile::fGetCurrentProjectFileCached( ).fFindTagByKey( mRawData );
			std::string value;
			if( !tag ) value == "Error!";
			else value = tag->mName;

			mTextBox->fSetValue( value );
		}

		mTextBox->fDisableControl( );
	}
	void tEditablePropertyProjectFileTag::fClearGui( )
	{
		mTextBox = 0;
	}
	tEditableProperty* tEditablePropertyProjectFileTag::fClone( ) const
	{
		tEditablePropertyProjectFileTag* o = new tEditablePropertyProjectFileTag( fGetName( ), mRawData );
		o->mAllowDelete = mAllowDelete;
		return o;
	}

	///
	/// \section tEditablePropertyString
	///

	register_rtti_factory( tEditablePropertyString, false )

		tEditablePropertyString::tEditablePropertyString( const std::string& name, const std::string& initState )
		: tRawDataEditableProperty<std::string>( name, initState )
		, mAllowDelete( false )
		, mLockFieldToName( false )
		, mTextBox( 0 )
	{
	}

	tEditablePropertyString::tEditablePropertyString( const std::string& name, b32 allowDelete, b32 lockFieldtoName )
		: tRawDataEditableProperty<std::string>( name, lockFieldtoName ? name.substr( name.find_last_of( "." ) + 1 ) : "" )
		, mAllowDelete( allowDelete )
		, mLockFieldToName( lockFieldtoName )
		, mTextBox( 0 )
	{
	}

	void tEditablePropertyString::fCreateGui( wxWindow* parent, const std::string& label )
	{
		mTextBox = new tTextBox( this, parent, label.c_str( ) );
		mTextBox->fSetToolTip( fDisplayOptions( ).mToolTip );

		fRefreshGui( );

		if( mAllowDelete )
			fAddRemoveButton( parent, *mTextBox );
	}
	void tEditablePropertyString::fRefreshGui( )
	{
		if( fHasConflict( ) )
			mTextBox->fSetValue( "" );
		else
			mTextBox->fSetValue( mRawData );

		if( mLockFieldToName )
			mTextBox->fDisableControl( );
	}
	void tEditablePropertyString::fClearGui( )
	{
		mTextBox = 0;
	}
	tEditableProperty* tEditablePropertyString::fClone( ) const
	{
		tEditablePropertyString* o = new tEditablePropertyString( fGetName( ), mRawData );
		o->mAllowDelete = mAllowDelete;
		o->mLockFieldToName = mLockFieldToName;
		return o;
	}
	void tEditablePropertyString::fGetRawData( Rtti::tClassId cid, void* dst, u32 size ) const
	{
		sigassert( cid == Rtti::fGetClassId<std::string>( ) );
		sigassert( size == sizeof( mRawData ) );
		*( std::string* )dst = mRawData;
	}
	void tEditablePropertyString::fSetRawData( Rtti::tClassId cid, const void* src, u32 size )
	{
		if( !mLockFieldToName )
		{
			sigassert( cid == Rtti::fGetClassId<std::string>( ) );
			sigassert( size == sizeof( mRawData ) );
			mRawData = *( std::string* )src;
		}
	}

	///
	/// \section tEditablePropertyScriptString
	///

	register_rtti_factory( tEditablePropertyScriptString, false )

		tEditablePropertyScriptString::tEditablePropertyScriptString( const std::string& name, const std::string& initState )
		: tRawDataEditableProperty<std::string>( name, initState )
		, mAllowDelete( false )
		, mLockFieldToName( false )
		, mTextBox( 0 )
		, mScriptCursorPos( 0 )
	{
	}

	tEditablePropertyScriptString::tEditablePropertyScriptString( const std::string& name, b32 allowDelete, b32 lockFieldtoName )
		: tRawDataEditableProperty<std::string>( name, lockFieldtoName ? name.substr( name.find_last_of( "." ) + 1 ) : "" )
		, mAllowDelete( allowDelete )
		, mLockFieldToName( lockFieldtoName )
		, mTextBox( 0 )
		, mScriptCursorPos( 0 )
	{
	}

	void tEditablePropertyScriptString::fCreateGui( wxWindow* parent, const std::string& label )
	{
		mTextBox = new tTextBox( this, parent, label.c_str( ) );
		mTextBox->fSetToolTip( fDisplayOptions( ).mToolTip );

		fRefreshGui( );

		wxButton* button = new wxButton( parent, wxID_ANY, "Edit", wxDefaultPosition, wxSize( 20, 20 ) );
		button->SetForegroundColour( wxColour( 0x99, 0x00, 0x00 ) );
		button->SetToolTip( "Edit script snippet" );
		mTextBox->fAddWindowToSizer( button, true );
		button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEditablePropertyScriptString::fOnEditPressed ), NULL, this );

		if( mAllowDelete )
			fAddRemoveButton( parent, *mTextBox );
	}

	void tEditablePropertyScriptString::fOnEditPressed( wxCommandEvent& )
	{
		tEditScriptSnippetDialog *edit = new tEditScriptSnippetDialog( mTextBox->fParent( ), "Edit Snippet", mStaticText, mTextBox->fGetValue( ), mScriptCursorPos );

		if( edit->fShowDialog( ) == tEditScriptSnippetDialog::cResultChanged )
		{
			fSetData<std::string>( std::string( edit->fGetScript( ) ) );
			fRefreshGui( );

			mScriptCursorPos = edit->fTextEditor( )->GetCurrentPos( );
		}
	}

	void tEditablePropertyScriptString::fRefreshGui( )
	{
		if( fHasConflict( ) )
			mTextBox->fSetValue( "" );
		else
			mTextBox->fSetValue( mRawData );

		if( mLockFieldToName )
			mTextBox->fDisableControl( );
	}
	void tEditablePropertyScriptString::fClearGui( )
	{
		mTextBox = 0;
	}
	tEditableProperty* tEditablePropertyScriptString::fClone( ) const
	{
		tEditablePropertyScriptString* o = new tEditablePropertyScriptString( fGetName( ), mRawData );
		o->mAllowDelete = mAllowDelete;
		o->mLockFieldToName = mLockFieldToName;
		o->mScriptCursorPos = mScriptCursorPos;
		o->mStaticText = mStaticText;
		return o;
	}
	void tEditablePropertyScriptString::fGetRawData( Rtti::tClassId cid, void* dst, u32 size ) const
	{
		sigassert( cid == Rtti::fGetClassId<std::string>( ) );
		sigassert( size == sizeof( mRawData ) );
		*( std::string* )dst = mRawData;
	}
	void tEditablePropertyScriptString::fSetRawData( Rtti::tClassId cid, const void* src, u32 size )
	{
		if( !mLockFieldToName )
		{
			sigassert( cid == Rtti::fGetClassId<std::string>( ) );
			sigassert( size == sizeof( mRawData ) );
			mRawData = *( std::string* )src;
		}
	}

	///
	/// \section tEditablePropertyFileNameString
	///

	struct tFileBrowseInfo
	{
		tGrowableArray< std::string > mFilters;
		std::string mDefaultBrowseDirectory;
	};

	typedef tHashTable< std::string, tFileBrowseInfo > tFileBrowserFilterMapBase;
	class tFileBrowserFilterMap : public tFileBrowserFilterMapBase
	{
		declare_singleton_define_own_ctor_dtor( tFileBrowserFilterMap );
		tFileBrowserFilterMap( ) : tFileBrowserFilterMapBase( 32 ) { }
		~tFileBrowserFilterMap( ) { }
	};

	void tEditablePropertyFileNameString::tTextBox::fOnLabelButtonClicked( wxCommandEvent& event )
	{
		std::string text = "";
		mOwnerProp->fGetData( text );

		if( text.empty( ) )
		{
			std::string name = mOwnerProp->fGetName( );
			if( name == Sigml::tObjectProperties::fEditablePropScriptName( ) )
			{
				tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
					mParent,
					"Create New Script",
					"",
					wxString( "untitled.nut" ),
					wxString( "*.nut" ),
					wxFD_SAVE | wxFD_OVERWRITE_PROMPT ) );

				if( openFileDialog->ShowModal( ) != wxID_OK )
					return;

				text = openFileDialog->GetPath( );
				FileSystem::fWriteBufferToFile( tDynamicBuffer( ), tFilePathPtr( text.c_str( ) ) );
				text = ToolsPaths::fMakeResRelative( tFilePathPtr( text.c_str( ) ) ).fCStr( );
				mOwnerProp->fSetData( text );
				fSetValue( text );
			}
		}

		// TODO: support other file types by switching on different extensions.
		// ULTRA TODO: find a less hacky way to accomplish this
		if( StringUtil::fCheckExtension( text.c_str( ), ".nut" ) )
		{
			ToolsPaths::fLaunchSigScript( text.c_str( ) );
		}
	}

	void tEditablePropertyFileNameString::fAddFilter( const std::string& propName, const std::string& filter )
	{
		tFileBrowserFilterMap::fInstance( )[ propName ].mFilters.fFindOrAdd( filter );
	}
	void tEditablePropertyFileNameString::fSetDefaultBrowseDirectory( const std::string& propName, const std::string& defaultBrowseDirectory )
	{
		tFileBrowserFilterMap::fInstance( )[ propName ].mDefaultBrowseDirectory = defaultBrowseDirectory;
	}	

	register_rtti_factory( tEditablePropertyFileNameString, false )

	tEditablePropertyFileNameString::tEditablePropertyFileNameString( const std::string& name, const std::string& initState )
		: tRawDataEditableProperty<std::string>( name, initState )
		, mTextBox( 0 )
		, mBrowseButton( 0 )
	{
		
	}

	void tEditablePropertyFileNameString::fCreateGui( wxWindow* parent, const std::string& label )
	{
		mTextBox = new tTextBox( this, parent, label.c_str( ), fButtonSupportedFile(label) );
		mTextBox->fSetToolTip( fDisplayOptions( ).mToolTip );
		mBrowseButton = new wxButton( parent, wxID_ANY, "...", wxDefaultPosition, wxSize( 22, 20 ) );
		mBrowseButton->SetForegroundColour( wxColour( 0x22, 0x22, 0xff ) );
		mTextBox->fAddWindowToSizer( mBrowseButton, true );
		mBrowseButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEditablePropertyFileNameString::fOnBrowse ), NULL, this );
		fRefreshGui( );
	}
	void tEditablePropertyFileNameString::fRefreshGui( )
	{
		if( fHasConflict( ) )
			mTextBox->fSetValue( "" );
		else
			mTextBox->fSetValue( mRawData );
	}
	void tEditablePropertyFileNameString::fClearGui( )
	{
		mTextBox = 0;
	}
	tEditableProperty* tEditablePropertyFileNameString::fClone( ) const
	{
		return new tEditablePropertyFileNameString( fGetName( ), mRawData );
	}
	void tEditablePropertyFileNameString::fOnBrowse( wxCommandEvent& )
	{
		const tGrowableArray<std::string>& filters = tFileBrowserFilterMap::fInstance( )[ fGetName( ) ].mFilters;
		const std::string defaultBrowseDirectory = tFileBrowserFilterMap::fInstance( )[ fGetName( ) ].mDefaultBrowseDirectory;

		std::string filter;
		for( u32 i = 0; i < filters.fCount( ); ++i )
		{
			filter += filters[ i ];
			if( i < filters.fCount( ) - 1 )
				filter += ";";
		}

		std::string path;
		if( WxUtil::fBrowseForFile( path, mBrowseButton->GetParent( ), "Select File", ToolsPaths::fMakeResAbsolute( tFilePathPtr( defaultBrowseDirectory ) ).fCStr( ), 0, filter.length( ) > 0 ? filter.c_str( ) : 0, wxFD_OPEN ) )
		{
			path = ToolsPaths::fMakeResRelative( tFilePathPtr( path ) ).fCStr( );

			fSetData<std::string>( path );
			if( fHasConflict( ) )
				mTextBox->fSetValue( "" );
			else
				mTextBox->fSetValue( mRawData );
		}
	}
	b32 tEditablePropertyFileNameString::fButtonSupportedFile( const std::string& label )
	{
		// TODO: add other editable properties here to support them and then do the right spawning in
		// the OnButtonClicked above.
		return label == "ScriptPath";
	}
	void tEditablePropertyFileNameString::fGetRawData( Rtti::tClassId cid, void* dst, u32 size ) const
	{
		sigassert( cid == Rtti::fGetClassId<std::string>( ) );
		sigassert( size == sizeof( mRawData ) );
		*( std::string* )dst = mRawData;
	}
	void tEditablePropertyFileNameString::fSetRawData( Rtti::tClassId cid, const void* src, u32 size )
	{
		sigassert( cid == Rtti::fGetClassId<std::string>( ) );
		sigassert( size == sizeof( mRawData ) );
		mRawData = *( std::string* )src;
	}


	///
	/// \section tEditablePropertyCustomString
	///

	typedef tHashTable< std::string, tEditablePropertyCustomString::tCallbackData > tCustomStringCallbackMapBase;
	class tCustomStringCallbackMap : public tCustomStringCallbackMapBase
	{
		declare_singleton_define_own_ctor_dtor( tCustomStringCallbackMap );
		tCustomStringCallbackMap( ) : tCustomStringCallbackMapBase( 32 ) { }
		~tCustomStringCallbackMap( ) { }
	};

	void tEditablePropertyCustomString::fAddCallback( const std::string& propName, const tCallbackData& callback )
	{
		tCustomStringCallbackMap::fInstance( )[ propName ] = callback;
	}

	register_rtti_factory( tEditablePropertyCustomString, false )

	tEditablePropertyCustomString::tEditablePropertyCustomString( const std::string& name, const std::string& initState )
		: tRawDataEditableProperty<std::string>( name, initState )
		, mTextBox( 0 )
		, mBrowseButton( 0 )
	{
	}

	void tEditablePropertyCustomString::fCreateGui( wxWindow* parent, const std::string& label )
	{
		mTextBox = new tTextBox( this, parent, label.c_str( ) );
		mTextBox->fSetToolTip( fDisplayOptions( ).mToolTip );
		mBrowseButton = new wxButton( parent, wxID_ANY, "...", wxDefaultPosition, wxSize( 22, 20 ) );
		mBrowseButton->SetForegroundColour( wxColour( 0x22, 0x22, 0xff ) );
		mTextBox->fAddWindowToSizer( mBrowseButton, true );
		mBrowseButton->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEditablePropertyCustomString::fOnBrowse ), NULL, this );

		fRefreshGui( );
	}
	void tEditablePropertyCustomString::fRefreshGui( )
	{
		if( fHasConflict( ) )
			mTextBox->fSetValue( "" );
		else
			mTextBox->fSetValue( mRawData );
	}
	void tEditablePropertyCustomString::fClearGui( )
	{
		mTextBox = 0;
	}
	tEditableProperty* tEditablePropertyCustomString::fClone( ) const
	{
		return new tEditablePropertyCustomString( fGetName( ), mRawData );
	}
	void tEditablePropertyCustomString::fOnBrowse( wxCommandEvent& )
	{
		tCallbackData cb = tCustomStringCallbackMap::fInstance( )[ fGetName( ) ];

		if( !cb.mFunc.fNull( ) )
		{
			std::string newValue;
			if( cb.mFunc( *this, newValue ) )
			{
				fSetData<std::string>( newValue );
				if( fHasConflict( ) )
					mTextBox->fSetValue( "" );
				else
					mTextBox->fSetValue( mRawData );
			}
		}
	}
	void tEditablePropertyCustomString::fGetRawData( Rtti::tClassId cid, void* dst, u32 size ) const
	{
		sigassert( cid == Rtti::fGetClassId<std::string>( ) );
		sigassert( size == sizeof( mRawData ) );
		*( std::string* )dst = mRawData;
	}
	void tEditablePropertyCustomString::fSetRawData( Rtti::tClassId cid, const void* src, u32 size )
	{
		sigassert( cid == Rtti::fGetClassId<std::string>( ) );
		sigassert( size == sizeof( mRawData ) );
		mRawData = *( std::string* )src;
	}


	///
	/// \section tEditablePropertyFloat
	///

	register_rtti_factory( tEditablePropertyFloat, false )

	tEditablePropertyFloat::tEditablePropertyFloat( 
		const std::string& name, 
		f32 initState,
		f32 min, f32 max, f32 increment, u32 precision, b32 allowDelete )
		: tEditablePropertyVectorBase<Math::tVec1f>( name, initState, min, max, increment, precision, allowDelete )
	{
	}

	tEditableProperty* tEditablePropertyFloat::fClone( ) const
	{
		return new tEditablePropertyFloat( mName, mRawData.x, mMin, mMax, mIncrement, mPrecision, mAllowDelete );
	}


	///
	/// \section tEditablePropertyVec2f
	///

	register_rtti_factory( tEditablePropertyVec2f, false )

	tEditablePropertyVec2f::tEditablePropertyVec2f( 
		const std::string& name, 
		const Math::tVec2f& initState,
		f32 min, f32 max, f32 increment, u32 precision, b32 allowDelete )
		: tEditablePropertyVectorBase<Math::tVec2f>( name, initState, min, max, increment, precision, allowDelete )
	{
	}

	tEditableProperty* tEditablePropertyVec2f::fClone( ) const
	{
		return new tEditablePropertyVec2f( mName, mRawData, mMin, mMax, mIncrement, mPrecision, mAllowDelete );
	}

	///
	/// \section tEditablePropertyVec3f
	///

	register_rtti_factory( tEditablePropertyVec3f, false )

	tEditablePropertyVec3f::tEditablePropertyVec3f( 
		const std::string& name, 
		const Math::tVec3f& initState,
		f32 min, f32 max, f32 increment, u32 precision, b32 allowDelete )
		: tEditablePropertyVectorBase<Math::tVec3f>( name, initState, min, max, increment, precision, allowDelete )
	{
	}

	tEditableProperty* tEditablePropertyVec3f::fClone( ) const
	{
		return new tEditablePropertyVec3f( mName, mRawData, mMin, mMax, mIncrement, mPrecision, mAllowDelete );
	}

	///
	/// \section tEditablePropertyVec4f
	///

	register_rtti_factory( tEditablePropertyVec4f, false )

	tEditablePropertyVec4f::tEditablePropertyVec4f( 
		const std::string& name, 
		const Math::tVec4f& initState,
		f32 min, f32 max, f32 increment, u32 precision, b32 allowDelete )
		: tEditablePropertyVectorBase<Math::tVec4f>( name, initState, min, max, increment, precision, allowDelete )
	{
	}

	tEditableProperty* tEditablePropertyVec4f::fClone( ) const
	{
		return new tEditablePropertyVec4f( mName, mRawData, mMin, mMax, mIncrement, mPrecision, mAllowDelete );
	}

	///
	/// \section tEditablePropertyVec2i
	///

	register_rtti_factory( tEditablePropertyVec2i, false )

	tEditablePropertyVec2i::tEditablePropertyVec2i( 
		const std::string& name, 
		const Math::tVec2i& initState,
		s32 min, s32 max, s32 increment, b32 allowDelete )
		: tEditablePropertyVectorBase<Math::tVec2i>( name, initState, ( f32 )min, ( f32 )max, ( f32 )increment, 0, allowDelete )
	{
	}

	tEditableProperty* tEditablePropertyVec2i::fClone( ) const
	{
		return new tEditablePropertyVec2i( mName, mRawData, mMin, mMax, mIncrement, mAllowDelete );
	}

	///
	/// \section tEditablePropertyVec3i
	///

	register_rtti_factory( tEditablePropertyVec3i, false )

	tEditablePropertyVec3i::tEditablePropertyVec3i( 
		const std::string& name, 
		const Math::tVec3i& initState,
		s32 min, s32 max, s32 increment, b32 allowDelete )
		: tEditablePropertyVectorBase<Math::tVec3i>( name, initState, ( f32 )min, ( f32 )max, ( f32 )increment, 0, allowDelete )
	{
	}

	tEditableProperty* tEditablePropertyVec3i::fClone( ) const
	{
		return new tEditablePropertyVec3i( mName, mRawData, mMin, mMax, mIncrement, mAllowDelete );
	}
}
