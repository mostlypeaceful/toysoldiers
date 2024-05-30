#ifndef __tSortedOverlapTree__
#define __tSortedOverlapTree__

namespace Sig { namespace Physics
{

#define cSOT_USE_INTS 1

	class tSortedOverlapTree
	{
		debug_watch( tSortedOverlapTree );
	private:
		struct tCell;
	public:
		tSortedOverlapTree( );

#ifdef cSOT_USE_INTS
		typedef u32 tCoord;
#else
		typedef f32 tCoord;
#endif
		enum tCoordinateLists
		{
			cXStart,
			cYStart,
			cZStart,
			cXEnd,
			cYEnd,
			cZEnd,
			cCoordinateListCount
		};

		enum tCoordinateAxes
		{
			cX,
			cY,
			cZ,
			cAxisCount,
			cAllAxesMask = (1<<cX)|(1<<cY)|(1<<cZ)
		};

		static u32 fCoordListAxis( tCoordinateLists list )
		{
			return list % 3;
		}

		enum tType
		{
			cStart = 0,
			cEnd = 1,
		};

		static u32 fCoordListType( tCoordinateLists list )
		{
			return list / 3;
		}

		//static u32 fCoordinateList( u32 axis, u32 type )
		//{
		//	return type * 3 + axis;
		//}


		enum tItemFlags
		{
			cItemFlagStatic = (1<<0),
			cItemFlagSleeping = (1<<1)
		};

		typedef u16 tIDType;
		typedef u32 tPairKeyType; //this must be able to fit two tIDTypes in it

		struct tItem : tRefCounter
		{
			debug_watch( tItem );
			declare_uncopyable( tItem );
		public:
			void*			mUserData;
			void*			mParent; //wont create pairs between entities with the same parent
			u32				mFlags;
			tIDType			mID;
			Math::tAabbf	mBounds; // todo switch this to quantinized bounds
			tFixedArray<u32, cCoordinateListCount> mCoordinateIndexs; //indexes into sorted coordinate lists
			tFixedArray<tCell*, cCoordinateListCount> mCoordinateCells; //which cells the indexes are refering to.
			
			tItem( void* userData = NULL, void* parent = NULL )
				: mUserData( userData )
				, mParent( parent )
				, mFlags( 0 )
				, mID( ~0 )
			{ }

			b32 fDynamic( ) const { return !mFlags; }
		};

	public:
		typedef tRefCounterPtr< tItem > tItemPtr;

		static tPairKeyType fMakeKey( tItem& a, tItem& b );
		static b32 fKeyContains( tPairKeyType key, tIDType id );

		struct tPairData : public tRefCounter
		{
			declare_uncopyable( tPairData );
		public:
			tPairData( ) { }
			virtual void fCleanUp( tItem& item ) { }
			virtual ~tPairData( ) { }
		};

		typedef tRefCounterPtr< tPairData > tPairDataPtr;

		struct tPair
		{
			debug_watch( tPair );
			tItem* mA;
			tItem* mB;
			b32 mDirty;

			// This pointer will be persist as long as the pair overlaps.
			// No duplicate or opposite pair will be created.
			tPairDataPtr mData;

			tPair( tItem* a = NULL, tItem* b = NULL, b32 dirty = true )
				: mA( a )
				, mB( b )
				, mDirty( dirty )
			{ }

			b32 operator==( tPairKeyType key ) const
			{
				sigcheckfail( mA, return false );
				sigcheckfail( mB, return false );
				return fMakeKey( *mA, *mB ) == key;
			}
			
			b32 fContains( const tItem& item ) const
			{
				sigcheckfail( mA, return false );
				sigcheckfail( mB, return false );
				return mA->mID == item.mID || mB->mID == item.mID;
			}

			b32 fDoTest( ) const
			{
				return (mA->fDynamic( ) || mB->fDynamic( ));
			}
		};

		// Batch sorting sorts the data to be inserted.
		// Using this, it only has to iterate through the data once when inserted, 
		//  since it knows all future data to be inserted will come after the current position.
		struct tBatch
		{
			tGrowableArray< tItem* > mItems;

			void fAddItem( tItem& item, u32 flags, Math::tAabbf& bounds );
		};

		// Items are expected to be heap allocated, they will be stored in smart pointers internally.
		void fAddBatch( const tBatch& batch ); // MUCH faster for multiple objects.
		void fAddItem( tItem& item, u32 flags, Math::tAabbf& bounds );
		void fRemoveItem( tItem& item );
		void fUpdateItem( tItem& item, Math::tAabbf& bounds );
		void fSetSleeping( tItem& item, b32 makeStatic );
		void fClearPairData( tItem& item );

		u32 fItemsCount( ) const { return mItems.fCount( ); }
		void fClear( );

		// If the tree changes in any way, you MUST call purge before using fPairs!
		void fPurge( );
		const tGrowableArray< tPair* >& fPairs( ) const { return mPairManager.fPairs( ); }

		// Debug stuff
		static void fTest( );
		void fRenderAABBs( );

	private:
		/// --- Sorted Data ---
		/// Any raw pointer risk is mitigated by the limited interface above. 
		/// the item will be preserved as long as it is in the tree.
		struct tEvent
		{
			tItem* mA;
			tItem* mB;
			tPairKeyType mKey;

			tEvent( ) { }
			tEvent( tItem* a, tItem* b, tPairKeyType key ) : mA( a ), mB( b ), mKey( key ) { }

			b32 operator == ( const tPairKeyType& right ) const { return mKey == right; }
		};

		struct tItemCoord
		{
			debug_watch( tItemCoord );
			tCoord	mPos;
			b32		mIsMax;
			tItem*  mItem;

			tItemCoord( )
				: mItem( NULL )
			{ }

			tItemCoord( tItem* ptr, tCoord pos, b32 isMax )
				: mItem( ptr ), mPos( pos ), mIsMax( isMax )
			{ }

			b32 operator < ( const tItemCoord& right ) const { return mPos < right.mPos; }

			u32& fCoordinateIndex( u32 axis ) const { return mItem->mCoordinateIndexs[ mIsMax * 3 + axis ]; }
			tCell*& fCoordinateCell( u32 axis ) const { return mItem->mCoordinateCells[ mIsMax * 3 + axis ]; }
		};

		struct tInsertPt
		{
			u32 mCell;
			u32 mIndex;

			tInsertPt( ) : mCell( ~0 ), mIndex( ~0 ) { }
			tInsertPt( u32 cell, u32 index )
				: mCell( cell )
				, mIndex( index )
			{ }
		};

		void fSortedErase( tItem& item, tCoordinateLists list );
		void fSortedUpdate( tItem& item, tCoord value, tCoordinateLists list, tGrowableArray< tEvent >& events );
		void fSwapCoords( u32 a, u32 b, tGrowableArray<tItemCoord>& listData, u32 axis, tGrowableArray< tEvent >& events );
		void fFindInsertionPoints( u32 axis, const tGrowableArray<tItemCoord>& input, tGrowableArray<tInsertPt>& out ) const;

		// This single coordinate insert variant is only for switching cells
		void fSortedInsert( tItem& item, tCoord value, b32 searchUp, tCoordinateLists list, u32 cellIndex, tGrowableArray< tEvent >& events );
		void fChangeCells( u32 cellIndex, b32 searchUp, tItem& item, tCoordinateLists list, tGrowableArray< tEvent >& events );

		void fConfigureCells( );
		void fAddEvent( tItem* a, tItem* b, tGrowableArray< tEvent >& events ) const;

		// not use for the algorithm, only for debugging
		void fValidateSort( );

		// Pair manager stuff
		class tIDManager
		{
		public:
			tIDManager( )
				: mNextID( 0 )
			{ }

			tIDType fGetID( ) { return mNextID++; }
			void fReset( ) { mNextID = 0; }

		private:
			tIDType mNextID;
		};

		class tPairManager
		{
		public:
			// dirty = false if you know the pair is overlapping
			void fAddPotentialPair( tItem& a, tItem& b, b32 dirty );

			// remove all non intersecting pairs
			void fPurge( );
			void fBakePairList( );

			void fRemoveItem( tItem& item );
			void fClearPairData( tItem& item );

			void fClear( );

			const tGrowableArray< tPair* >& fPairs( ) const { return mPairs; }

		private:
			tGrowableArray< tPair > mTable;
			tGrowableArray< tPair* > mPairs; //cached linear list of pairs
		};

		tIDManager mIDManager;
		tGrowableArray< tItemPtr > mItems;
		tPairManager mPairManager;
		
		struct tCell
		{
			debug_watch( tCell );
			tCoord mMin, mMax;
			tGrowableArray< tItemCoord > mSortedCoordinates;

			void fSetBounds( f32 min, f32 max );
			void fClear( );
			void fValidateSort( );
		};

		static const u32 cCellSubDivision = 10;

		struct tSortedAxis
		{
			debug_watch( tSortedAxis );
			tFixedArray< tCell, cCellSubDivision > mCells;

			void fClear( );
			void fValidateSort( );
		};

		tFixedArray< tSortedAxis, cAxisCount > mSortedAxes;
	};

	
}}

#endif//__tSortedOverlapTree__
