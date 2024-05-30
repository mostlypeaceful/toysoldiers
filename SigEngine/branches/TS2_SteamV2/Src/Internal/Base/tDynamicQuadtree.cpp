#include "BasePch.hpp"
#include "tDynamicQuadtree.hpp"
#include "tRandom.hpp"
#include "Gfx/tDebugGeometry.hpp"

namespace Sig
{
	sig_static_assert( sizeof( tDynamicQuadtree::tObject ) == 32 );

	devvar_clamp( u32, SceneGraph_Quadtree_ObjectCountBeforeSplit, 0, 0, 1024, 0 );

	///
	/// \section tDynamicQuadtree
	///

	tDynamicQuadtree::tDynamicQuadtree( )
		: mParent( 0 )
		, mIndex( -1 )
	{
		fZeroOut( mChildren );
		mBounds.fInvalidate( );
	}

	tDynamicQuadtree::tDynamicQuadtree( const Math::tAabbf& bounds, tDynamicQuadtree* parent, s32 index )
		: mBounds( bounds )
		, mParent( parent )
		, mIndex( index )
	{
		fZeroOut( mChildren );
	}

	tDynamicQuadtree::~tDynamicQuadtree( )
	{
		fClearChildren( );
		if( mParent && mIndex >= 0 )
			mParent->mChildren[ mIndex ] = 0;
	}

	void tDynamicQuadtree::fClearChildren( )
	{
		for( u32 i = 0; i < mChildren.fCount( ); ++i )
		{
			delete mChildren[ i ];
			mChildren[ i ] = 0;
		}
	}

	void tDynamicQuadtree::fClearContents( )
	{
		mContents.fDeleteArray( );
	}

	void tDynamicQuadtree::fClearAll( )
	{
		fClearContents( );
		fClearChildren( );
		mBounds.fInvalidate( );
	}

	void tDynamicQuadtree::fClean( )
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

	b32 tDynamicQuadtree::fInsert( 
		const tObjectPtr& object, 
		u32 depth, 
		u32 maxDepth,
		tDynamicQuadtree* ignore )
	{
		sigassert( object->mCellKey.fNull( ) );
		sigassert( depth <= maxDepth );

		if( this == ignore )
			return false;

		if( depth > 0 && ( mBounds.fComputeDiagonal( ).fLengthSquared( ) < object->mWorldSpaceBox.fComputeDiagonal( ).fLengthSquared( ) ) )
			return false; // the object is too big to possibly fit in this cell

		const b32 containsInstance = mBounds.fContains( object->mWorldSpaceBox );

		const u32 objectCountBeforeSplit = SceneGraph_Quadtree_ObjectCountBeforeSplit; // don't split until we accumulate enough objects
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
					mChildren[ i ] = NEW tDynamicQuadtree( fComputeChildAabb( mBounds, i ), this, i );

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
			sig_assertvecvalid( object->mWorldSpaceBox.mMin );
			sig_assertvecvalid( object->mWorldSpaceBox.mMax );
			object->mCellKey = tCellKey( this, mContents.fCount( ) );
			mContents.fPushBack( object );
			return true;
		}

		// couldn't do it
		return false;
	}

	void tDynamicQuadtree::fRemove( u32 itemIndex )
	{
		mContents.fErase( itemIndex );
		if( itemIndex < mContents.fCount( ) )
			mContents[ itemIndex ]->mCellKey.mIndex = itemIndex; // adjust moved item's index
	}

	void tDynamicQuadtree::fUpdateBounds( const Math::tAabbf& aabb )
	{
		mBounds = aabb;
		for( u32 i = 0; i < mChildren.fCount( ); ++i )
		{
			if( mChildren[ i ] )
				mChildren[ i ]->fUpdateBounds( fComputeChildAabb( aabb, i ) );
		}
	}

	void tDynamicQuadtree::fRenderDebug( 
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

	tDynamicQuadtreeRoot::tDynamicQuadtreeRoot( )
		: mMaxDepth( 0 )
	{
	}

	tDynamicQuadtreeRoot::~tDynamicQuadtreeRoot( )
	{
		fClear( );
	}

	void tDynamicQuadtreeRoot::fCreate( const Math::tAabbf& rootBounds, u32 maxDepth )
	{
		fClear( );
		mMaxDepth = maxDepth;
		mRoot.fUpdateBounds( rootBounds );

		sigassert( mRoot.fBounds( ).fIsValid( ) );
		sigassert( mMaxDepth < 64 );
	}

	void tDynamicQuadtreeRoot::fClear( )
	{
		mRoot.fClearAll( );
		mMaxDepth = 0;
	}

	void tDynamicQuadtreeRoot::fClean( )
	{
		mRoot.fClean( );
	}

	void tDynamicQuadtreeRoot::fInsert( const tObjectPtr& object )
	{
		mRoot.fInsert( object, 0, mMaxDepth );
	}

	void tDynamicQuadtreeRoot::fRemove( const tObjectPtr& object )
	{
		object->fRemove( );
	}

	void tDynamicQuadtreeRoot::fMove( const tObjectPtr& object )
	{
		tDynamicQuadtree* ignoreOnInsert = object->fMove( mMaxDepth );
		if( ignoreOnInsert )
			mRoot.fInsert( object, 0, mMaxDepth, ignoreOnInsert );
	}

	void tDynamicQuadtreeRoot::fRenderDebug( 
		s32 targetDepth, 
		b32 objectBoxesOnly,
		Gfx::tDebugGeometryContainer& debugGeom ) const
	{
		mRoot.fRenderDebug( 0, targetDepth, objectBoxesOnly, debugGeom );
	}
}
