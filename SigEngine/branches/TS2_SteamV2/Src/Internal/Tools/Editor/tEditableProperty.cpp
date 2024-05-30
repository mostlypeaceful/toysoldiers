#include "ToolsPch.hpp"
#include "tEditableProperty.hpp"
#include "tWxSlapOnControl.hpp"
#include "tWxSlapOnGroup.hpp"
#include "Sigml.hpp"

namespace Sig
{
	///
	/// \section tEditableProperty
	///

	tEditableProperty::tEditableProperty( )
		: mOwner( 0 )
		, mGuiOwner( 0 )
		, mName( "" )
		, mConflictFlags( 0 )
	{
	}

	tEditableProperty::tEditableProperty( const std::string& name )
		: mOwner( 0 )
		, mGuiOwner( 0 )
		, mName( name )
		, mConflictFlags( 0 )
	{
	}

	b32 tEditableProperty::fEqual( const tEditableProperty& other ) const
	{
		if( fClassId( ) != other.fClassId( ) )
			return false;

		return fEqualByType( other );
	}

	void tEditableProperty::fClearDependents( )
	{
		mConflictFlags = 0;
		for( u32 i = 0; i < mDependents.fCount( ); ++i )
			mDependents[ i ]->mGuiOwner = 0;
		mDependents.fSetCount( 0 );
	}

	void tEditableProperty::fNotifyOwnerOfChange( )
	{
		if( mOwner )
		{
			mOwner->fNotifyPropertyChanged( *this );
		}
	}

	u32 tEditableProperty::fComputeConflictFlags( const tEditableProperty& other ) const
	{
		return mConflictFlags || !fEqual( other );
	}

	void tEditableProperty::fAddRemoveButton( wxWindow* parent, tWxSlapOnControl& ctrl )
	{
		wxButton* button = new wxButton( parent, wxID_ANY, "X", wxDefaultPosition, wxSize( 20, 20 ) );
		button->SetForegroundColour( wxColour( 0x99, 0x00, 0x00 ) );
		button->SetToolTip( "Delete Property" );
		ctrl.fAddWindowToSizer( button, true );
		button->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEditableProperty::fOnRemovePressed ), NULL, this );
	}

	void tEditableProperty::fOnRemovePressed( wxCommandEvent& )
	{
		if( mOwner )
			mOwner->fNotifyPropertyWantsRemoval( *this );
	}

	///
	/// \section tEditablePropertyTable
	///

	namespace
	{
		struct tSortEditableProperties
		{
			inline b32 operator( )( const tEditablePropertyPtr& a, const tEditablePropertyPtr& b ) const
			{
				u32 orderA = a->fDisplayOptions( ).mOrder;
				u32 orderB = b->fDisplayOptions( ).mOrder;

				if( orderA == orderB )
				{
					// HACK: make object name always appear as first in the list (this is safe, just hacky and introduces unnecessary dependency on Sigml.hpp)
					if( a->fGetName( ) == Sigml::tObjectProperties::fEditablePropObjectName( ) ) return true;
					if( b->fGetName( ) == Sigml::tObjectProperties::fEditablePropObjectName( ) ) return false;
					return a->fGetName( ) < b->fGetName( );
				}
				else
					return a->fDisplayOptions( ).mOrder < b->fDisplayOptions( ).mOrder;
			}
		};
	}

	u32 tEditablePropertyTable::fAddPropsToWindow( wxWindow* panel, tEditablePropertyTable& commonProps, b32 collapsibleGroups )
	{
		typedef tHashTable< std::string, tWxSlapOnGroup* > tGroupMap;
		tGroupMap groupMap( 32 );

		tGrowableArray<tEditablePropertyPtr> commonPropsSorted;
		commonProps.fToSortedList( commonPropsSorted );

		// create gui for all common properties
		for( u32 i = 0; i < commonPropsSorted.fCount( ); ++i )
		{
			if( !commonPropsSorted[ i ]->fDisplayOptions( ).mShow ) continue;

			std::string groupName, realName;

			const std::string key = commonPropsSorted[ i ]->fGetName( );
			const size_t dot = key.find_last_of( "." );
			if( dot == std::string::npos || dot == key.length( ) - 1 )
			{
				groupName = "";
				realName = key;
			}
			else
			{
				groupName = std::string( &key[0], &key[dot] ); if( groupName == "$" ) groupName = "Common";
				realName = std::string( &key[dot+1], &key[key.length( )-1] + 1 );
			}

			tWxSlapOnGroup** group = groupMap.fFind( groupName );
			if( !group )
				group = groupMap.fInsert( groupName, new tWxSlapOnGroup( panel, groupName.c_str( ), collapsibleGroups ) );
			sigassert( group && *group );

			commonPropsSorted[ i ]->fCreateGui( ( *group )->fGetMainPanel( ), realName );
		}

		u32 totalHeight = 0;
		for( tGroupMap::tIterator i = groupMap.fBegin( ); i != groupMap.fEnd( ); ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			i->mValue->fGetMainPanel( )->GetSizer( )->AddSpacer( 8 );
			i->mValue->fGetMainPanel( )->Layout( );
			totalHeight += i->mValue->fGetMainPanel( )->GetSize( ).y;
		}
		return totalHeight;
	}

	tEditablePropertyTable::tEditablePropertyTable( )
		: tEditablePropertyTableBase( 32 )
		, mOnPropertyChanged( false )
		, mOnPropertyWantsRemoval( false )
	{
	}

	tEditablePropertyTable::tEditablePropertyTable( const tEditablePropertyTable& other )
		: tEditablePropertyTableBase( 32 )
		, mOnPropertyChanged( false )
		, mOnPropertyWantsRemoval( false )
	{
		other.fClone( *this );
	}
	tEditablePropertyTable& tEditablePropertyTable::operator=( const tEditablePropertyTable& other )
	{
		if( &other == this ) return *this;
		other.fClone( *this );
		return *this;
	}
	tEditablePropertyTable::~tEditablePropertyTable( )
	{
		for( tIterator i = fBegin( ); i != fEnd( ); ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			i->mValue->mOwner = 0;
		}
	}

	tEditablePropertyPtr* tEditablePropertyTable::fInsert( const tEditablePropertyPtr& prop )
	{
		tEditablePropertyPtr* find = fFind( prop->fGetName( ) );
		if( find )
		{
			if( prop->fClassId( ) == (*find)->fClassId( ) )
				return find;
			else
				tEditablePropertyTableBase::fRemove( find );
		}

		find = tEditablePropertyTableBase::fInsert( prop->fGetName( ), prop );
		sigassert( find );
		(*find)->mOwner = this;
		return find;
	}

	tEditablePropertyPtr tEditablePropertyTable::fFindPartial( const char* partialName )
	{
		for( tIterator i = fBegin( ); i != fEnd( ); ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			if( StringUtil::fStrStrI( i->mValue->fGetName( ).c_str( ), partialName ) )
				return i->mValue;
		}
		return tEditablePropertyPtr( );
	}

	b32 tEditablePropertyTable::fRemove( const std::string& key )
	{
		tEditablePropertyPtr* find = fFind( key );
		if( find )
		{
			(*find)->mOwner = 0;
			tEditablePropertyTableBase::fRemove( find );
			return true;
		}

		return false;
	}

	void tEditablePropertyTable::fCollectCommonPropertiesForGui( const tEditablePropertyTable& other )
	{
		{
			tEditablePropertyTable intersection;

			for( tIterator i = fBegin( ); i != fEnd( ); ++i )
			{
				if( i->fNullOrRemoved( ) )
					continue;

				tEditablePropertyPtr* find = other.fFind( i->mKey );
				if( find && ( i->mValue->fClassId( ) == (*find)->fClassId( ) ) )
				{
					// make sure it's not the same pointer
					sigassert( i->mValue != *find );

					i->mValue->mDependents.fPushBack( *find );
					i->mValue->mConflictFlags = i->mValue->fComputeConflictFlags( **find );
					intersection.fInsert( i->mValue );
				}
			}

			fSwap( intersection );
		}

		for( tIterator i = fBegin( ); i != fEnd( ); ++i )
		{
			if( i->fNullOrRemoved( ) ) continue;

			tEditableProperty* guiOwner = i->mValue.fGetRawPtr( );
			guiOwner->mOwner = this;
			guiOwner->mGuiOwner = 0;
			for( u32 i = 0; i < guiOwner->mDependents.fCount( ); ++i )
				guiOwner->mDependents[ i ]->mGuiOwner = guiOwner;
		}
	}

	void tEditablePropertyTable::fUnion( const tEditablePropertyTable& other, b32 priorityToOther )
	{
		for( tConstIterator i = other.fBegin( ); i != other.fEnd( ); ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;

			if( priorityToOther )
			{
				fRemove( i->mKey );
				fInsert( i->mValue->fGetClone( ) );
			}
			else
			{
				tEditablePropertyPtr* find = fFind( i->mKey );
				if( !find )
					fInsert( i->mValue->fGetClone( ) );
			}
		}
	}

	void tEditablePropertyTable::fClearGui( )
	{
		for( tIterator i = fBegin( ); i != fEnd( ); ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			i->mValue->fClearDependents( );
			i->mValue->fClearGui( );
		}
	}

	void tEditablePropertyTable::fSerializeXml( tXmlSerializer& s )
	{
		// convert hash table to array
		tGrowableArray< tEditablePropertyPtr > items;
		items.fSetCapacity( fGetItemCount( ) );

		fToSortedList( items );

		// serialize the array
		fSerializeXmlObject( s, items );
	}

	void tEditablePropertyTable::fSerializeXml( tXmlDeserializer& s )
	{
		// deserialize the array
		tGrowableArray< tEditablePropertyPtr > items;
		fSerializeXmlObject( s, items );

		// clear current
		fClear( 32 );

		// convert to hash table
		for( u32 i = 0; i < items.fCount( ); ++i )
			fInsert( items[ i ] );
	}

	void tEditablePropertyTable::fClone( tEditablePropertyTable& clone ) const
	{
		clone.fClear( 32 );
		for( tConstIterator i = fBegin( ); i != fEnd( ); ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			clone.fInsert( i->mValue->fGetClone( ) );
		}
	}

	void tEditablePropertyTable::fNotifyPropertyChanged( tEditableProperty& property )
	{
		mOnPropertyChanged.fFire( property );
	}

	void tEditablePropertyTable::fNotifyPropertyWantsRemoval( tEditableProperty& property )
	{
		mOnPropertyWantsRemoval.fFire( property );
	}

	void tEditablePropertyTable::fToSortedList( tGrowableArray<tEditablePropertyPtr>& propsOut ) const
	{
		for( tConstIterator i = fBegin( ); i != fEnd( ); ++i )
		{
			if( i->fNullOrRemoved( ) ) continue;
			propsOut.fPushBack( tEditablePropertyPtr( i->mValue ) );
		}

		std::sort( propsOut.fBegin( ), propsOut.fEnd( ), tSortEditableProperties( ) );
	}

	void tEditablePropertyTable::fGetGroup( const char* groupName, tGrowableArray<tEditablePropertyPtr>& propsOut ) const
	{
		const u32 groupNameStrLen = ( u32 )strlen( groupName );

		for( tConstIterator i = fBegin( ); i != fEnd( ); ++i )
		{
			if( i->fNullOrRemoved( ) ) continue;
			if( strncmp( groupName, i->mKey.c_str( ), groupNameStrLen ) != 0 ) continue;
			propsOut.fPushBack( tEditablePropertyPtr( i->mValue ) );
		}

		std::sort( propsOut.fBegin( ), propsOut.fEnd( ), tSortEditableProperties( ) );
	}

}

