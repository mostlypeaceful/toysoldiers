//------------------------------------------------------------------------------
// \file tLinkedList.hpp - 23 Aug 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tLinkedList__
#define __tLinkedList__

namespace Sig
{
	//------------------------------------------------------------------------------
	// All instances of the doubly link and doubly list should be declared using
	// these macros to ensure that variables exist and are offset properly
	//------------------------------------------------------------------------------
	#define doubly_link( label ) tDoublyLink label
	#define doubly_list( t, m ) tDoublyList<t, offsetof( t, m )>

	///
	/// \class tDoublyLink
	/// \brief Usage requires that this link be available for the doubli link list
	///		   which accesses the member link variable invasively and uses offsetof
	///		   to move from one parent object to the next
	class tDoublyLink
	{
	public:

		tDoublyLink( ) : mPrevious( NULL ), mNext( NULL ) { }
		~tDoublyLink( ) { fUnlink( ); }

		b32 fIsLinked( ) const { return mNext || mPrevious; }

		tDoublyLink * fNext( ) { return mNext && mNext->mNext ? mNext : NULL; }
		const tDoublyLink * fNext( ) const { return mNext && mNext->mNext ? mNext : NULL; }

		tDoublyLink * fPrev( ) { return mPrevious && mPrevious->mPrevious ? mPrevious : NULL; }
		const tDoublyLink * fPrev( ) const { return mPrevious && mPrevious->mPrevious ? mPrevious : NULL; }

		void fUnlink( )
		{
			if( mPrevious )
				mPrevious->mNext = mNext;
			if( mNext )
				mNext->mPrevious = mPrevious;

			mPrevious = NULL;
			mNext = NULL;
		}

		void fInsertAfter( tDoublyLink * link )
		{
			link->mPrevious = this;
			link->mNext = mNext;
			if( mNext )
				mNext->mPrevious = link;
			mNext = link;
		}

		void fInsertBefore( tDoublyLink * link )
		{
			link->mNext = this;
			link->mPrevious = mPrevious;
			if( mPrevious )
				mPrevious->mNext = link;
			mPrevious = link;
		}

	private:

		tDoublyLink * mPrevious;
		tDoublyLink * mNext;
	};

	///
	/// \class tDoublyList
	/// \brief Invasively uses nodes stored as member variables on type T
	///		   templatized on the offset for the member from T and uses
	///        the member variable to move along the nodes
	template<class T, int LnkOffset>
	class tDoublyList
	{
	public:

		tDoublyList( ) { mHead.fInsertAfter( &mTail ); }
		~tDoublyList( ) { fUnlinkAll( ); }

		void fLinkFront( T * toLink )
		{
			sigassert( toLink );

			tDoublyLink * link = fGetLink( toLink );
			link->fUnlink( );

			mHead.fInsertAfter( link );
		}

		void fLinkBack( T * toLink )
		{
			sigassert( toLink );

			tDoublyLink * link = fGetLink( toLink );
			link->fUnlink( );

			mTail.fInsertBefore( link );
		}

		void fUnlink( T * toUnlink )
		{
			sigassert( toUnlink );

			fGetLink( toUnlink )->fUnlink( );
		}

		T * fHead( ) { return fGetObject( mHead.fNext( ) ); }
		const T * fHead( ) const { return fGetObject( mHead.fNext( ) ); }

		T * fTail( ) { return fGetObject( mTail.fPrev( ) ); }
		const T * fTail( ) const { return fGetObject( mTail.fPrev( ) ); }

		T * fNext( T * curr )
		{
			return fGetObject( fGetLink( curr )->fNext( ) );
		}

		const T * fNext( const T * curr ) const
		{
			return fGetObject( fGetLink( curr )->fNext( ) );
		}

		T * fPrev( T * curr )
		{
			return fGetObject( fGetLink( curr )->fPrev( ) );
		}

		const T * fPrev( const T * curr ) const
		{
			return fGetObject( fGetLink( curr )->fPrev( ) );
		}

		void fUnlinkAll( )
		{
			while( mHead.fNext( ) )
				mHead.fNext( )->fUnlink( );
		}

		u32 fCount_ReallySlowly( ) const
		{
			u32 count = 0;
			const tDoublyLink* iter = mHead.fNext( );
			while( iter )
			{
				++count;
				iter = iter->fNext( );
			}
			return count;
		}

	private:

		T * fGetObject( tDoublyLink * link )
		{
			if( link )
				return (T*)(((Sig::byte*)link) - LnkOffset);
			else 
				return NULL;
		}

		const T * fGetObject( const tDoublyLink * link ) const
		{
			if( link )
				return (const T*)(((const Sig::byte*)link) - LnkOffset);
			else 
				return NULL;
		}

		tDoublyLink * fGetLink( T * toGet )
		{
			if( toGet )
				return (tDoublyLink *)(((Sig::byte*)toGet) + LnkOffset);
			else
				return NULL;
		}

		const tDoublyLink * fGetLink( const T * toGet ) const
		{
			if( toGet )
				return (const tDoublyLink *)(((const Sig::byte*)toGet) + LnkOffset);
			else
				return NULL;
		}

	private:

		tDoublyLink mHead;
		tDoublyLink mTail;
	};

	typedef tDoublyList<tDoublyLink, 0> tSimpleDoublyList;

};

#endif//__tLinkedList__
