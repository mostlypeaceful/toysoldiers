#ifndef __tMasterPtr__
#define __tMasterPtr__
#include "tWeakPtr.hpp"

namespace Sig
{

	///
	/// \brief A container for a ref-counted pointer, contains the master shell.
	/// One master object is shared by many master pointer instances.
	template<class t>
	class tMasterObject : public tUncopyable, public tRefCounter
	{
	public:
		tRefCounterPtr< tMasterShell<t> > mPtrShell;

		inline tMasterObject( ) : mPtrShell( fNewRefCounterPtr< tMasterShell<t> >( ) ) { }
		inline ~tMasterObject( ) { sigassert( !mPtrShell.fNull( ) ); fDeleteMaster( ); }
		void fDeleteMaster( ) { if( mPtrShell->mP ){ delete mPtrShell->mP; mPtrShell->mP = 0; } }
	};

	///
	/// \brief The master pointer is a strong pointer that can hand out weak pointers.
	///
	/// Every weak pointer ultimately comes from a master pointer.  Only the reference
	/// counts of the master pointers contribute to the lifetime of the object pointed to;
	/// in other words, if there is one master pointer, with N weak pointers, all pointing
	/// to the same object, and the master pointer is released, the object itself will
	/// be deleted; the weak pointers will now return null, reflecting that there are no
	/// more "strong" references to the object.
	template<class t>
	class tMasterPtr : public tRefCounterPtr< tMasterObject<t> >
	{
	public:
		typedef tRefCounterPtr< tMasterObject<t> > tBase;

		inline tMasterPtr( ) { }
		inline explicit tMasterPtr( t* p ) : tBase( fNewRefCounterPtr< tMasterObject<t> >( ) ) { fGetShell( )->mP = p; }

		///
		/// \brief Create and set a new master instance.
		///
		/// This will change what all outstanding weak pointers are pointing to, as well
		/// as release a reference on the previous master (if any).
		template<class newt>
		inline void fNewMaster()
		{
			if( fNull( ) )
				fNewMasterObject( );
			else
				tBase::mP->fDeleteMaster( );
			fGetShell( )->mP = NEW newt;
		}

		///
		/// \brief Create a new weak pointer pointing to the underlying master object.
		inline tWeakPtr<t> fHandOutWeakPtr( ) const { return fNull( ) ? tWeakPtr<t>( ) : tWeakPtr<t>( fGetShell( ) ); }

		///
		/// \brief Reference dereference operator. Slightly misleading because it actually performs
		/// a double-pointer dereference, returning not the master shell object, but the object
		/// pointed to by the master shell.
		inline t& operator*( ) const { return *fGetRawPtr( ); }

		///
		/// \brief Pointer dereference operator. Slightly misleading because it actually performs
		/// a double-pointer dereference, returning not the master shell object, but the object
		/// pointed to by the master shell.
		inline t* operator->( ) const { return fGetRawPtr( ); }

		///
		/// \brief Access the raw pointers; note that this can be unsafe and interfere with the 'smart-'ness of the pointer...
		inline t* fGetRawPtr() const {return tBase::fNull( ) ? 0 : tBase::mP->mPtrShell->mP;}

		///
		/// \brief Test for validity (whether the underlying pointer can be safely de-referenced).
		/// \return true if the underlying pointer is NOT safe to dereference.
		inline b32 fNull( ) const {return !fGetRawPtr( ); }

		///
		/// \brief Convert to boolean as a test for validity (whether the underlying pointer can be safely de-referenced).
		/// \return true if the underlying pointer IS safe to dereference.
		inline operator b32() const {return !fNull();}

	private:

		inline tRefCounterPtr< tMasterShell<t> >& fGetShell( ) const { return tBase::mP->mPtrShell; }
		inline void fNewMasterObject( ) { *static_cast<tBase*>( this ) = fNewRefCounterPtr< tMasterObject<t> >( ); }
	};

}

#endif//__tMasterPtr__
