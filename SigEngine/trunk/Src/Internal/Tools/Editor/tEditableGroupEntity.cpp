#include "ToolsPch.hpp"
#include "tEditableGroupEntity.hpp"
#include "tSceneGraph.hpp"




namespace Sig
{

	tEditableGroupEntity::tEditableGroupEntity( tEditableObjectContainer& container )
		: tEditableObject( container )
	{
		fCommonCtor( );
	}
	void tEditableGroupEntity::fCommonCtor( )
	{
		mSelectionBoxColor = Math::tVec4f( 1.f, 0.4f, 0.4f, 1.f );
	}

	tEditableGroupEntity::~tEditableGroupEntity( )
	{
	}

	std::string tEditableGroupEntity::fGetToolTip( ) const
	{
		if( fIsFrozen( ) ) return std::string( );
		const std::string name = tEditableObject::fGetToolTip( );
		if( name.length( ) > 0 )
			return "Group - " + name;
		return "Group";
	}

	void tEditableGroupEntity::fAddObjects( tEditorSelectionList& entities )
	{
		// Move the group entity to the center of all these selected entities.
		const Math::tVec3f averagePosition = entities.fComputeAveragePosition();
		fMoveTo( averagePosition );

		Math::tAabbf heldItemsBox;
		heldItemsBox.fInvalidate();
		for( u32 i = 0; i < entities.fCount(); ++i )
		{
			tEditableObject* eo = entities[i]->fDynamicCast< tEditableObject >( );

			// Set each object's parent, store the objects we're responsible for, determine new selection box.
			eo->fSetGroupParent( this );
			mHeldEntities.fPushBack( entities[i] );
			heldItemsBox |= eo->fObjectSpaceBox( ).fTransform( eo->fParentRelative() );
		}

		fSetLocalSpaceMinMax( heldItemsBox.mMin, heldItemsBox.mMax );
	}

	void tEditableGroupEntity::fEmptyObjects( )
	{
		for( u32 i = 0; i < mHeldEntities.fCount(); ++i )
		{
			tEditableObject* eo = mHeldEntities[i]->fDynamicCast< tEditableObject >( );
			sigassert( eo );

			eo->fClearGroupParent( );
			eo->fReparent( fSceneGraph()->fRootEntity() );
		}
	}

	tEditorSelectionList tEditableGroupEntity::fGetObjects( ) const
	{
		tEditorSelectionList ret;

		const u32 numChild = fChildCount();
		for( u32 i = 0; i < numChild; ++i )
		{
			tEditableObject* eo = fChild( i )->fDynamicCast< tEditableObject >( );
			if( !eo )
				continue;

			ret.fAdd( tEntityPtr( eo ), false );
		}

		return ret;
	}
}

