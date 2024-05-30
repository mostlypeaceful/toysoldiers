#include "BasePch.hpp"
#include "tSortedOverlapTree.hpp"
#include "tPhysicsWorld.hpp" //DEBUG RENDERING

#include "tGJK.hpp" //just for debugging

using namespace Sig::Math;

namespace Sig { namespace Physics
{

	devvar( bool, Physics_Sweep_TestAll, false );

#ifdef build_debug
	devvar( bool, Physics_Sweep_Debug, true );
#else
	devvar( bool, Physics_Sweep_Debug, false );
#endif

	// Anything outside of this will still work, there are "abyss" cells reserved for being outside this range. 
	//  No worries if this is too small.
	devvar( f32, Physics_Sweep_WorldExtents, 1000 );

	namespace
	{

#ifdef cSOT_USE_INTS
		// This will convert a float to a sortable u32 without destroying any precision
		inline tSortedOverlapTree::tCoord fEncodeFloat( f32 newPos )
		{
			//we may need to check on -0 and 0
			//But it should make no practical difference.
			u32 ir = *reinterpret_cast<u32*>( &newPos );
			if( ir & 0x80000000 ) //negative?
				ir = ~ir;//reverse sequence of negative numbers
			else
				ir |= 0x80000000; // flip sign

			ir += 0x0000FFFF;
			return ir;
		}
		inline f32 fDecodeFloat( tSortedOverlapTree::tCoord coord )
		{
			//we may need to check on -0 and 0
			//But it should make no practical difference.
			coord -= 0x0000FFFF;

			if( coord & 0x80000000 ) //negative?
				coord &= ~0x80000000; // flip sign
			else
				coord = ~coord;//reverse sequence of negative numbers

			return *reinterpret_cast<f32*>( &coord );
		}
#else
		inline f32 fEncodeFloat( f32 newPos ) 
		{ 
			return newPos; 
		}
		inline f32 fDecodeFloat( tSortedOverlapTree::tCoord coord )
		{
			return coord;
		}
#endif

		static b32 fOverlaps( const tSortedOverlapTree::tItem& a, const tSortedOverlapTree::tItem& b )
		{
			return a.mBounds.fIntersects( b.mBounds );
		}

	}

	void tSortedOverlapTree::tCell::fSetBounds( f32 min, f32 max )
	{
		mMin = fEncodeFloat( min );
		mMax = fEncodeFloat( max );
	}

	void tSortedOverlapTree::tCell::fClear( )
	{
		mSortedCoordinates.fSetCount( 2 );
		mSortedCoordinates[ 0 ] = tItemCoord( NULL, mMin, false );
		mSortedCoordinates[ 1 ] = tItemCoord( NULL, mMax, true );
	}

	void tSortedOverlapTree::tCell::fValidateSort( )
	{
		for( u32 i = 1; i < mSortedCoordinates.fCount( ); ++i )
			sigassert( mSortedCoordinates[ i-1 ].mPos <= mSortedCoordinates[ i ].mPos );
	}

	void tSortedOverlapTree::tSortedAxis::fClear( )
	{
		for( u32 i = 0; i < mCells.fCount( ); ++i )
			mCells[ i ].fClear( );
	}

	void tSortedOverlapTree::tSortedAxis::fValidateSort( )
	{
		for( u32 i = 0; i < mCells.fCount( ); ++i )
			mCells[ i ].fValidateSort( );
	}

	tSortedOverlapTree::tSortedOverlapTree( )
	{
		fConfigureCells( );
		fClear( );
	}

	void tSortedOverlapTree::fConfigureCells( )
	{
		f32 startPt = -Physics_Sweep_WorldExtents;
		f32 rate = 2 * Physics_Sweep_WorldExtents / cCellSubDivision;

		for( u32 a = 0; a < cAxisCount; ++a )
		{
			tFixedArray< tCell, cCellSubDivision >& cells = mSortedAxes[ a ].mCells;

			// The edge cells need to extend to infinity
			f32 min = -cInfinity;

			for( u32 c = 0; c < cCellSubDivision; ++c )
			{
				f32 max = (c==0) ? startPt : ((c==cCellSubDivision-1) ? cInfinity : min + rate);

				cells[ c ].fSetBounds( min, max );

				min = max; //these intervals need to be as contiguous as possible
			}
		}
	}

	void tSortedOverlapTree::fAddItem( tItem& item, u32 flags, Math::tAabbf& bounds )
	{
		tBatch batch;
		batch.fAddItem( item, flags, bounds );
		fAddBatch( batch );
	}

	void tSortedOverlapTree::fRemoveItem( tItem& item )
	{
		profile( cProfilePerfPhysicsCollisionMaintain );
		sigassert( mItems.fFind( &item ) && "Item not found in the list!" );

		fSortedErase( item, cXEnd );
		fSortedErase( item, cYEnd );
		fSortedErase( item, cZEnd );
		fSortedErase( item, cXStart );
		fSortedErase( item, cYStart );
		fSortedErase( item, cZStart );

		mPairManager.fRemoveItem( item );
		mItems.fFindAndErase( &item );

		fValidateSort( );
	}

	void tSortedOverlapTree::fUpdateItem( tItem& item, Math::tAabbf& bounds )
	{
		profile( cProfilePerfPhysicsCollisionMaintain );
		
		sigcheckfail( bounds.fIsValid( ), return );
		sigassert( mItems.fFind( &item ) && "Item not found in the list!" );

		item.mBounds = bounds;

		tGrowableArray< tEvent > events;

		fSortedUpdate( item, fEncodeFloat( bounds.mMin.x ), cXStart, events );
		fSortedUpdate( item, fEncodeFloat( bounds.mMin.y ), cYStart, events );
		fSortedUpdate( item, fEncodeFloat( bounds.mMin.z ), cZStart, events );
		fSortedUpdate( item, fEncodeFloat( bounds.mMax.x ), cXEnd, events );
		fSortedUpdate( item, fEncodeFloat( bounds.mMax.y ), cYEnd, events );
		fSortedUpdate( item, fEncodeFloat( bounds.mMax.z ), cZEnd, events );

		for( u32 i = 0; i < events.fCount( ); ++i )
		{
			const tEvent& e = events[ i ];
			mPairManager.fAddPotentialPair( *e.mA, *e.mB, true );
		}

		fValidateSort( );
	}

	void tSortedOverlapTree::fSetSleeping( tItem& item, b32 sleep )
	{
		item.mFlags = fSetClearBits( item.mFlags, cItemFlagSleeping, sleep );
	}

	void tSortedOverlapTree::fClearPairData( tItem& item )
	{
		profile( cProfilePerfPhysicsCollisionMaintain );
		sigassert( mItems.fFind( &item ) && "Item not found in the list!" );
		mPairManager.fClearPairData( item );
	}

	void tSortedOverlapTree::fClear( )
	{
		mItems.fSetCount( 0 );

		for( u32 i = 0; i < mSortedAxes.fCount( ); ++i )
			mSortedAxes[ i ].fClear( );

		mPairManager.fClear( );
		mIDManager.fReset( );
	}

	void tSortedOverlapTree::fPurge( )
	{
		if( Physics_Sweep_TestAll )
		{
			// hax O(n^2) testing
			mPairManager.fClear( );
			for( u32 i = 0; i < mItems.fCount( ); ++i )
			{
				for( u32 j = i + 1; j < mItems.fCount( ); ++j )
				{
					if( mItems[ i ]->fDynamic( ) || mItems[ j ]->fDynamic( ) )
					{
						mPairManager.fAddPotentialPair( *mItems[ i ], *mItems[ j ], false );
					}
				}
			}

			mPairManager.fBakePairList( );
			return;
		}

		mPairManager.fPurge( );
	}



	namespace
	{
		template< typename t >
		void fShiftArray( u32 startIndex, s32 shift, t& data )
		{
			u32 copyLength = data.fCount( ) - startIndex;

			if( shift > 0 )
				data.fSetCount( data.fCount( ) + shift );

			tCopier<t::tValue>::fCopyOverlapped( data.fBegin( ) + startIndex + shift, data.fBegin( ) + startIndex, copyLength );

			if( shift < 0 )
				data.fSetCount( data.fCount( ) + shift );
		}
	}

	void tSortedOverlapTree::tBatch::fAddItem( tItem& item, u32 flags, Math::tAabbf& bounds )
	{
		profile( cProfilePerfPhysicsCollisionMaintain );
		sigassert( !mItems.fFind( &item ) && "Item already in the batch!" );

		item.mFlags = flags;
		// Do on batch add. item.mID = mIDManager.fGetID( );
		item.mBounds = bounds;

		mItems.fPushBack( &item );
	}

	void tSortedOverlapTree::fFindInsertionPoints( u32 axis, const tGrowableArray<tItemCoord>& input, tGrowableArray<tInsertPt>& out ) const
	{
		const tFixedArray< tCell, cCellSubDivision >& cells = mSortedAxes[ axis ].mCells;

		s32 cell = cCellSubDivision - 1;

		s32 currentPosition = -1;

		// Example data, first insert.
		// Before: [cLower, cUpper]
		// After: [cLower, newVal, cUpper ]; First iteration should == 1

		for( s32 i = input.fCount( ) - 1; i >= 0 ; --i )
		{
			// Stop when the next item is <= than ours.
			const tCoord pos = input[ i ].mPos;

			// Find cell
			while( cell > 0 )
			{
				if( cells[ cell ].mMin <= pos )
					break;
				--cell;
				currentPosition = -1;
			}

			const tGrowableArray<tItemCoord>& coordList = cells[ cell ].mSortedCoordinates;
			if( currentPosition == -1 )
				currentPosition = coordList.fCount( ) - 1;

			//find index
			while( currentPosition > 0 && coordList[ currentPosition - 1 ].mPos > pos )
			{
				--currentPosition;
			}
		
			out[ i ] = tInsertPt( cell, currentPosition );
		}
	}

	void tSortedOverlapTree::fAddBatch( const tBatch& batch )
	{
		profile( cProfilePerfPhysicsCollisionMaintain );

		const u32 itemCount = batch.mItems.fCount( );
		const u32 coordCount = itemCount * 2;

		sigassert( itemCount );

		tGrowableArray< tInsertPt > inserts;
		inserts.fSetCount( coordCount );

		tGrowableArray< tItemCoord > coords;
		coords.fSetCount( coordCount );

		for( u32 a = 0; a < cAxisCount; ++a )
		{
			// Bake in sorted coordinates.
			for( u32 i = 0; i < itemCount; ++i )
			{
				tItem* item = batch.mItems[ i ];
				const Math::tAabbf& bounds = item->mBounds;
				switch( a )
				{	
				case cX:
					{
						coords[ i * 2 + 0 ] = tItemCoord( item, fEncodeFloat( bounds.mMin.x ), false );
						coords[ i * 2 + 1 ] = tItemCoord( item, fEncodeFloat( bounds.mMax.x ), true );
						break;
					}
				case cY:
					{
						coords[ i * 2 + 0 ] = tItemCoord( item, fEncodeFloat( bounds.mMin.y ), false );
						coords[ i * 2 + 1 ] = tItemCoord( item, fEncodeFloat( bounds.mMax.y ), true );
						break;
					}
				case cZ:
					{
						coords[ i * 2 + 0 ] = tItemCoord( item, fEncodeFloat( bounds.mMin.z ), false );
						coords[ i * 2 + 1 ] = tItemCoord( item, fEncodeFloat( bounds.mMax.z ), true );
						break;
					}
				}
			}

			// Sort them locally
			std::sort( coords.fBegin( ), coords.fEnd( ) );

			// Decide where they go
			fFindInsertionPoints( a, coords, inserts );

			u32 lastCell = ~0;
			u32 insertsAfterThis = 0; //how many more inserts are remaining for this cell

			// Now begin inserting
			s32 insertCntMin1 = coordCount - 1;
			for( s32 i = insertCntMin1; i >= 0; --i )
			{
				u32 newCell = inserts[ i ].mCell;
				tCell& cell = mSortedAxes[ a ].mCells[ newCell ];
				tGrowableArray<tItemCoord>& coordList = cell.mSortedCoordinates;

				u32 originalInsert = inserts[ i ].mIndex;
				b32 firstInsertThisCell = false;

				if( newCell != lastCell )
				{
					lastCell = newCell;
					firstInsertThisCell = true;
					//Resize this cell

					// count inserts for this cell
					insertsAfterThis = 0;
					for( s32 t = i - 1; t >= 0; --t )
						if( inserts[ t ].mCell == newCell )
							++insertsAfterThis;
						else
							break;

					// Move the back end of the array back.
					u32 endPartOfArray = originalInsert;
					u32 shift = insertsAfterThis + 1;
					fShiftArray( endPartOfArray, shift, coordList );

					// Update those guys
					u32 count = coordList.fCount( ) - 1; //dont count sentinel
					for( u32 c = endPartOfArray + shift; c < count; ++c )
						coordList[ c ].fCoordinateIndex( a ) += shift;
				}

				// Move stuff out of the way first, wont need to move stuff for the last insert
				if( !firstInsertThisCell )
				{
					// Other original is the original index of the slot we just previously inserted too
					u32 otherOriginal = inserts[ i + 1 ].mIndex;

					// Compute how many coords are inbetween by subtracting the current original insertion point.
					u32 length = otherOriginal - originalInsert;

					if( length )
					{
						u32 shift = insertsAfterThis + 1;
						u32 moveTarget = originalInsert + shift;
						tCopier<tItemCoord>::fCopyOverlapped( coordList.fBegin( ) + moveTarget, coordList.fBegin( ) + originalInsert, length );
						for( u32 c = 0; c < length; ++c )
							coordList[ moveTarget + c ].fCoordinateIndex( a ) += shift;
					}
				}
				
				// Actually insert the element.
				u32 actualInsertPt = originalInsert + insertsAfterThis;
				coordList[ actualInsertPt ] = coords[ i ];
				coordList[ actualInsertPt ].fCoordinateIndex( a ) = actualInsertPt;
				coordList[ actualInsertPt ].fCoordinateCell( a ) = &cell;

				--insertsAfterThis;
			}

			fValidateSort( );
		}

		// add the items.
		mItems.fReserve( mItems.fCount( ) + itemCount );
		for( u32 b = 0; b < batch.mItems.fCount( ); ++b )
		{
			tItem* item = batch.mItems[ b ];
			sigassert( !mItems.fFind( item ) && "Item already in the tree list!" );

			item->mID = mIDManager.fGetID( );

			// super hack!
			// could potentially be intersecting anything
			for( u32 i = 0; i < mItems.fCount( ); ++i )
			{
				if( mItems[ i ]->mParent != item->mParent && fOverlaps( *mItems[ i ], *item ) )
					mPairManager.fAddPotentialPair( *item, *mItems[ i ], false );
			}

			mItems.fPushBack( tItemPtr( item ) );
		}

		fValidateSort( );
	}

	void tSortedOverlapTree::fSortedErase( tItem& item, tCoordinateLists list )
	{
		u32 axis = fCoordListAxis( list );
		tGrowableArray<tItemCoord>& coordList = item.mCoordinateCells[ list ]->mSortedCoordinates;

		u32 index = item.mCoordinateIndexs[ list ];
		coordList.fEraseOrdered( index );

		u32 count = coordList.fCount( ) - 1; //dont count sentinel
		for( u32 i = index; i < count; ++i )
		{
			tItemCoord& update = coordList[ i ];
			sigassert( update.fCoordinateIndex( axis ) > 0 );
			--update.fCoordinateIndex( axis );
		}
	}

	void tSortedOverlapTree::fAddEvent( tItem* a, tItem* b, tGrowableArray< tEvent >& events ) const
	{
		if( a != b && a->mParent != b->mParent )
		{
			tPairKeyType key = fMakeKey( *a, *b );

			if( !events.fFind( key ) )
				events.fPushBack( tEvent( a, b, key ) );
		}
	}

	void tSortedOverlapTree::fSwapCoords( u32 a, u32 b, tGrowableArray<tItemCoord>& listData, u32 axis, tGrowableArray< tEvent >& events )
	{
		tItemCoord& updateA = listData[ a ];
		updateA.fCoordinateIndex( axis ) = b;

		tItemCoord& updateB = listData[ b ];
		updateB.fCoordinateIndex( axis ) = a;

		// A pair only changes overlap status when a Min has crossed a Max
		//  We dont care when a min crosses a min, or a max crosses a max.
		if( updateA.mIsMax ^ updateB.mIsMax )
			fAddEvent( updateA.mItem, updateB.mItem, events );

		fSwap( updateA, updateB );
	}

	// This single coordinate insert variant is only for switching cells
	void tSortedOverlapTree::fSortedInsert( tItem& item, tCoord value, b32 searchUp, tCoordinateLists list, u32 cellIndex, tGrowableArray< tEvent >& events )
	{
		u32 axis = fCoordListAxis( list );
		tCell& cell = mSortedAxes[ axis ].mCells[ cellIndex ];

#ifdef build_debug
		sigassert( value != cell.mMin );
		sigassert( value != cell.mMax );
#endif

		tGrowableArray<tItemCoord>& coordList = cell.mSortedCoordinates;

		s32 index = 0;

		if( searchUp )
		{
			// 1 to skip sentinel, we never insert at front sentinel
			for( index = 1; index < (s32)coordList.fCount( ); ++index )
			{
				if( value < coordList[ index ].mPos )
					break;

				// Generate events for these passed coordinates
				fAddEvent( &item, coordList[ index ].mItem, events );
			}
		}
		else
		{
			// -1 for array, inserting at end sentinel is valid
			index = coordList.fCount( ) - 1;
			for( ; index > 0; --index )
			{
				if( value > coordList[ index - 1 ].mPos )
					break;

				// Generate events for these passed coordinates
				fAddEvent( &item, coordList[ index - 1 ].mItem, events );
			}
		}

		b32 isMax = (fCoordListType( list ) == cEnd);
		coordList.fInsert( index, tItemCoord( &item, value, isMax ) );
		item.mCoordinateIndexs[ list ] = index;
		item.mCoordinateCells[ list ] = &cell;

		u32 count = coordList.fCount( ) - 1; //dont count sentinel
		for( u32 i = index + 1; i < count; ++i )
		{
			tItemCoord& update = coordList[ i ];
			++update.fCoordinateIndex( axis );
		}
	}

	void tSortedOverlapTree::fChangeCells( u32 cellIndex, b32 searchUp, tItem& item, tCoordinateLists list, tGrowableArray< tEvent >& events )
	{
		u32 axis = fCoordListAxis( list );
		tCoord value = item.mCoordinateCells[ list ]->mSortedCoordinates[ item.mCoordinateIndexs[ list ] ].mPos;
		fSortedErase( item, list );

		const tFixedArray< tCell, cCellSubDivision >& cells = mSortedAxes[ axis ].mCells;

		if( searchUp )
		{
			++cellIndex;

			while( value >= cells[ cellIndex ].mMax )
			{
				// unfortunately we just passed a whole cell in one move!
				//  we need to add all the items in the cell as potential events.
				const tGrowableArray<tItemCoord>& coordList = cells[ cellIndex ].mSortedCoordinates;

				// Skipping sentinels since their items will be null.
				for( u32 i = 1; i < coordList.fCount( ) - 1; ++i )
					fAddEvent( &item, coordList[ i ].mItem, events );

				++cellIndex;
			}
		}
		else
		{
			--cellIndex;

			while( value < cells[ cellIndex ].mMin )
			{
				// unfortunately we just passed a whole cell in one move!
				//  we need to add all the items in the cell as potential events.
				const tGrowableArray<tItemCoord>& coordList = cells[ cellIndex ].mSortedCoordinates;

				// Skipping sentinels since their items will be null.
				for( u32 i = 1; i < coordList.fCount( ) - 1; ++i )
					fAddEvent( &item, coordList[ i ].mItem, events );

				--cellIndex;
			}

		}

		fSortedInsert( item, value, searchUp, list, cellIndex, events );
		fValidateSort( );
	}

	void tSortedOverlapTree::fSortedUpdate( tItem& item, tCoord value, tCoordinateLists list, tGrowableArray< tEvent >& events )
	{
#ifdef build_debug
		sigassert( value != item.mCoordinateCells[ list ]->mMin );
		sigassert( value != item.mCoordinateCells[ list ]->mMax );
#endif

		u32 axis = fCoordListAxis( list );

		u32 currentCell = fPtrDiff( item.mCoordinateCells[ list ], mSortedAxes[ axis ].mCells.fBegin( ) );
		tGrowableArray<tItemCoord>& coordList = mSortedAxes[ axis ].mCells[ currentCell ].mSortedCoordinates;

		u32 oldIndex = item.mCoordinateIndexs[ list ];
		tCoord oldValue = coordList[ oldIndex ].mPos;

		if( value == oldValue )
			return; //nothing to do

		coordList[ oldIndex ].mPos = value;

		b32 moveUp = value > oldValue;
		if( moveUp )
		{
			// search up
			u32 testIndex = oldIndex + 1;
			while( coordList[ testIndex ].mPos <= value )
			{
				if( testIndex == coordList.fCount( ) - 1 )
				{
					// we reached the end of this cell.
					sigassert( currentCell < cCellSubDivision - 1 && "no higher to go, something bad happened we past the sentient?" );
					
					fChangeCells( currentCell, true, item, list, events );
					return;
				}

				fSwapCoords( oldIndex, testIndex, coordList, axis, events );
				oldIndex = testIndex;
				++testIndex;
			}
		}
		else
		{
			// search down
			u32 testIndex = oldIndex - 1;
			while( coordList[ testIndex ].mPos > value )
			{
				if( testIndex == 0 )
				{
					// we reached the end of this cell.
					sigassert( currentCell > 0 && "no lower to go, something bad happened we past the sentient?" );

					fChangeCells( currentCell, false, item, list, events );
					return;
				}

				fSwapCoords( oldIndex, testIndex, coordList, axis, events );
				oldIndex = testIndex;
				--testIndex;
			}
		}
	}

	void tSortedOverlapTree::fRenderAABBs( )
	{
		for( u32 i = 0; i < mItems.fCount( ); ++i )
		{
			tItemPtr& item = mItems[ i ];

			tVec3f min( fDecodeFloat( item->mCoordinateCells[ cXStart ]->mSortedCoordinates[ item->mCoordinateIndexs[ cXStart ] ].mPos ), fDecodeFloat( item->mCoordinateCells[ cYStart ]->mSortedCoordinates[ item->mCoordinateIndexs[ cYStart ] ].mPos ), fDecodeFloat( item->mCoordinateCells[ cZStart ]->mSortedCoordinates [ item->mCoordinateIndexs[ cZStart ]].mPos ) );
			tVec3f max( fDecodeFloat( item->mCoordinateCells[ cXEnd ]->mSortedCoordinates[ item->mCoordinateIndexs[ cXEnd ] ].mPos ), fDecodeFloat( item->mCoordinateCells[ cYEnd ]->mSortedCoordinates[ item->mCoordinateIndexs[ cYEnd ] ].mPos ), fDecodeFloat( item->mCoordinateCells[ cZEnd ]->mSortedCoordinates[ item->mCoordinateIndexs[ cZEnd ] ].mPos ) );
			tAabbf bounds( min, max );
			bounds.fInflate( 0.1f );

			tVec4f color = item->fDynamic( ) ? tVec4f( 0,1,0, 0.25f ) : tVec4f( 1,0,0, 0.25f );
			tPhysicsWorld::fDebugGeometry( ).fRenderOnce( bounds, color );
		}

		// draw lines representing the cell grid

		for( u32 a = 0; a < mSortedAxes.fCount( ); ++a )
		{
			// for cross hatching
			u32 crossDir = (a==1) ? 0 : 1; //up if a is not up.
			tVec3f crossVec = tVec3f::cZeroVector;
			crossVec[ crossDir ] = 0.2f;

			for( u32 c = 0; c < mSortedAxes[ a ].mCells.fCount( ); ++c )
			{
				tCell& cell = mSortedAxes[ a ].mCells[ c ];
				tVec4f color = tVec4f::cZeroVector;
				color.w = 1.0f;
				color[ a ] = 1.0f;

				tVec3f p1 = tVec3f::cZeroVector;
				tVec3f p2 = tVec3f::cZeroVector;
				p1[ a ] = fDecodeFloat( cell.mMin );
				p2[ a ] = fDecodeFloat( cell.mMax );

				tPhysicsWorld::fDebugGeometry( ).fRenderOnce( p1, p2, color );
				tPhysicsWorld::fDebugGeometry( ).fRenderOnce( p2 + crossVec, p2 - crossVec, tVec4f::cOnesVector );
			}
		}
	}

	void tSortedOverlapTree::fValidateSort( )
	{
		if( Physics_Sweep_Debug )
		{
			for( u32 c = 0; c < mSortedAxes.fCount( ); ++c )
				mSortedAxes[ c ].fValidateSort( );
		}
	}

	tSortedOverlapTree::tPairKeyType tSortedOverlapTree::fMakeKey( tItem& a, tItem& b )
	{
		sigassert( &a != &b );
		sigassert( a.mID != b.mID );

		if( a.mID < b.mID )
			return (u32(a.mID) << 16) | b.mID;
		else
			return (u32(b.mID) << 16) | a.mID;
	}	

	b32 tSortedOverlapTree::fKeyContains( tPairKeyType key, tIDType id )
	{
		return( (key & 0xFFFF) == id || (key >> 16) == id );
	}

	void tSortedOverlapTree::tPairManager::fAddPotentialPair( tItem& a, tItem& b, b32 dirty )
	{
		if( a.fDynamic( ) || b.fDynamic( ) )
		{
			const tPairKeyType key = fMakeKey( a, b );

			tPair* pair = mTable.fFind( key );
			if( pair )
				pair->mDirty = dirty;
			else
				mTable.fPushBack( tPair( &a, &b, dirty ) );
		}
	}	

	void tSortedOverlapTree::tPairManager::fPurge( )
	{
		for( s32 i = mTable.fCount( ) - 1; i >= 0; --i )
		{
			tPair& pair = mTable[ i ];
			if( pair.mDirty )
			{
				pair.mDirty = false;
				if( !fOverlaps( *pair.mA, *pair.mB ) )
					mTable.fErase( i );
			}
		}

		fBakePairList( );
	}

	void tSortedOverlapTree::tPairManager::fBakePairList( )
	{
		mPairs.fSetCount( 0 );

		for( u32 i = 0; i < mTable.fCount( ); ++i )
		{
			tPair& pair = mTable[ i ];
			if( pair.fDoTest( ) )
				mPairs.fPushBack( &pair );
		}
	}
	
	void tSortedOverlapTree::tPairManager::fRemoveItem( tItem& item )
	{
		for( s32 i = mTable.fCount( ) - 1; i >= 0; --i )
		{
			tPair& pair = mTable[ i ];
			if( pair.fContains( item ) )
				mTable.fErase( i );
		}
	}

	void tSortedOverlapTree::tPairManager::fClearPairData( tItem& item )
	{
		for( u32 i = 0; i < mTable.fCount( ); ++i )
		{
			tPair& pair = mTable[ i ];
			if( pair.fContains( item ) )
				pair.mData.fRelease( );
		}
	}
	

	void tSortedOverlapTree::tPairManager::fClear( )
	{
		mTable.fClear( );
		mPairs.fSetCount( 0 );
	}




	// TESTING
	namespace
	{
		void fCallbackAllCollisions( tSortedOverlapTree& tree )
		{
			for( u32 i = 0; i < tree.fPairs( ).fCount( ); ++i )
			{
				const tSortedOverlapTree::tPair& pair = *tree.fPairs( )[ i ];
				log_line( 0, "Overlap: A: " << (int)pair.mA << " B: " << (int)pair.mB );
			}
		}
	}


	void tSortedOverlapTree::fTest( )
	{
		//tGJK::fTest( );

		tSortedOverlapTree tree;

		tSortedOverlapTree::tItemPtr i1( NEW tSortedOverlapTree::tItem( (void*)1, (void*)1 ) );
		tSortedOverlapTree::tItemPtr i2( NEW tSortedOverlapTree::tItem( (void*)2, (void*)2 ) );
		tSortedOverlapTree::tItemPtr i3( NEW tSortedOverlapTree::tItem( (void*)3, (void*)3 ) );
		tSortedOverlapTree::tItemPtr i4( NEW tSortedOverlapTree::tItem( (void*)4, (void*)4 ) );

		tAabbf b1( tVec3f( 0 ), tVec3f( 0.66f ) );
		tAabbf b2( tVec3f( 0.33f ), tVec3f( 1.0f ) );
		tAabbf b3( tVec3f( 0.25f ), tVec3f( 0.75f ) );

		// These wee box won't collide, set the second number here to be greater than b2's min to see it collid
		tAabbf b4( tVec3f( 0.0f ), tVec3f( 0.2f ) ); //collides with nothing
		//tAabbf b4( tVec3f( 0.0f ), tVec3f( 0.3f ) ); //collides with 3
		//tAabbf b4( tVec3f( 0.0f ), tVec3f( 0.4f ) ); //collides with 2 and 3
		b4.mMin.x = 0.7f;
		b4.mMax.x = 0.99f;

		log_line( 0, "Test 1:" );

		tree.fAddItem( *i1, 0, b1 );
		tree.fAddItem( *i2, 0, b2 );
		tree.fAddItem( *i3, 0, b3 );
		tree.fAddItem( *i4, 0, b4 );

		fCallbackAllCollisions( tree );

		log_line( 0, "Test 2:" );

		b3 = tAabbf( tVec3f( 0.0f ), tVec3f( 0.25f ) );
		tree.fUpdateItem( *i3, b3 );

		fCallbackAllCollisions( tree );

		log_line( 0, "Test 3:" );

		b3 = tAabbf( tVec3f( 0.25f ), tVec3f( 0.75f ) );
		tree.fUpdateItem( *i3, b3 );

		fCallbackAllCollisions( tree );

		log_line( 0, "Test 4:" );

		b3 = tAabbf( tVec3f( 0.75f ), tVec3f( 0.95f ) );
		tree.fUpdateItem( *i3, b3 );

		fCallbackAllCollisions( tree );
	}
	
}}
