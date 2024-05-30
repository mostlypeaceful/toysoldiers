#ifndef __tStack__
#define __tStack__

namespace Sig
{

	template<class t>
	class tStack
	{
	private:
		tGrowableArray< t > mItems;

	public:

		tStack( )
		{
		}

		///
		/// \brief Initialize the stack with a specified number of allocated slots.
		explicit tStack( u32 startCapacity )
		{
			mItems.fReserve( startCapacity );
		}

		///
		/// \brief Add a new object to the top of the stack.
		void fPush( const t& object )
		{
			mItems.fPushBack( object );
		}

		///
		/// \brief This version will copy the top of the
		/// stack to the output parameter before removing it.
		b32 fPop( t& object )
		{
			if( mItems.fCount( ) > 0 )
			{
				object = mItems.fBack( );
				mItems.fSetCount( mItems.fCount( ) - 1 );
				return true;
			}
			return false;
		}

		///
		/// \brief This version doesn't bother copying the top of the stack
		/// as an output parameter.
		b32 fPop( )
		{
			if( mItems.fCount( ) > 0 )
			{
				mItems.fSetCount( mItems.fCount( ) - 1 );
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
