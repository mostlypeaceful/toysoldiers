#ifndef __tPriorityQueue__
#define __tPriorityQueue__

// use macros to improve readability and facilitate changes
// these are undef'd at the end of the file to prevent pollution
#define TEMPLATE_LIST			template <class T, class CMP>
#define TEMPLATE_LIST_DEFAULT	template <class T, class CMP=tPQGreaterThanOrEqualTo<T> >
#define HEAP					tPriorityQueue<T,CMP>

namespace Sig
{

	// default heap comparison is a >= compare
	template <class T>
	struct tPQGreaterThanOrEqualTo
	{
		inline b32 operator()(const T &a, const T &b) { return a >= b; }
	};

	/*---------------------------------------------------------------------------
	class tPriorityQueue
		
	template params:
		class T:
			*the type of the objects to store in the heap
			*the type must support copy construction if any of the copying 
			methods are invoked
		class CMP:
			*the binary comparison operation that defines upward propagation of
			items
	---------------------------------------------------------------------------*/
	TEMPLATE_LIST_DEFAULT
	class tPriorityQueue : public CMP
	{
	public:
		// standard first class ADT operations
		tPriorityQueue(u32 size=1); // size specifies pre-allocation amount (for efficiency)
		tPriorityQueue(const tPriorityQueue &h);
		tPriorityQueue &operator=(const tPriorityQueue &h);
		~tPriorityQueue();

		// Adds an item to the heap
		void fPut(const T &t);

		// Returns element at the top of the heap
		// and removes it
		T fGet();

		// View the top of the heap without removal
		const T &fViewTop() const;

		// get the index of the first instance '==' specified value; returns a value greater than fCount( ) if not found
		u32 fIndexOf( const T& t ) const;

		// erases the value at the specified index
		void fEraseIndex( u32 index );

		// finds the first instance '==' specified value and erases it; returns true if element was found
		b32 fErase( const T& t );

		// updates the item in the heap (assumes the cost of the item has changed)
		void fUpdateIndex( u32 index );

		// finds the first instance '==' specified value and updates it; returns true if element was found
		b32 fUpdate( const T& t );

		// Returns the number of elements in the heap
		u32 fCount() const;

		//	Destroys all the elements in the heap
		void fClear();

	private:
		void fCopy(const tPriorityQueue &h);
		inline u32 fLeft(u32 i);
		inline u32 fRight(u32 i);
		inline u32 fParent(u32 i);
		void fFixDown(u32 current);
		void fFixUp(u32 current);

		// compare functor
		inline CMP& fCompare( ) { return *this ; }

		// item count
		u32 mItemCount;

		// the array representing the balanced binary tree
		tDynamicArray<T> mItems;
	};

	/*--------------------------------------------------------------
	Standard first-class ADT operations (construction, 
		copy construction, assignment, and destruction)
	--------------------------------------------------------------*/

	// Constructor parameter size allows user to 
	// hint at the maximum size of the heap
	// the heap will not be limited to this, but it will
	// prevent constant resizing of the array used to
	// represent the heap
	TEMPLATE_LIST
	HEAP::tPriorityQueue(u32 size)
	: mItemCount(0)
	{
		if ( size < 1 )
			size = 1;
		mItems.fNewArray( size );
	}

	TEMPLATE_LIST
	HEAP::tPriorityQueue(const tPriorityQueue &h)
	{
		fCopy(h);
	}

	TEMPLATE_LIST
	HEAP &HEAP::operator=(const tPriorityQueue &h)
	{
		if ( this != &h )
		{
			fClear();
			fCopy(h);
		}
		return *this;
	}

	TEMPLATE_LIST
	HEAP::~tPriorityQueue()
	{
	}


	/*--------------------------------------------------------------
	Empty()
		Destroys all the elements in the heap
	Params:
		void
	Return:
		void
	--------------------------------------------------------------*/
	TEMPLATE_LIST
	void HEAP::fClear()
	{
		mItemCount = 0;
		mItems.fNewArray( 1 );
	}

	/*--------------------------------------------------------------
	fCopy()
		Makes an exact copy of the heap passed, copy constructing
		all the elements contained therein
	Params:
		const tPriorityQueue &h:
			const reference to the heap to copy
	Return:
		void
	--------------------------------------------------------------*/
	TEMPLATE_LIST
	void HEAP::fCopy(const tPriorityQueue &h)
	{
		mItemCount = h.mItemCount;
		mItems.fNewArray( h.mItems.fCount( ) );
		for ( u32 i = 1; i <= mItemCount; ++i )
			mItems[i] = h.mItems[i];
	}

	/*--------------------------------------------------------------
	fRight()
		Calculates the index of a given item's left child, assuming
		balanced binary tree
	Params:
		u32 i:
			index of the item whose left child index will be returned
	Return:
		u32:
			index of the left child
	--------------------------------------------------------------*/
	TEMPLATE_LIST
	inline u32 HEAP::fLeft(u32 i)
	{
		// i * 2
		return( i << 1 );
	}

	/*--------------------------------------------------------------
	fRight()
		Calculates the index of a given item's right child, assuming
		balanced binary tree
	Params:
		u32 i:
			index of the item whose right child index will be returned
	Return:
		u32:
			index of the right child
	--------------------------------------------------------------*/
	TEMPLATE_LIST
	inline u32 HEAP::fRight(u32 i)
	{
		// i * 2 + 1
		return( (i << 1) + 1 );
	}

	/*--------------------------------------------------------------
	fParent()
		Calculates the index of a given item's parent, assuming
		balanced binary tree
	Params:
		u32 i:
			index of the item whose parent index will be returned
	Return:
		u32:
			index of the parent
	--------------------------------------------------------------*/
	TEMPLATE_LIST
	inline u32 HEAP::fParent(u32 i)
	{
		// i / 2 (integer division, drop remainder)
		return( i >> 1 );
	}

	/*--------------------------------------------------------------
	fFixDown()
		Moves down the heap from the index passed, "fixing" it along
		the way.  This means that parent/children elements are
		compared, and elements with lower priority are propagated
		to the bottom
	Params:
		u32 cur:
			index into the array denoting the item at which
			to begin fixing downward
	Return:
		void
	--------------------------------------------------------------*/
	TEMPLATE_LIST
	void HEAP::fFixDown(u32 cur)
	{
		// start by going to left child, and
		// continue as long as current's children are
		// larger than current
		u32 child = fLeft(cur);
		while ( child <= mItemCount )
		{
			// do not go beyond the array bounds,
			// and check if right child is larger than left
			if ( child < mItemCount && !fCompare( )(mItems[child], mItems[child+1]) )
				++child; // right is larger, so make child be right

			// if current is larger than child, we're done
			// by virtue of balanced binary tree property
			if ( fCompare( )(mItems[cur], mItems[child]) )
				break;

			// child is larger than current, so swap
			// (propagate smaller downward)
			std::swap(mItems[cur], mItems[child]);

			// repeat
			cur = child;
			child = fLeft(cur);
		}
	}

	/*--------------------------------------------------------------
	fFixUp()
		Moves up the heap from the index passed, "fixing" it along
		the way.  This means that parent/children elements are
		compared, and elements with higher priority are propagated
		to the top
	Params:
		u32 cur:
			index into the array denoting the item at which
			to begin fixing upward
	Return:
		void
	--------------------------------------------------------------*/
	TEMPLATE_LIST
	void HEAP::fFixUp(u32 cur)
	{
		// while we haven't reached the top
		while ( cur > 1 )
		{
			// get my parent
			u32 parent = fParent( cur );

			// compare me and my parent
			// if my parent is "larger", then nothing else to
			// do (assumes heap is ordered before I started fixing)
			// else swap me with my parent and move up heap

			if ( fCompare( )( mItems[ parent ], mItems[ cur ] ) )
				break;
			else
			{
				std::swap( mItems[ cur ], mItems[ parent ] );
				cur = parent;
			}
		}
	}

	/*--------------------------------------------------------------
	Put()
		Adds an item to the heap
	Params:
		const T &t:
			const reference to the item to copy and insert
	Return:
		void
	--------------------------------------------------------------*/
	TEMPLATE_LIST
	void HEAP::fPut(const T &t)
	{
		// increment the item count and check
		// if we need to grow the array
		if ( ++mItemCount >= mItems.fCount( )-1 )
			mItems.fResize( 2 * mItems.fCount( ) );

		// insert the new item at the end of the heap and fix up
		mItems[ mItemCount ] = t;
		fFixUp( mItemCount );
	}

	/*--------------------------------------------------------------
	Get()
		Returns the the item with highest priority and removes
		it from the heap.
	Params:
		void
	Return:
		T:
			copy of the item at the top of the heap
	--------------------------------------------------------------*/
	TEMPLATE_LIST
	T HEAP::fGet()
	{
		// put the item with highest priority
		// in auto-deleting pointer
		T t = mItems[ 1 ];

		// put the last element in the heap at the head and fix down
		mItems[ 1 ] = mItems[ mItemCount ];
		mItems[ mItemCount ] = T( );
		--mItemCount;
		fFixDown( 1 );

		// return the item
		return t;
	}

	/*--------------------------------------------------------------
	ViewTop()
		Allows user to "view" the item with highest priority
		without removing it from the heap
	Params:
		void
	Return:
		const T &:
			const reference to the item at the top of the heap
	--------------------------------------------------------------*/
	TEMPLATE_LIST
	const T &HEAP::fViewTop() const
	{
		return mItems[1];
	}

	// get the index of the first instance '==' specified value; returns a value greater than fCount( ) if not found
	TEMPLATE_LIST
	u32 HEAP::fIndexOf( const T& t ) const
	{
		for ( u32 i = 1; i <= mItemCount; ++i )
			if( t == mItems[i] )
				return i;
		return ~0;
	}

	// erases the value at the specified index
	TEMPLATE_LIST
	void HEAP::fEraseIndex( u32 index )
	{
		sigassert( index <= mItemCount );

		// put the last element in the heap at the position to erase and then fix both up and down
		mItems[ index ] = mItems[ mItemCount ];
		mItems[ mItemCount ] = T( );
		--mItemCount;

		if( index <= mItemCount ) // check index again in case we just removed the last item in the pq
		{
			fFixUp( index );
			fFixDown( index );
		}
	}

	// finds the first instance '==' specified value and erases it; returns true if element was found
	TEMPLATE_LIST
	b32 HEAP::fErase( const T& t )
	{
		const u32 index = fIndexOf( t );
		if( index <= mItemCount )
		{
			fEraseIndex( index );
			return true;
		}
		return false;
	}

	// updates the item in the heap (assumes the cost of the item has changed)
	TEMPLATE_LIST
	void HEAP::fUpdateIndex( u32 index )
	{
		sigassert( index <= mItemCount );

		// store item at specified index
		T t = mItems[ index ];

		// erase item
		fEraseIndex( index );

		// re-add item
		fPut( t );
	}

	// finds the first instance '==' specified value and updates it; returns true if element was found
	TEMPLATE_LIST
	b32 HEAP::fUpdate( const T& t )
	{
		const u32 index = fIndexOf( t );
		if( index <= mItemCount )
		{
			fUpdateIndex( index );
			return true;
		}
		return false;
	}


	/*--------------------------------------------------------------
	Size()
		Returns the number of items stored in the heap
	Params:
		void
	Return:
		u32:
			the number of items stored in the heap
	--------------------------------------------------------------*/
	TEMPLATE_LIST
	u32 HEAP::fCount() const
	{
		return mItemCount;
	}

}

//undefine macros to prevent pollution of global macro namespace
#undef TEMPLATE_LIST
#undef TEMPLATE_LIST_DEFAULT
#undef HEAP

#endif//__tPriorityQueue__
