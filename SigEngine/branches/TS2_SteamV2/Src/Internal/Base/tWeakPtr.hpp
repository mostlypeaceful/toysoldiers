#ifndef __tWeakPtr__
#define __tWeakPtr__

namespace Sig
{

	///
	/// \brief A ref-counter pointer type, contains the final target pointer
	/// of a master-pointer/weak-pointer chain.
	template<class t>
	class tMasterShell : public tUncopyable, public tRefCounter
	{
	public:
		inline explicit tMasterShell(t* p=0) : mP(p) { }
		t* mP;
	};

	///
	/// \brief The weak pointer allows for safely finding out that an object 
	/// has been deleted, without holding an outstanding reference on that object.
	///
	/// The weak pointer's 'null' method returns true when the object pointed to
	/// has been deleted.  This allows for users of a shared object to safely 
	/// pass around references of the object without forcing the object to remain
	/// allocated, while at the same time finding out and safely handling the case
	/// when the object has been deleted (by checking the 'null' method before
	/// performing any dereferencing).
	/// 
	/// \see tMasterPtr for more information on the interaction between
	/// weak and master pointers.
	template<class t>
	class tWeakPtr : public tRefCounterPtr< tMasterShell<t> >
	{
	public:

		typedef tRefCounterPtr< tMasterShell<t> > tBase;

		inline tWeakPtr( ) { }
		inline tWeakPtr( const tBase& b ) : tBase( b ) { }

		///
		/// \brief Test for validity (whether the underlying pointer can be safely de-referenced).
		/// \return true if the underlying pointer is NOT safe to dereference.
		inline b32 fNull( ) const { return fGetRawPtr( )==0; }

		///
		/// \brief Convert to boolean as a test for validity (whether the underlying pointer can be safely de-referenced).
		/// \return true if the underlying pointer IS safe to dereference.
		inline operator b32() const{return !fNull();}

		///
		/// \brief Reference dereference operator. Slightly misleading because it actually performs
		/// a double-pointer dereference, returning not the master shell object, but the object
		/// pointed to by the master shell.
		inline t& operator*( ) const {return *fGetRawPtr( );}

		///
		/// \brief Pointer dereference operator. Slightly misleading because it actually performs
		/// a double-pointer dereference, returning not the master shell object, but the object
		/// pointed to by the master shell.
		inline t* operator->( ) const {return fGetRawPtr( );}

		///
		/// \brief Access the raw pointers; note that this can be unsafe and interfere with the 'smart-'ness of the pointer...
		inline t* fGetRawPtr() const {return tBase::fNull( ) ? 0 : tBase::mP->mP;}

#define define_comparison_op( op ) inline b32 operator op( const tWeakPtr& other ) const { return tBase::operator op ( static_cast<const tBase&>( other ) ); }
		define_comparison_op( == )
		define_comparison_op( != )
		define_comparison_op( < )
		define_comparison_op( <= )
		define_comparison_op( > )
		define_comparison_op( >= )
#undef define_comparison_op

#define define_comparison_op( op ) inline b32 operator op( const t* other ) const { return fGetRawPtr( ) op other; }
		define_comparison_op( == )
		define_comparison_op( != )
		define_comparison_op( < )
		define_comparison_op( <= )
		define_comparison_op( > )
		define_comparison_op( >= )
#undef define_comparison_op

#define define_comparison_op( op ) inline friend b32 operator op( const t* a, const tWeakPtr& b ) { return b op a; }
		define_comparison_op( == )
		define_comparison_op( != )
		define_comparison_op( < )
		define_comparison_op( <= )
		define_comparison_op( > )
		define_comparison_op( >= )
#undef define_comparison_op

	};

}


#endif//__tWeakPtr__
