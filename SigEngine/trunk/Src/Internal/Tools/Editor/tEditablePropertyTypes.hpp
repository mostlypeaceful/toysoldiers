#ifndef __tEditablePropertyTypes__
#define __tEditablePropertyTypes__
#include "tEditableProperty.hpp"
#include "tWxSlapOnCheckBox.hpp"
#include "tWxSlapOnTextBox.hpp"
#include "tWxSlapOnSpinner.hpp"
#include "tWxSlapOnChoice.hpp"
#include "tWxSlapOnMask.hpp"
#include "tProjectFile.hpp"
#include "wx/spinctrl.h"

namespace Sig
{

	///
	/// \brief Templatized type providing implementations for the majority of the pure virtual methods
	/// required for deriving from tEditableProperty. This type should provide sufficient base functionality
	/// for all common properties, like floats, ints, vectors, matrices, strings, etc. However, you will still
	/// need to derive your own type to provide a unique serializable class id, as well as gui methods.
	template<class tRawData>
	class tRawDataEditableProperty : public tEditableProperty
	{
		//implement_rtti_serializable_base_class( YourDerivedClassName, YourUniqueClassId );
	protected:
		tRawData mRawData;
	public:

		tRawDataEditableProperty( ) { }

		explicit tRawDataEditableProperty( const std::string& name, const tRawData& initState = tRawData( ) )
			: tEditableProperty( name )
			, mRawData( initState )
		{
		}

		const tRawData& fRawData( ) const { return mRawData; }

	protected:
		virtual void fSerializeXml( tXmlSerializer& s )
		{
			s( "Name", mName );
			s( "Data", mRawData );
		}
		virtual void fSerializeXml( tXmlDeserializer& s )
		{
			s( "Name", mName );
			s( "Data", mRawData );
		}
		virtual void fGetRawData( Rtti::tClassId cid, void* dst, u32 size ) const
		{
			sigassert( cid == Rtti::fGetClassId<tRawData>( ) );
			sigassert( size == sizeof( mRawData ) );
			fMemCpy( dst, &mRawData, size );
		}
		virtual void fSetRawData( Rtti::tClassId cid, const void* src, u32 size )
		{
			sigassert( cid == Rtti::fGetClassId<tRawData>( ) );
			sigassert( size == sizeof( mRawData ) );
			fMemCpy( &mRawData, src, size );
		}
		virtual b32 fEqualByType( const tEditableProperty& other ) const
		{
			return Sig::fEqual( mRawData, static_cast< const tRawDataEditableProperty<tRawData>& >( other ).mRawData );
		}
	};

	class tools_export tEditablePropertyBool : public tRawDataEditableProperty<b32>
	{
		implement_rtti_serializable_base_class( tEditablePropertyBool, 0xC379FC2B );
	private:
		class tools_export tCheckBox : public tWxSlapOnCheckBox
		{
			tEditablePropertyBool* mOwnerProp;
		public:
			tCheckBox( tEditablePropertyBool* ownerProp, wxWindow* parent, const char* label ) 
				: tWxSlapOnCheckBox( parent, label ), mOwnerProp( ownerProp ) { }
			virtual void fOnControlUpdated( ) { mOwnerProp->fSetData<b32>( fGetValue( ) == cTrue ); }
		};
		tCheckBox* mCheckBox;
	public:
		tEditablePropertyBool( ) { mRawData = false; }
		explicit tEditablePropertyBool( const std::string& name, b32 initState = false );
		virtual void fCreateGui( tCreateGuiData& data );
		virtual void fRefreshGui( );
		virtual void fClearGui( );
		virtual tEditableProperty* fClone( ) const;
	};

	class tools_export tEditablePropertyMask : public tRawDataEditableProperty<u32>
	{
		implement_rtti_serializable_base_class( tEditablePropertyMask, 0xE6E69568 );
	private:
		// this has to be inherent to the type since there's no easy way for editable properties
		//  to pass along configuration data. such as the number of bits
		static const u32 cNumBits = 16;

		class tools_export tMask : public tWxSlapOnMask
		{
			tEditablePropertyMask* mOwnerProp;
			u32	mIndex;
		public:
			tMask( tEditablePropertyMask* ownerProp, wxWindow* parent, const char* label )
				: tWxSlapOnMask( parent, label, cNumBits )
				, mOwnerProp( ownerProp )
			{ }
			virtual void fOnControlUpdated( );
		};

		tMask* mUI;

	public:
		explicit tEditablePropertyMask( ) : mUI( NULL ) { mRawData = 0; }
		explicit tEditablePropertyMask( u32 intialValue ) : mUI( NULL ) { mRawData = intialValue; }
		explicit tEditablePropertyMask( const std::string& name, u32 initState );
		virtual void fCreateGui( tCreateGuiData& data );
		virtual void fRefreshGui( );
		virtual void fClearGui( );
		virtual tEditableProperty* fClone( ) const;
	};

	class tools_export tEditablePropertyEnum : public tRawDataEditableProperty<u32>
	{
		implement_rtti_serializable_base_class( tEditablePropertyEnum, 0x2C1AC345 );
	private:
		class tools_export tComboBox : public tWxSlapOnChoice
		{
			tEditablePropertyEnum* mOwnerProp;
		public:
			tComboBox( tEditablePropertyEnum* ownerProp, wxWindow* parent, const char* label, const tDynamicArray<wxString>& enumNames, u32 defChoice );
			virtual void fOnControlUpdated( ) { mOwnerProp->fSetData<u32>( fGetValue( ) ); }
		};
		tComboBox* mComboBox;
		b32 mAllowDelete;
		tDynamicArray<std::string> mEnumNames;
	public:
		tEditablePropertyEnum( ) : mComboBox( 0 ), mAllowDelete( false ) { mRawData = 0; }
		explicit tEditablePropertyEnum( const std::string& name, const tDynamicArray<std::string>& enumNames, u32 initState = 0, b32 allowDelete = false );
		virtual void fCreateGui( tCreateGuiData& data );
		virtual void fRefreshGui( );
		virtual void fClearGui( );
		virtual tEditableProperty* fClone( ) const;
		tComboBox* fComboBox( ) { return mComboBox; }

		template<class tSerializer>
		void fSerialize( tSerializer& s )
		{
			tRawDataEditableProperty<u32>::fSerializeXml( s );
			s( "AllowDelete", mAllowDelete );
			s( "EnumNames", mEnumNames );
		}

		virtual void fSerializeXml( tXmlSerializer& s ) { fSerialize( s ); }
		virtual void fSerializeXml( tXmlDeserializer& s ) { fSerialize( s ); }
		virtual void fUpdate( tEditableProperty& other );
	};

	class tools_export tEditablePropertyLenseFlare : public tRawDataEditableProperty<u32>
	{
		implement_rtti_serializable_base_class( tEditablePropertyLenseFlare, 0x73C0734C );
	private:
		class tools_export tComboBox : public tWxSlapOnChoice
		{
			tEditablePropertyLenseFlare* mOwnerProp;
		public:
			tComboBox( tEditablePropertyLenseFlare* ownerProp, wxWindow* parent, const char* label, const tDynamicArray<wxString>& enumNames, u32 defChoice );
			virtual void fOnControlUpdated( ) 
			{ 
				mOwnerProp->fSetByIndex( fGetValue( ) );
			}
		};
		tComboBox* mComboBox;
		wxButton* mEditButton;
		b32 mAllowDelete;
	public:
		tEditablePropertyLenseFlare( ) : mComboBox( NULL ), mEditButton( NULL ), mAllowDelete( false ) { mRawData = 0; }
		explicit tEditablePropertyLenseFlare( const std::string& name, u32 initState = ~0, b32 allowDelete = false );

		void fSetByIndex( u32 index );

		virtual void fCreateGui( tCreateGuiData& data );
		virtual void fRefreshGui( );
		virtual void fClearGui( );
		virtual tEditableProperty* fClone( ) const;

		template<class tSerializer>
		void fSerialize( tSerializer& s )
		{
			tRawDataEditableProperty<u32>::fSerializeXml( s );
			s( "AllowDelete", mAllowDelete );
		}

		virtual void fSerializeXml( tXmlSerializer& s ) { fSerialize( s ); }
		virtual void fSerializeXml( tXmlDeserializer& s ) { fSerialize( s ); }

		void fOnEdit( wxCommandEvent& );
	};

	// The name is stored like so: [group].[enum key]
	//  The value key is stored in the raw data.
	class tools_export tEditablePropertyProjectFileEnum : public tRawDataEditableProperty<u32>
	{
		implement_rtti_serializable_base_class( tEditablePropertyProjectFileEnum, 0x7AEC289A );
	private:
		class tools_export tComboBox : public tWxSlapOnChoice
		{
			tEditablePropertyProjectFileEnum* mOwnerProp;
		public:
			tComboBox( tEditablePropertyProjectFileEnum* ownerProp, wxWindow* parent, const char* label, const tDynamicArray<wxString>& enumNames, u32 defChoice );
			virtual void fOnControlUpdated( ) 
			{ 
				u32 index = fGetValue( );
				const tProjectFile::tGameEnumeratedType* enumType = mOwnerProp->fEnumType( );
				sigassert( enumType );
				mOwnerProp->fSetData<u32>( enumType->mValues[ index ].mKey ); 
			}
		};
		tComboBox* mComboBox;
		b32 mAllowDelete;
	public:
		tEditablePropertyProjectFileEnum( ) : mComboBox( 0 ), mAllowDelete( false ) { mRawData = 0; }
		explicit tEditablePropertyProjectFileEnum( const std::string& name, u32 initState = 0, b32 allowDelete = false );

		u32 fEnumTypeKey( ) const;
		const tProjectFile::tGameEnumeratedType* fEnumType( ) const;
		virtual void fCreateGui( tCreateGuiData& data );
		virtual void fRefreshGui( );
		virtual void fClearGui( );
		virtual tEditableProperty* fClone( ) const;

		template<class tSerializer>
		void fSerialize( tSerializer& s )
		{
			tRawDataEditableProperty<u32>::fSerializeXml( s );
			s( "AllowDelete", mAllowDelete );
		}

		virtual void fSerializeXml( tXmlSerializer& s ) { fSerialize( s ); }
		virtual void fSerializeXml( tXmlDeserializer& s ) { fSerialize( s ); }
	};

	// The name is stored like so: [group].[tag key]
	//  The key is also stored in the raw data.
	class tools_export tEditablePropertyProjectFileTag : public tRawDataEditableProperty<u32>
	{
		implement_rtti_serializable_base_class( tEditablePropertyProjectFileTag, 0x322446CF );
	private:
		class tools_export tTextBox : public tWxSlapOnTextBox
		{
			tEditablePropertyProjectFileTag* mOwnerProp;
		public:
			tTextBox( tEditablePropertyProjectFileTag* ownerProp, wxWindow* parent, const char* label );
			virtual void fOnControlUpdated( ) 
			{ 
				log_assert( false, "This should not be getting called. data is locked!" );
			}
		};
		tTextBox* mTextBox;
		b32 mAllowDelete;
	public:
		tEditablePropertyProjectFileTag( ) : mTextBox( 0 ), mAllowDelete( false ) { mRawData = 0; }
		explicit tEditablePropertyProjectFileTag( const std::string& name, b32 allowDelete = false );
		virtual void fCreateGui( tCreateGuiData& data );
		virtual void fRefreshGui( );
		virtual void fClearGui( );
		virtual tEditableProperty* fClone( ) const;

		template<class tSerializer>
		void fSerialize( tSerializer& s )
		{
			tRawDataEditableProperty<u32>::fSerializeXml( s );
			s( "AllowDelete", mAllowDelete );
		}

		virtual void fSerializeXml( tXmlSerializer& s ) { fSerialize( s ); }
		virtual void fSerializeXml( tXmlDeserializer& s ) { fSerialize( s ); }
	};

	class tools_export tEditablePropertyString : public tRawDataEditableProperty<std::string>
	{
		implement_rtti_serializable_base_class( tEditablePropertyString, 0x18C579EE );
	private:
		class tools_export tTextBox : public tWxSlapOnTextBox
		{
			tEditablePropertyString* mOwnerProp;
		public:
			tTextBox( tEditablePropertyString* ownerProp, wxWindow* parent, const char* label ) 
				: tWxSlapOnTextBox( parent, label ), mOwnerProp( ownerProp ) { }
			virtual void fOnControlUpdated( ) { mOwnerProp->fSetData<std::string>( fGetValue( ) ); }
		};
		tTextBox* mTextBox;
		b32 mAllowDelete;
		b32 mLockFieldToName;
	public:
		tEditablePropertyString( ) { }
		explicit tEditablePropertyString( const std::string& name, const std::string& initState = "" );
		explicit tEditablePropertyString( const std::string& name, b32 allowDelete, b32 lockFieldtoName );
		virtual void fCreateGui( tCreateGuiData& data );
		virtual void fRefreshGui( );
		virtual void fClearGui( );
		virtual tEditableProperty* fClone( ) const;
	protected:
		virtual void fGetRawData( Rtti::tClassId cid, void* dst, u32 size ) const;
		virtual void fSetRawData( Rtti::tClassId cid, const void* src, u32 size );

		template<class tSerializer>
		void fSerialize( tSerializer& s )
		{
			tRawDataEditableProperty<std::string>::fSerializeXml( s );
			s( "AllowDelete", mAllowDelete );
			s( "LockFieldToName", mLockFieldToName );
		}

		virtual void fSerializeXml( tXmlSerializer& s ) { fSerialize( s ); }
		virtual void fSerializeXml( tXmlDeserializer& s ) { fSerialize( s ); }
	};

	class tools_export tEditablePropertyScriptString : public tRawDataEditableProperty<std::string>
	{
		implement_rtti_serializable_base_class( tEditablePropertyScriptString, 0x9755C72 );
	private:
		class tools_export tTextBox : public tWxSlapOnTextBox
		{
			tEditablePropertyScriptString* mOwnerProp;
		public:
			tTextBox( tEditablePropertyScriptString* ownerProp, wxWindow* parent, const char* label ) 
				: tWxSlapOnTextBox( parent, label ), mOwnerProp( ownerProp ) { }
			virtual void fOnControlUpdated( ) { mOwnerProp->fSetData<std::string>( fGetValue( ) ); }
		};
		tTextBox* mTextBox;
		b32 mAllowDelete;
		b32 mLockFieldToName;
		u32 mScriptCursorPos;
		std::string mStaticText;

		void fOnEditPressed( wxCommandEvent& );
	public:
		tEditablePropertyScriptString( ) : mScriptCursorPos( 0 ) { }
		explicit tEditablePropertyScriptString( const std::string& name, const std::string& initState = "" );
		explicit tEditablePropertyScriptString( const std::string& name, b32 allowDelete, b32 lockFieldtoName );
		virtual void fCreateGui( tCreateGuiData& data );
		virtual void fRefreshGui( );
		virtual void fClearGui( );
		virtual tEditableProperty* fClone( ) const;

		void fSetStaticText( const std::string& text ) { mStaticText = text; }
	protected:
		virtual void fGetRawData( Rtti::tClassId cid, void* dst, u32 size ) const;
		virtual void fSetRawData( Rtti::tClassId cid, const void* src, u32 size );

		template<class tSerializer>
		void fSerialize( tSerializer& s )
		{
			tRawDataEditableProperty<std::string>::fSerializeXml( s );
			s( "AllowDelete", mAllowDelete );
			s( "LockFieldToName", mLockFieldToName );
		}

		virtual void fSerializeXml( tXmlSerializer& s ) { fSerialize( s ); }
		virtual void fSerializeXml( tXmlDeserializer& s ) { fSerialize( s ); }
	};

	class tools_export tEditablePropertyFileNameString : public tRawDataEditableProperty<std::string>
	{
		implement_rtti_serializable_base_class( tEditablePropertyFileNameString, 0x13FF3201 );
	private:
		class tools_export tTextBox : public tWxSlapOnTextBox
		{
			tEditablePropertyFileNameString* mOwnerProp;
			wxWindow* mParent;
		public:
			tTextBox( tEditablePropertyFileNameString* ownerProp, wxWindow* parent, const char* label, b32 labelIsButton = false ) 
				: tWxSlapOnTextBox( parent, label, -1, labelIsButton ), mOwnerProp( ownerProp ), mParent( parent ) { }
			virtual void fOnControlUpdated( ) { mOwnerProp->fSetData<std::string>( fGetValue( ) ); }
			virtual void fOnLabelButtonClicked( wxCommandEvent& event );
		};
		tTextBox* mTextBox;
		wxButton* mBrowseButton;
	public:
		static void fAddFilter( const std::string& propName, const std::string& filter );
		static void fSetDefaultBrowseDirectory( const std::string& propName, const std::string& defaultBrowseDirectory );
		static void fSetDefaultFileName( const std::string& propName, const std::string& defaultFileName );
		tEditablePropertyFileNameString( ) : mTextBox( 0 ), mBrowseButton( 0 ) { }
		explicit tEditablePropertyFileNameString( const std::string& name, const std::string& initState = "" );
		virtual void fCreateGui( tCreateGuiData& data );
		virtual void fRefreshGui( );
		virtual void fClearGui( );
		virtual tEditableProperty* fClone( ) const;

	private:
		void fOnBrowse( wxCommandEvent& );
		b32 fButtonSupportedFile( const std::string& label );
	protected:
		virtual void fGetRawData( Rtti::tClassId cid, void* dst, u32 size ) const;
		virtual void fSetRawData( Rtti::tClassId cid, const void* src, u32 size );
	};

	class tools_export tEditablePropertyCustomString : public tRawDataEditableProperty<std::string>
	{
		implement_rtti_serializable_base_class( tEditablePropertyCustomString, 0x7E0D2DEB );
	private:
		class tools_export tTextBox : public tWxSlapOnTextBox
		{
			tEditablePropertyCustomString* mOwnerProp;
		public:
			tTextBox( tEditablePropertyCustomString* ownerProp, wxWindow* parent, const char* label ) 
				: tWxSlapOnTextBox( parent, label ), mOwnerProp( ownerProp ) { }
			virtual void fOnControlUpdated( ) { mOwnerProp->fSetData<std::string>( fGetValue( ) ); }
		};
		tTextBox* mTextBox;
		wxButton* mBrowseButton;
	public:
		typedef tDelegate<b32 ( tEditablePropertyCustomString& prop, std::string& newValue )> tCallbackFunction;
		struct tCallbackData
		{
			tCallbackFunction mFunc;
		};
		static void fAddCallback( const std::string& propName, const tCallbackData& filter );
		tEditablePropertyCustomString( ) : mTextBox( 0 ), mBrowseButton( 0 ) { }
		explicit tEditablePropertyCustomString( const std::string& name, const std::string& initState = "" );
		virtual void fCreateGui( tCreateGuiData& data );
		virtual void fRefreshGui( );
		virtual void fClearGui( );
		virtual tEditableProperty* fClone( ) const;
	private:
		void fOnBrowse( wxCommandEvent& );
	protected:
		virtual void fGetRawData( Rtti::tClassId cid, void* dst, u32 size ) const;
		virtual void fSetRawData( Rtti::tClassId cid, const void* src, u32 size );
	};

	/*
		This will create an array of buttons as a property item.
		When the user clicks the buttons the property will be set to the button name.
		Use OnPropertyChanged to detect this and execute your behavior.
		Search SigInternal for use examples.
	*/
	class tEditablePropertyButtons : public tEditablePropertyString
	{
		implement_rtti_serializable_base_class( tEditablePropertyButtons, 0xFB5955D1 );
	public:
		tEditablePropertyButtons( );
		tEditablePropertyButtons( const std::string& name, const std::string& button, const std::string& rawData = "" );
		tEditablePropertyButtons( const std::string& name, const tDynamicArray<std::string>& buttons, const std::string& rawData = "" );

		virtual tEditableProperty* fClone( ) const
		{
			tEditablePropertyString* o = new tEditablePropertyButtons( mName, mButtonNames, mRawData );
			return o;
		}

		virtual void fCreateGui( tCreateGuiData& data );

		template<class tSerializer>
		void fSerialize( tSerializer& s )
		{
			tEditablePropertyString::fSerializeXml( s );
			s( "Buttons", mButtonNames );
		}

		virtual void fSerializeXml( tXmlSerializer& s ) { fSerialize( s ); }
		virtual void fSerializeXml( tXmlDeserializer& s ) { fSerialize( s ); }

	private:
		tDynamicArray<std::string> mButtonNames;
		void fButtonClicked( wxCommandEvent& x );

	};

	/*
		This templated class will allow you to to store a struct full of data as a property.
		It will additionally allow you to register a dialog that will handle the display of this data,
		since the default object properties was obviously not sufficient to warrant the need of custom data.

	*/
	template< typename tUserData, typename tDialog >
	class tools_export tEditablePropertyUserData : public tRawDataEditableProperty< tUserData >
	{
		//You will need to provide your own base id, since you are specifying a new type of tUserData
		//implement_rtti_serializable_base_class( tEditablePropertyUserData, 0x8D0D61D9 );
	public:
		tEditablePropertyUserData( ) { }
		explicit tEditablePropertyUserData( const std::string& name, const tUserData& data = tUserData( ) )
			: tRawDataEditableProperty<tUserData>( name, data )
		{ }

	protected:
		virtual void fCreateGui( tCreateGuiData& data )	{ tDialog::fAddProperty( *this, data.mSelection ); }
		virtual void fRefreshGui( )						{ tDialog::fRefreshProperty( *this ); }
		virtual void fClearGui( )						{ tDialog::fRemoveProperty( *this ); }

		virtual void fGetRawData( Rtti::tClassId cid, void* dst, u32 size ) const
		{
			sigassert( size == sizeof( mRawData ) );
			*( tUserData* )dst = mRawData;
		}
		virtual void fSetRawData( Rtti::tClassId cid, const void* src, u32 size )
		{
			sigassert( size == sizeof( mRawData ) );
			mRawData = *( tUserData* )src;
		}

		template<class tSerializer>
		void fSerialize( tSerializer& s )
		{
			tRawDataEditableProperty<tUserData>::fSerializeXml( s );
		}

		virtual void fSerializeXml( tXmlSerializer& s ) { fSerialize( s ); }
		virtual void fSerializeXml( tXmlDeserializer& s ) { fSerialize( s ); }
	};



	template<class tVecType>
	class tEditablePropertyVectorBase : public tRawDataEditableProperty<tVecType>
	{
	protected:

		f32 mMin, mMax, mIncrement;
		u32 mPrecision;
		b32 mAllowDelete;

		class tools_export tSpinner : public tWxSlapOnSpinner
		{
			tEditablePropertyVectorBase* mOwnerProp;
		public:
			tSpinner( tEditablePropertyVectorBase* ownerProp, wxWindow* parent, const std::string& label, f32 min, f32 max, f32 increment, u32 precision ) 
				: tWxSlapOnSpinner( parent, label.c_str( ), min, max, increment, precision ), mOwnerProp( ownerProp ) { }
			virtual void fOnControlUpdated( ) { mOwnerProp->fOnControlUpdated( ); }
		};
		tFixedArray< tSpinner*, tVecType::cDimension > mSpinners;
	public:

		tEditablePropertyVectorBase( )
			: mMin( -1.f ), mMax( +1.f ), mIncrement( 0.1f ), mPrecision( 1 ), mAllowDelete( false )
		{
			fZeroOut( mSpinners );
		}

		explicit tEditablePropertyVectorBase( 
			const std::string& name, 
			const tVecType& initState,
			f32 min, f32 max, f32 increment, u32 precision, b32 allowDelete )
			: tRawDataEditableProperty<tVecType>( name, initState )
			, mMin( min )
			, mMax( max )
			, mIncrement( increment )
			, mPrecision( precision )
			, mAllowDelete( allowDelete )
		{
			fZeroOut( mSpinners );
		}

		virtual void fCreateGui( tCreateGuiData& data )
		{
			for( u32 i = 0; i < mSpinners.fCount( ); ++i )
			{
				mSpinners[ i ] = new tSpinner( this, data.mParent, fIthSpinnerName( data.mLabel, i ), mMin, mMax, mIncrement, mPrecision );
				mSpinners[ i ]->fSetToolTip( fDisplayOptions( ).mToolTip );
			}

			fRefreshGui( );

			if( mAllowDelete )
				fAddRemoveButton( data.mParent, *mSpinners.fBack( ) );
		}

		virtual void fRefreshGui( )
		{
			for( u32 i = 0; i < mSpinners.fCount( ); ++i )
			{
				if( mConflictFlags & ( 1 << i ) )
					mSpinners[ i ]->fSetIndeterminateNoEvent( );
				else
					mSpinners[ i ]->fSetValueNoEvent( mRawData.fAxis( i ) );
			}
		}

		virtual void fClearGui( )
		{
			fZeroOut( mSpinners );
		}

	protected:

		virtual void fGetRawData( Rtti::tClassId cid, void* dst, u32 size ) const
		{
			const b32 isInt = ( cid == Rtti::fGetClassId<s32>( ) || cid == Rtti::fGetClassId<u32>( ) );
			const b32 imFloat = ( Rtti::fGetClassId( mRawData.x ) == Rtti::fGetClassId<f32>( ) );
			if( tVecType::cDimension == 1 && isInt && imFloat )
				*( int* )dst = ( int )mRawData.x;
			else
			{
				sigassert( cid == Rtti::fGetClassId<tVecType>( ) || ( tVecType::cDimension == 1 && cid == Rtti::fGetClassId<f32>( ) ) );
				sigassert( size == sizeof( mRawData ) );
				fMemCpy( dst, &mRawData, size );
			}
		}

		virtual void fSetRawData( Rtti::tClassId cid, const void* src, u32 size )
		{
			const b32 isInt = ( cid == Rtti::fGetClassId<s32>( ) || cid == Rtti::fGetClassId<u32>( ) );
			const b32 imFloat = ( Rtti::fGetClassId( mRawData.x ) == Rtti::fGetClassId<f32>( ) );
			if( tVecType::cDimension == 1 && isInt && imFloat )
				mRawData.x = ( f32 )*( int* )src;
			else
			{
				sigassert( cid == Rtti::fGetClassId<tVecType>( ) || ( tVecType::cDimension == 1 && cid == Rtti::fGetClassId<f32>( ) ) );
				sigassert( size == sizeof( mRawData ) );
				fMemCpy( &mRawData, src, size );
			}
		}

		template<class tSerializer>
		void fSerialize( tSerializer& s )
		{
			tRawDataEditableProperty<tVecType>::fSerializeXml( s );
			s( "Min", mMin );
			s( "Max", mMax );
			s( "Increment", mIncrement );
			s( "Precision", mPrecision );
			s( "AllowDelete", mAllowDelete );
		}

		virtual void fSerializeXml( tXmlSerializer& s ) { fSerialize( s ); }
		virtual void fSerializeXml( tXmlDeserializer& s ) { fSerialize( s ); }

		virtual std::string fIthSpinnerName( const std::string& baseLabel, u32 i ) const
		{
			if( tVecType::cDimension > 1 )
			{
				switch( i )
				{
				case 0: return baseLabel + ".x";
				case 1: return baseLabel + ".y";
				case 2: return baseLabel + ".z";
				case 3: return baseLabel + ".w";
				default: sigassert( !"invalid case" ); break;
				}
			}

			return baseLabel;
		}

		void fOnControlUpdated( )
		{
			tVecType v;
			for( u32 i = 0; i < mSpinners.fCount( ); ++i )
				v.fAxis( i ) = mSpinners[ i ]->fGetValue( );
			fSetData( v );
		}

		virtual u32 fComputeConflictFlags( const tEditableProperty& other ) const
		{
			tVecType othersValue;
			other.fGetData( othersValue );

			u32 flags = 0;
			for( u32 i = 0; i < tVecType::cDimension; ++i )
			{
				if( !Sig::fEqual( mRawData.fAxis( i ), othersValue.fAxis( i ) ) )
					flags |= ( 1 << i );
			}
			return mConflictFlags | flags;
		}
	};

	class tools_export tEditablePropertyFloat : public tEditablePropertyVectorBase<Math::tVec1f>
	{
		implement_rtti_serializable_base_class( tEditablePropertyFloat, 0xE9F584DF );
	public:
		tEditablePropertyFloat( ) { }
		explicit tEditablePropertyFloat( 
			const std::string& name, 
			f32 initState,
			f32 min, f32 max, f32 increment, u32 precision, b32 allowDelete = false );
		virtual tEditableProperty* fClone( ) const;
	};

	/// 
	/// \brief CDC stands for Centers for Disease Control. And Camera Distance Capture.
	class tools_export tEditablePropertyCDCFloat : public tEditablePropertyVectorBase<Math::tVec1f>
	{
		implement_rtti_serializable_base_class( tEditablePropertyCDCFloat, 0xC5C01529 );
	public:
		tEditablePropertyCDCFloat( ) { }
		explicit tEditablePropertyCDCFloat( 
			const std::string& name, 
			f32 initState,
			f32 min, f32 max, f32 increment, u32 precision, b32 allowDelete = false );
		virtual tEditableProperty* fClone( ) const;
		virtual void fCreateGui(tCreateGuiData& data );
		typedef tDelegate<b32 ( tEditablePropertyCDCFloat& prop, f32& newValue )> tCallbackFunction;
		struct tCallbackData
		{
			tCallbackFunction mFunc;
		};
		static void fAddCallback( const std::string& propName, const tCallbackData& filter );
	private:
		void fOnCapturePressed( wxCommandEvent& );
	};

	class tools_export tEditablePropertyVec2f : public tEditablePropertyVectorBase<Math::tVec2f>
	{
		implement_rtti_serializable_base_class( tEditablePropertyVec2f, 0x68CAC566 );
	public:
		tEditablePropertyVec2f( ) { }
		explicit tEditablePropertyVec2f( 
			const std::string& name, 
			const Math::tVec2f& initState,
			f32 min, f32 max, f32 increment, u32 precision, b32 allowDelete = false );
		virtual tEditableProperty* fClone( ) const;
	};

	class tools_export tEditablePropertyVec3f : public tEditablePropertyVectorBase<Math::tVec3f>
	{
		implement_rtti_serializable_base_class( tEditablePropertyVec3f, 0xABF577E7 );
	public:
		tEditablePropertyVec3f( ) { }
		explicit tEditablePropertyVec3f( 
			const std::string& name, 
			const Math::tVec3f& initState,
			f32 min, f32 max, f32 increment, u32 precision, b32 allowDelete = false );
		virtual tEditableProperty* fClone( ) const;
	};

	class tools_export tEditablePropertyVec4f : public tEditablePropertyVectorBase<Math::tVec4f>
	{
		implement_rtti_serializable_base_class( tEditablePropertyVec4f, 0x322793F4 );
	public:
		tEditablePropertyVec4f( ) { }
		explicit tEditablePropertyVec4f( 
			const std::string& name, 
			const Math::tVec4f& initState,
			f32 min, f32 max, f32 increment, u32 precision, b32 allowDelete = false );
		virtual tEditableProperty* fClone( ) const;
	};

	class tools_export tEditablePropertyVec2i : public tEditablePropertyVectorBase<Math::tVec2i>
	{
		implement_rtti_serializable_base_class( tEditablePropertyVec2i, 0xF1C9F5E2 );
	public:
		tEditablePropertyVec2i( ) { }
		explicit tEditablePropertyVec2i( 
			const std::string& name, 
			const Math::tVec2i& initState,
			s32 min, s32 max, s32 increment, b32 allowDelete = false );
		virtual tEditableProperty* fClone( ) const;
	};

	class tools_export tEditablePropertyVec3i : public tEditablePropertyVectorBase<Math::tVec3i>
	{
		implement_rtti_serializable_base_class( tEditablePropertyVec3i, 0xB8B24857 );
	public:
		tEditablePropertyVec3i( ) { }
		explicit tEditablePropertyVec3i( 
			const std::string& name, 
			const Math::tVec3i& initState,
			s32 min, s32 max, s32 increment, b32 allowDelete = false );
		virtual tEditableProperty* fClone( ) const;
	};
}


#endif//__tEditablePropertyTypes__
