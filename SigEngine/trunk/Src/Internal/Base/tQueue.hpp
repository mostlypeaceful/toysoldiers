#ifndef __tQueue__
#define __tQueue__

namespace Sig
{
	/// \brief First In First Out queue
	/// \note Currently using tGrowableArray for the inner workings, should be using a linked list @TODO make linked lists!
	template<class t>
	class tQueue
	{
	private:
		tGrowableArray< t > mItems;

	public:

		tQueue( )
		{
		}

		///
		/// \brief Initialize the queue with a specified number of allocated slots.
		explicit tQueue( u32 startCapacity )
		{
			mItems.fReserve( startCapacity );
		}

		///
		/// \brief Add a new object to the back of the queue.
		void fPush( const t& object )
		{
			mItems.fPushBack( object );
		}

		///
		/// \brief This version will copy the front of the
		/// queue to the output parameter before removing it.
		b32 fPop( t& object )
		{
			if( mItems.fCount( ) > 0 )
			{
				object = mItems.fFront( );
				mItems.fEraseOrdered( 0 );
				return true;
			}
			return false;
		}

		///
		/// \brief This version doesn't bother copying the front of the queue
		/// as an output parameter.
		b32 fPop( )
		{
			if( mItems.fCount( ) > 0 )
			{
				mItems.fEraseOrdered( 0 );
				return true;
			}
			return false;
		}

		inline u32		fCount( ) const	{ return mItems.fCount( ); }
		inline void		fClear( )		{ mItems.fDeleteArray( ); }
		inline b32		fEmpty( ) const { return fCount( )==0; }
		inline t&		fBottom( )		{ return mItems.fFront( ); }
		inline const t&	fBottom( ) const{ return mItems.fFront( ); }
		inline t&		fTop( )			{ return mItems.fBack( ); }
		inline const t&	fTop( ) const	{ return mItems.fBack( ); }
		inline t*		fBegin( )		{ return mItems.fBegin( ); }
		inline const t* fBegin( ) const { return mItems.fBegin( ); }
		inline t*		fEnd( )			{ return mItems.fEnd( ); }
		inline const t* fEnd( ) const	{ return mItems.fEnd( ); }
	};

}

#endif//__tStack__
