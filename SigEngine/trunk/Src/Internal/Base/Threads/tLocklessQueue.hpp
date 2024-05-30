#ifndef __tLocklessQueue__
#define __tLocklessQueue__

namespace Sig { namespace Threads
{

	///
	/// \brief Allows for one thread to feed another thread with tasks,
	/// without using expensive mutex or critical-section thread synchronization.
	///
	/// \note As it stands, the lockless queue can only grow dynamically in special
	/// circumstances (see method fGrow).  This means the push and pop operations 
	/// will not automatically resize the queue, and will return false if they do not 
	/// succeed.  Likewise, you can query for whether the queue is full.
	///
	/// The lockless queue is designed to work with two threads only.  It is required 
	/// that one thread (call it A, or the primary thread) do the pushing/feeding, 
	/// and the other thread (call it B, or the worker thread) do the popping/consuming.
	template<class t>
	class tLocklessQueue : public tUncopyable
	{
		u32 mCapacity;
		t*	mEntries;
		u32 mHead;
		u32 mTail;

	public:

		tLocklessQueue( u32 capacity )
			: mCapacity( capacity )
			, mHead( 0 )
			, mTail( 0 )
		{
			mEntries = NEW t[mCapacity];
		}

		~tLocklessQueue( )
		{
			delete [] mEntries;
		}

		inline u32 fGetCapacity( ) const
		{
			return mCapacity;
		}

		inline b32 fIsFull( ) const
		{
			const u32 nextHead = ( mHead + 1 ) % mCapacity;
			return( nextHead == mTail );
		}

		inline b32 fIsEmpty( ) const
		{
			return( mHead == mTail );
		}

		b32 fPush( const t& p )
		{
			const u32 nextHead = ( mHead + 1 ) % mCapacity;
			if( nextHead == mTail )
				return false;

			mEntries[mHead] = p;
			mHead = nextHead;
			return true;
		}

		b32 fPop( t& p )
		{
			if( mHead == mTail )
			{
				p = t( );
				return false;
			}

			p = mEntries[mTail];
			mEntries[mTail] = t( );
			mTail = ( mTail + 1 ) % mCapacity;
			return true;
		}

		///
		/// \brief Increase the queue's capacity.
		///
		/// This method is not safe unless the other thread 
		/// (NOT the calling thread) has been suspended or
		/// can somehow be guaranteed to not be accessing the queue.
		void fGrow( u32 mul, u32 add )
		{
			const u32 oldSize = mCapacity;
			const u32 newSize = mCapacity * mul + add;

			mCapacity = newSize;

			t* newEntries = NEW t[mCapacity];
			t* oldEntries = mEntries;

			u32 inew = 0, iold = mTail;
			for( ; iold != mHead; ( iold = (iold + 1) % oldSize ), ++inew )
				newEntries[inew] = oldEntries[iold];

			mTail = 0;
			mHead = inew;
			mEntries = newEntries;

			delete [] oldEntries;
		}

	};


}}


#endif//__tLocklessQueue__
