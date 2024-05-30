#ifndef __tRefCounterPtr__
#define __tRefCounterPtr__

namespace Sig
{
	///
	/// \brief Provides a type that can be inherited from to ensure
	/// your derived type can be used with a tRefCounterPtr. You could
	/// alternatively implement the same interface on your type (for example
	/// if you wanted to do something special in one of the methods).
	class base_export tRefCounter
	{
		declare_reflector( );
	private:
		s32 mRefCount;
	public:
		inline		tRefCounter( ) : mRefCount( 0 ) { }
		inline s32	fRefCount( ) const { return mRefCount; }
		inline void	fAddRef( ) const { ++const_cast<s32&>( mRefCount ); }
		inline s32  fDecRef( ) const { return --const_cast<s32&>( mRefCount ); }
	};

	class base_export tThreadSafeRefCounter
	{
		declare_null_reflector( );
	private:
		volatile s32 mRefCount;
	public:
		inline		tThreadSafeRefCounter( ) : mRefCount( 0 ) { }
		inline s32	fRefCount( ) const { return mRefCount; }
		inline void	fAddRef( ) const { interlocked_inc( &mRefCount ); }
		inline s32  fDecRef( ) const { return interlocked_dec( &mRefCount ); }
	};

	template<class t>
	inline void fStandardRefCounterPtrAddRef( t* p )
	{
		p->fAddRef( );
	}

	///
	/// \brief This templatized function allows a compile time 'optional' callback
	/// mechanism to receive a notification when the reference count of a strong
	/// pointer is about to be incremented. By default, this function will call
	/// the standard fStandardRefCounterPtrAddRef which will increment the reference
	/// count of the pointer; however, if you want a notification for
	/// a tRefCounterPtr<tYourClass>, then you must define a specialization of this
	/// function for tYourClass; additionally, you should be sure you actually increment
	/// the reference count of the object.
	template<class t>
	inline void fRefCounterPtrAddRef( t* p )
	{
		fStandardRefCounterPtrAddRef( p );
	}

	template<class t>
	inline void fStandardRefCounterPtrDecRef( t* p )
	{
		// decr. ref. count
		const s32 refCount = p->fDecRef( );
		sigassert(refCount >= 0);

		// and if ref. count is zero
		if(refCount == 0)
		{
			// then delete the data itself
			delete p;
		}
	}

	///
	/// \brief This templatized function allows a compile time 'optional' callback
	/// mechanism to receive a notification when the reference count of a strong
	/// pointer is about to be decremented. By default, this function will call
	/// the standard fStandardRefCounterPtrDecRef which will decrement the reference
	/// count and potentially delete the pointer; however, if you want a notification for
	/// a tRefCounterPtr<tYourClass>, then you must define a specialization of this
	/// function for tYourClass; additionally, you should be sure you actually decrement
	/// the reference count and handle deletion of the object.
	template<class t>
	inline void fRefCounterPtrDecRef( t* p )
	{
		fStandardRefCounterPtrDecRef( p );
	}



	template<class t>
	inline u32 fStandardRefCounterPtrRefCount( t* p )
	{
		return p->fRefCount( );
	}

	///
	/// \brief This templatized function allows a customizable way of returning the
	/// reference count associated with an object. By default, this function will call
	/// the standard fStandardRefCounterPtrRefCount, which will simply return the ref
	/// count of the underlying tRefCounter object; however, if you want to define your
	/// way of getting the ref count for tRefCounterPtr<tYourClass>, then you must define 
	/// a specialization of this function for tYourClass; additionally, you should be sure 
	/// you actually return the valid reference count of the object.
	template<class t>
	inline u32 fRefCounterPtrRefCount( t* p )
	{
		return fStandardRefCounterPtrRefCount( p );
	}

	///
	/// \brief Provides "strong" ownership and smart pointer reference-counting semantics.
	/// tRefCounterPtr differs from tStrongPtr in that it expects the object itself
	/// contains the reference count variable, and supports the tRefCounter interface.
	/// Otherwise, their semantics are the same.
	///
	/// As long as the reference count on the underlying shared pointer is > 0, the 
	/// pointed-to object will not be deleted.  This means you have to be careful to 
	/// break cyclic references explicilty.
	template<class t>
	class tRefCounterPtr
	{
		sig_make_loggable( tRefCounterPtr<t>, "RefPtr[ " << mP << " refs: " << fRefCount( ) << " ]" );
	public:
		typedef t tValueType;
	protected:
		t*		mP; ///< ptr to the data itself

	public:

		// ctor, dtor, assignment, etc.
		inline tRefCounterPtr() : mP(0) { }
		inline explicit tRefCounterPtr(t* p) { fCopy(p); }
		inline tRefCounterPtr(const tRefCounterPtr& h) { fCopy(h.mP); }
		inline tRefCounterPtr& operator=(const tRefCounterPtr& h) { if(mP!=h.mP){fDestroy(); fCopy(h.mP);} return *this; }
		inline ~tRefCounterPtr() { fDestroy(); }

		///
		/// \brief Test for validity (whether the underlying pointer can be safely de-referenced).
		/// \return true if the underlying pointer is NOT safe to dereference.
		inline b32 fNull() const{return mP==0;}

		///
		/// \brief Convert to boolean as a test for validity (whether the underlying pointer can be safely de-referenced).
		/// \return true if the underlying pointer IS safe to dereference.
		inline operator b32() const{return !fNull();}

		// comparison
		inline b32 operator==( const tRefCounterPtr& other ) const { return mP==other.mP; }
		inline b32 operator!=( const tRefCounterPtr& other ) const { return mP!=other.mP; }
		inline b32 operator<( const tRefCounterPtr& other ) const { return mP<other.mP; }
		inline b32 operator<=( const tRefCounterPtr& other ) const { return mP<=other.mP; }
		inline b32 operator>( const tRefCounterPtr& other ) const { return mP>other.mP; }
		inline b32 operator>=( const tRefCounterPtr& other ) const { return mP>=other.mP; }
		inline b32 operator==( const t* other ) const { return mP==other; }
		inline b32 operator!=( const t* other ) const { return mP!=other; }
		inline b32 operator<( const t* other ) const { return mP<other; }
		inline b32 operator<=( const t* other ) const { return mP<=other; }
		inline b32 operator>( const t* other ) const { return mP>other; }
		inline b32 operator>=( const t* other ) const { return mP>=other; }
		inline friend b32 operator==( const t* a, const tRefCounterPtr& b ) { return a==b.mP; }
		inline friend b32 operator!=( const t* a, const tRefCounterPtr& b ) { return a!=b.mP; }
		inline friend b32 operator<( const t* a, const tRefCounterPtr& b ) { return a<b.mP; }
		inline friend b32 operator<=( const t* a, const tRefCounterPtr& b ) { return a<=b.mP; }
		inline friend b32 operator>( const t* a, const tRefCounterPtr& b ) { return a>b.mP; }
		inline friend b32 operator>=( const t* a, const tRefCounterPtr& b ) { return a>=b.mP; }


		inline t& operator*() const {return *mP;}		///< Dereference operator.
		inline t* operator->() const {return mP;}		///< Dereference for calling functions on underlying pointer.

		///
		/// \brief Access the raw pointers; note that this can be unsafe and interfere with the 'smart-'ness of the pointer...
		inline t* fGetRawPtr() const {return mP;}

		///
		/// \brief Reset the strong pointer (release current ref count and object, and start new ref count with new object).
		inline void fReset( t* p ) { if(p!=mP) { fDestroy( ); fCopy( p ); } }

		// misc
		inline t* fDisown( ) { t* o = mP; mP = 0; return o; }
		inline void fRelease( ) { fDestroy( ); }
		inline u32 fRefCount( ) const {return mP ? fRefCounterPtrRefCount( mP ) : 0; }

	private:

		// copy data over from another ptr
		inline void fCopy(t* p)
		{
			// copy over the pointers
			mP=p;

			// if they were valid, then
			// increment the reference count
			if(mP) fRefCounterPtrAddRef( mP );
		}

		// release my pointers
		void fDestroy()
		{
			// if i have data
			if(mP)
			{
				// callback mechanism, see above
				fRefCounterPtrDecRef( mP );

				// zero out my pointers to data
				mP=0;
			}
		}
	};

	///
	/// \brief Create a strong pointer with a new instance of class 't'; 
	/// the reference count on this new object will be one.
	/// \return A valid strong pointer pointing to the newly created instance of 't'.
	template<class t>
	inline tRefCounterPtr<t> fNewRefCounterPtr( ) { return tRefCounterPtr<t>( NEW t ); }

}


#endif//__tRefCounterPtr__
