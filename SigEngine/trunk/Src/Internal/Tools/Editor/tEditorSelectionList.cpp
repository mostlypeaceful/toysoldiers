#include "ToolsPch.hpp"
#include "tEditorSelectionList.hpp"
#include "tEditableObject.hpp"

namespace Sig
{
	tEditorSelectionList::tEditorSelectionList( )
		: mOnSelChanged( false )
	{
	}

	void tEditorSelectionList::fAdd( const tEntityPtr& entity, b32 select )
	{
		mObjects.fPushBack( entity );
		if( select )
		{
			tEditableObject* eo = entity->fDynamicCast< tEditableObject >();
			if( eo->fIsHidden() || !eo->fIsSelectable() )
				mObjects.fErase( mObjects.fCount( ) - 1 );
			else
				eo->fSetSelected( true );
		}

		mOnSelChanged.fFire( *this );
	}

	void tEditorSelectionList::fRemove( const tEntityPtr& entity, b32 deselect )
	{
		if( deselect )
			entity->fDynamicCast< tEditableObject >( )->fSetSelected( false );
		mObjects.fFindAndErase( entity );

		mOnSelChanged.fFire( *this );
	}

	void tEditorSelectionList::fClear( b32 deselect )
	{
		const b32 selChanged = ( mObjects.fCount( ) > 0 );

		if( deselect )
			fDeSelectAll( );
		mObjects.fSetCount( 0 );

		if( selChanged )
			mOnSelChanged.fFire( *this );
	}

	void tEditorSelectionList::fReset( const tEditorSelectionList& newList, b32 modifySelection )
	{
		if( modifySelection )
			fDeSelectAll( );

		mObjects = newList.mObjects;

		if( modifySelection )
			fSelectAll( );

		mOnSelChanged.fFire( *this );
	}

	void tEditorSelectionList::fSelectAll( )
	{
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
		{
			tEntityPtr master = mObjects[ i ];
			tEditableObject* eo = master->fDynamicCast< tEditableObject >( );
			if( eo->fIsHidden( ) )
			{
				mObjects.fErase( i );
				--i;
			}
			else
				eo->fSetSelected( true );
		}
	}

	void tEditorSelectionList::fDeSelectAll( )
	{
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
			mObjects[ i ]->fDynamicCast< tEditableObject >( )->fSetSelected( false );
	}

	void tEditorSelectionList::fDuplicateAndReplace( )
	{
		tEditorSelectionList newList;
		newList.mObjects.fSetCapacity( fCount( ) );
		
		for( u32 i = 0; i < fCount( ); ++i )
		{
			// Prevent non-serializable entities from causing null pointers.
			tEntityPtr clone = mObjects[ i ]->fDynamicCast< tEditableObject >( )->fClone( );
			if( !clone.fNull() )
				newList.fAdd( clone, false );
		}

		for( u32 i = 0; i < newList.fCount( ); ++i )
			newList[ i ]->fDynamicCast< tEditableObject >( )->fAfterAllObjectsCloned( newList );

		fReset( newList );
	}

	Math::tAabbf tEditorSelectionList::fComputeBounding( ) const
	{
		Math::tAabbf o, tmp;
		o.fInvalidate( );
		for( u32 i = 0; i < fCount( ); ++i )
		{
			tEditableObject* eo = mObjects[ i ]->fDynamicCast< tEditableObject >( );
			o |= eo->fWorldSpaceBox( );
		}
		return o;
	}

	Math::tVec3f tEditorSelectionList::fComputeAveragePosition( ) const
	{
		Math::tVec3f avgPos = Math::tVec3f::cZeroVector;
		for( u32 i = 0; i < fCount( ); ++i )
			avgPos += (*this)[ i ]->fDynamicCast< tEditableObject >( )->fObjectToWorld( ).fGetTranslation( );
		avgPos /= fCount( );
		return avgPos;
	}

	void tEditorSelectionList::fComputeSelectedDisplayStats( Gfx::tDisplayStats& displayStatsOut ) const
	{
		for( u32 i = 0; i < fCount( ); ++i )
			mObjects[ i ]->fDynamicCast< tEditableObject >( )->fComputeDisplayStats( displayStatsOut );
	}

	void tEditorSelectionList::fSnapToGround( b32 onlySnapIfSpecified ) const
	{
		tDynamicArray< tEntity* > toIgnore( fCount( ) );
		for( u32 i = 0; i < toIgnore.fCount( ); ++i )
			toIgnore[ i ] = (*this)[ i ]->fDynamicCast< tEditableObject >( );

		for( u32 i = 0; i < fCount( ); ++i )
		{
			tEditableObject* eo = (*this)[ i ]->fDynamicCast< tEditableObject >( );
			if( !onlySnapIfSpecified || eo->fGetEditableProperties( ).fGetValue<b32>( Sigml::tObject::fEditablePropGroundRelative( ), false ) )
				eo->fSnapToGround( toIgnore );
		}
	}

}

