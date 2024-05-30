#include "BasePch.hpp"
#include "tDynamicOctree.hpp"
#include "tRandom.hpp"
#include "Gfx/tDebugGeometry.hpp"

namespace Sig
{
	sig_static_assert( sizeof( tDynamicOctree::tObject ) == 32 );

	devvar_clamp( u32, SceneGraph_Octree_ObjectCountBeforeSplit, 0, 0, 1024, 0 );

	///
	/// \section tDynamicOctree
	///

	tDynamicOctree::tDynamicOctree( )
		: mParent( 0 )
		, mIndex( -1 )
	{
		fZeroOut( mChildren );
		mBounds.fInvalidate( );
	}

	tDynamicOctree::tDynamicOctree( const Math::tAabbf& bounds, tDynamicOctree* parent, s32 index )
		: mBounds( bounds )
		, mParent( parent )
		, mIndex( index )
	{
		fZeroOut( mChildren );
	}

	tDynamicOctree::~tDynamicOctree( )
	{
		fClearChildren( );
		if( mParent && mIndex >= 0 )
			mParent->mChildren[ mIndex ] = 0;
	}

	void tDynamicOctree::fClearChildren( )
	{
		for( u32 i = 0; i < mChildren.fCount( ); ++i )
		{
			delete mChildren[ i ];
			mChildren[ i ] = 0;
		}
	}

	void tDynamicOctree::fClearContents( )
	{
		mContents.fDeleteArray( );
	}

	void tDynamicOctree::fClearAll( )
	{
		fClearContents( );
		fClearChildren( );
		mBounds.fInvalidate( );
	}

	void tDynamicOctree::fClean( )
	{
		u32 numChildren = 0;
		for( u32 i = 0; i < mChildren.fCount( ); ++i )
		{
			if( mChildren[ i ] )
			{
				mChildren[ i ]->fClean( ); // this may result in mChildren[ i ] being null
				numChildren += mChildren[ i ] ? 1 : 0;
			}
		}

		if( mParent && numChildren == 0 && mContents.fCount( ) == 0 ) // we only delete ourselves if we have a parent, i.e., if we're not the root
			delete this;
	}

	b32 tDynamicOctree::fInsert( 
		const tObjectPtr& object, 
		u32 depth, 
		u32 maxDepth,
		tDynamicOctree* ignore )
	{
		sigassert( object->mCellKey.fNull( ) );
		sigassert( depth <= maxDepth );

		if( this == ignore )
			return false;

		if( depth > 0 && ( mBounds.fComputeDiagonal( ).fLengthSquared( ) < object->mWorldSpaceBox.fComputeDiagonal( ).fLengthSquared( ) ) )
			return false; // the object is too big to possibly fit in this cell

		const b32 containsInstance = mBounds.fContains( object->mWorldSpaceBox );

		const u32 objectCountBeforeSplit = SceneGraph_Octree_ObjectCountBeforeSplit; // don't split until we accumulate enough objects
		if( containsInstance && depth < maxDepth && mContents.fCount( ) >= objectCountBeforeSplit )
		{
			// attempt to add object to children; note that
			// we only attempt this if the current cell actually
			// contains the object (as otherwise the children
			// certainly won't), and if we haven't split too many times

			for( u32 i = 0; i < mChildren.fCount( ); ++i )
			{
				const b32 wasNull = !mChildren[ i ];
				if( wasNull )
					mChildren[ i ] = NEW tDynamicOctree( fComputeChildAabb( mBounds, i ), this, i );

				if( mChildren[ i ]->fInsert( object, depth + 1, maxDepth, ignore ) )
					return true; // one of the children took it

				if( wasNull )
				{
					// immediately prune bcz it wasn't taken
					delete mChildren[ i ];
					mChildren[ i ] = 0;
				}
			}
		}

		// no children were kind enough to take the object, so now we try
		if( depth == 0 || containsInstance )
		{
			// we have to accept it
			object->mCellKey = tCellKey( this, mContents.fCount( ) );
			mContents.fPushBack( object );
			return true;
		}

		// couldn't do it
		return false;
	}

	void tDynamicOctree::fRemove( u32 itemIndex )
	{
		mContents.fErase( itemIndex );
		if( itemIndex < mContents.fCount( ) )
			mContents[ itemIndex ]->mCellKey.mIndex = itemIndex; // adjust moved item's index
	}

	void tDynamicOctree::fUpdateBounds( const Math::tAabbf& aabb )
	{
		mBounds = aabb;
		for( u32 i = 0; i < mChildren.fCount( ); ++i )
		{
			if( mChildren[ i ] )
				mChildren[ i ]->fUpdateBounds( fComputeChildAabb( aabb, i ) );
		}
	}

	void tDynamicOctree::fRayCastCollect(
		u32 depth,
		const Math::tRayf& ray,
		tGrowableArray<const tDynamicOctree*>& cells ) const
	{
		b32 recurse = true;

		const b32 intersectsCell = mBounds.fIntersectsOrContains( ray );
		if( !intersectsCell )
		{
			if( depth == 0 )
				recurse = false; // for the root, we might actually contain objects that are outside of the cell bounds
			else
				return; // quick rejection, don't have to test contents or children
		}

		cells.fPushBack( this );

		if( recurse )
		{
			// if we're here, we know that the frustum intersects our volume, so attempt to recurse on children
			for( u32 i = 0; i < mChildren.fCount( ); ++i )
			{
				if( mChildren[ i ] )
					mChildren[ i ]->fRayCastCollect( depth + 1, ray, cells );
			}
		}
	}

	void tDynamicOctree::fRenderDebug( 
		u32 depth, 
		s32 targetDepth, 
		b32 objectBoxesOnly,
		Gfx::tDebugGeometryContainer& debugGeom ) const
	{
#ifdef sig_devmenu
		if( depth == targetDepth || targetDepth < 0 )
		{
			const Math::tVec4f rgba = tRandom( ( u32 )( size_t )this ).fColor( 0.25f );

			if( mContents.fCount( ) > 0 )
			{
				// render the contents of this cell

				if( !objectBoxesOnly )
					debugGeom.fRenderOnce( mBounds, rgba );

				for( tCellContents::tConstIterator i = mContents.fBegin( ), iend = mContents.fEnd( ); i != iend; ++i )
					debugGeom.fRenderOnce( (*i)->mWorldSpaceBox, rgba );
			}

			if( targetDepth >= 0 && !objectBoxesOnly )
			{
				// if requesting to render a specific depth, then we also draw the children at that depth

				for( u32 i = 0; i < mChildren.fCount( ); ++i )
				{
					if( mChildren[ i ] )
						debugGeom.fRenderOnce( mChildren[ i ]->mBounds, 0.5f * rgba );
				}
			}
		}

		if( depth != targetDepth )
		{
			// we only recurse if this isn't the target depth

			for( u32 i = 0; i < mChildren.fCount( ); ++i )
			{
				if( mChildren[ i ] )
					mChildren[ i ]->fRenderDebug( depth + 1, targetDepth, objectBoxesOnly, debugGeom );
			}
		}
#endif//sig_devmenu
	}

	tDynamicOctreeRoot::tDynamicOctreeRoot( )
		: mMaxDepth( 0 )
	{
	}

	tDynamicOctreeRoot::~tDynamicOctreeRoot( )
	{
		fClear( );
	}

	void tDynamicOctreeRoot::fCreate( const Math::tAabbf& rootBounds, u32 maxDepth )
	{
		fClear( );
		mMaxDepth = maxDepth;
		mRoot.fUpdateBounds( rootBounds );

		sigassert( mRoot.fBounds( ).fIsValid( ) );
		sigassert( mMaxDepth < 64 );
	}

	void tDynamicOctreeRoot::fClear( )
	{
		mRoot.fClearAll( );
		mMaxDepth = 0;
	}

	void tDynamicOctreeRoot::fClean( )
	{
		mRoot.fClean( );
	}

	void tDynamicOctreeRoot::fInsert( const tObjectPtr& object )
	{
		mRoot.fInsert( object, 0, mMaxDepth );
	}

	void tDynamicOctreeRoot::fRemove( const tObjectPtr& object )
	{
		object->fRemove( );
	}

	void tDynamicOctreeRoot::fMove( const tObjectPtr& object )
	{
		tDynamicOctree* ignoreOnInsert = object->fMove( mMaxDepth );
		if( ignoreOnInsert )
			mRoot.fInsert( object, 0, mMaxDepth, ignoreOnInsert );
	}

	void tDynamicOctreeRoot::fRenderDebug( 
		s32 targetDepth, 
		b32 objectBoxesOnly,
		Gfx::tDebugGeometryContainer& debugGeom ) const
	{
		mRoot.fRenderDebug( 0, targetDepth, objectBoxesOnly, debugGeom );
	}
}
