#ifndef __tEditableProperty__
#define __tEditableProperty__
#include "tDelegate.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"

class wxWindow;

namespace Sig
{
	class tEditableProperty;
	class tEditablePropertyTable;
	class tWxSlapOnControl;

	typedef tRefCounterPtr< tEditableProperty > tEditablePropertyPtr;

	///
	/// \brief Base type for editable properties. Editable properties represent fairly
	/// simple types (int, float, vector, matrix, string) generally, but in theory can be more complex.
	/// Fundamental abilities of an editable property are xml serialization and automatic gui form
	/// population.
	class tools_export tEditableProperty : public wxEvtHandler, public Rtti::tSerializableBaseClass, public tRefCounter
	{
		friend class tEditablePropertyTable;
		declare_null_reflector( );
	public:
		struct tDisplayOptions
		{
			b32 mShow;
			u32 mOrder;
			std::string mToolTip;

			tDisplayOptions( b32 show = true, u32 order = 0, const std::string toolTip = std::string( ) )
				: mShow( show ), mOrder( order  ), mToolTip( toolTip )
			{ }
		};

	protected:

		tEditablePropertyTable*					mOwner;
		tEditableProperty*						mGuiOwner;
		std::string								mName;
		u32										mConflictFlags; // indicates a conflict in value with any/all dependents
		tGrowableArray< tEditablePropertyPtr >	mDependents;

		tDisplayOptions							mDisplayOptions;

	public:

		tEditableProperty( );
		explicit tEditableProperty( const std::string& name );

		///
		/// \brief Access the name string.
		inline const std::string& fGetName( ) const { return mName; }

		///
		/// \brief Get and set different usability aspects of the property.
		const tDisplayOptions& fDisplayOptions( ) const { return mDisplayOptions; }
		tDisplayOptions& fDisplayOptions( ) { return mDisplayOptions; }

		///
		/// \brief Access the property owner which actually manages the gui.
		tEditableProperty* fGuiOwner( ) const { return mGuiOwner; }

		///
		/// \brief Query if the given property is of the same type and the underlying
		/// raw data is equivalent (i.e., if two floats, that they're equal, etc.).
		b32 fEqual( const tEditableProperty& other ) const;

		///
		/// \brief Clears list of dependents.
		void fClearDependents( );

		///
		/// \brief See if we have a conflict.
		inline b32 fHasConflict( ) const { return mConflictFlags != 0; }

		///
		/// \brief Obtain a copy of the underlying raw data (i.e., float, vector, matrix, whatever).
		template<class t>
		t& fGetData( t& o ) const
		{
			fGetRawData( Rtti::fGetClassId<t>( ), &o, sizeof( o ) );
			return o;
		}

		///
		/// \brief Set via copying by value the underlying raw data (i.e., float, vector, matrix, whatever).
		template<class t>
		void fSetDataNoNotify( const t& o, b32 setDependents = true )
		{
			fSetRawData( Rtti::fGetClassId<t>( ), &o, sizeof( o ) );

			if( setDependents )
			{
				for( u32 i = 0; i < mDependents.fCount( ); ++i )
				{
					tEditableProperty* guiOwner = mDependents[ i ]->mGuiOwner; // save off
					mDependents[ i ]->mGuiOwner = 0; // temporarily set to null
					mDependents[ i ]->fSetData( o, true );
					mDependents[ i ]->mGuiOwner = guiOwner; // restore
				}
				mConflictFlags = 0;
			}
		}

		template<class t>
		void fSetData( const t& o, b32 setDependents = true )
		{
			if( mGuiOwner )
			{
				mGuiOwner->fSetData( o, setDependents );
				mGuiOwner->fRefreshGui( );
			}
			else
			{
				fSetDataNoNotify( o, setDependents );
				fNotifyOwnerOfChange( );
			}
		}

		///
		/// \brief Serialize to xml file.
		virtual void fSerializeXml( tXmlSerializer& s ) = 0;

		///
		/// \brief Deserialize from xml file.
		virtual void fSerializeXml( tXmlDeserializer& s ) = 0;

		///
		/// \brief Create gui objects and add them to specified parent window.
		virtual void fCreateGui( wxWindow* parent, const std::string& label ) = 0;

		///
		/// \brief Refreshes existing gui from current value.
		virtual void fRefreshGui( ) = 0;

		///
		/// \brief Clear any retained gui pointers, as they have been deleted.
		virtual void fClearGui( ) = 0;

		///
		/// \brief Clone the property, user implemented.
		virtual tEditableProperty* fClone( ) const = 0;

		///
		/// \brief Clone the property and copy over base data.
		tEditablePropertyPtr fGetClone( ) const
		{
			tEditablePropertyPtr result( fClone( ) );

			// copy over base data
			result->fDisplayOptions( ) = fDisplayOptions( );

			return result;
		}

	private:

		///
		/// \brief Used to notify owner of modification.
		void fNotifyOwnerOfChange( );

	protected:

		///
		/// \brief Get the raw data represented by the property, and store in 'dst'. Derived
		/// type should validate the class id of the raw data 'cid', as well as the size.
		virtual void fGetRawData( Rtti::tClassId cid, void* dst, u32 size ) const = 0;

		///
		/// \brief Set the raw data represented by the property, using 'src' as the source. Derived
		/// type should validate the class id of the raw data 'cid', as well as the size.
		virtual void fSetRawData( Rtti::tClassId cid, const void* src, u32 size ) = 0;

		///
		/// \brief This method can assume that 'other' is safely of the property's derived type,
		/// and hence can static_cast to it.
		virtual b32 fEqualByType( const tEditableProperty& other ) const = 0;

		///
		/// \brief For vectorized derived types, you may have sub-conflicts and sub-agreements,
		/// hence requiring multiple bits of conflicts. The default implemenation just checks 'equal'.
		virtual u32 fComputeConflictFlags( const tEditableProperty& other ) const;

		void fAddRemoveButton( wxWindow* parent, tWxSlapOnControl& ctrl );

	private:

		void fOnRemovePressed( wxCommandEvent& );
	};

	class tPropertyKeyEqual
	{
	public:
		b32 operator( )( const std::string& a, const std::string& b ) const
		{
			return _stricmp( a.c_str( ), b.c_str( ) ) == 0;
		}
	};
	typedef tHashTable< std::string, tEditablePropertyPtr, tHashTableExpandAndShrinkResizePolicy, tHashStringICase, tPropertyKeyEqual > tEditablePropertyTableBase;

	///
	/// \brief Represents a collection of editable properties of potentially diverse type.
	class tools_export tEditablePropertyTable : public tEditablePropertyTableBase
	{
	public:
		typedef tEvent<void ( tEditableProperty& )> tOnPropertyChanged;
		typedef tEvent<void ( tEditableProperty& )> tOnPropertyWantsRemoval;
	public:
		static u32 fAddPropsToWindow( wxWindow* panel, tEditablePropertyTable& commonProps, b32 collapsibleGroups = true );
	public:
		tOnPropertyChanged mOnPropertyChanged;
		tOnPropertyWantsRemoval mOnPropertyWantsRemoval;
	public:
		tEditablePropertyTable( );
		tEditablePropertyTable( const tEditablePropertyTable& other );
		tEditablePropertyTable& operator=( const tEditablePropertyTable& other );
		~tEditablePropertyTable( );
		tEditablePropertyPtr* fInsert( const tEditablePropertyPtr& prop );
		tEditablePropertyPtr fFindPartial( const char* partialName );
		b32 fRemove( const std::string& key );
		void fCollectCommonPropertiesForGui( const tEditablePropertyTable& other );
		void fUnion( const tEditablePropertyTable& other, b32 priorityToOther = true );
		void fClearGui( );
		void fSerializeXml( tXmlSerializer& s );
		void fSerializeXml( tXmlDeserializer& s );
		void fClone( tEditablePropertyTable& clone ) const;
		void fNotifyPropertyChanged( tEditableProperty& property );
		void fNotifyPropertyWantsRemoval( tEditableProperty& property );
		void fToSortedList( tGrowableArray<tEditablePropertyPtr>& propsOut ) const;
		void fGetGroup( const char* groupName, tGrowableArray<tEditablePropertyPtr>& propsOut ) const;

		template<class t>
		t fGetValue( const std::string& key, const t& defValue )
		{
			t o = defValue;
			tEditablePropertyPtr* find = fFind( key );
			if( find )
				(*find)->fGetData( o );
			return o;
		}

		template<class t>
		void fSetData( const std::string& key, const t& o, b32 setDependents = true )
		{
			tEditablePropertyPtr* find = fFind( key );
			if( find )
				(*find)->fSetData( o, setDependents );
		}

		template<class t>
		void fSetDataNoNotify( const std::string& key, const t& o, b32 setDependents = true )
		{
			tEditablePropertyPtr* find = fFind( key );
			if( find )
				(*find)->fSetDataNoNotify( o, setDependents );
		}

	protected:
		tEditablePropertyPtr& operator[]( const std::string& key );
	};
}

#endif//__tEditableProperty__
