#ifndef __tStrongPtr__
#define __tStrongPtr__

namespace Sig
{
	///
	/// \brief This templatized function allows a compile time 'optional' callback
	/// mechanism to receive a notification when the reference count of a strong
	/// pointer is about to be decremented. By default, this function will get
	/// compiled out and have no effect; however, if you want a notification for
	/// a tStrongPtr<tYourClass>, then you must define a specialization of this
	/// function for tYourClass.
	template<class t>
	inline void fOnStrongPtrDecRef( t& object, u32 refCountAfterDecr )
	{
	}

	///
	/// \brief Provides "strong" ownership and smart pointer reference-counting semantics.
	///
	/// As long as the reference count on the underlying shared pointer is > 0, the 
	/// pointed-to object will not be deleted.  This means you have to be careful to 
	/// break cyclic references explicilty.
	template<class t>
	class tStrongPtr
	{
		t*		mP; ///< ptr to the data itself
		u32*	mR; ///< ptr to the shared reference count

	public:

		// ctor, dtor, assignment, etc.
		inline tStrongPtr() : mP(0), mR(0) { }
		inline explicit tStrongPtr(t* p) : mP(p), mR(0) { if(p){ mR = NEW u32(1); } }
		inline tStrongPtr(const tStrongPtr& h) { fCopy(h); }
		inline tStrongPtr& operator=(const tStrongPtr& h) { if(mP!=h.mP){fDestroy(); fCopy(h);} return *this; }
		inline ~tStrongPtr() { fDestroy(); }

		///
		/// \brief Test for validity (whether the underlying pointer can be safely de-referenced).
		/// \return true if the underlying pointer is NOT safe to dereference.
		inline b32 fNull() const{return mP==0;}

		///
		/// \brief Convert to boolean as a test for validity (whether the underlying pointer can be safely de-referenced).
		/// \return true if the underlying pointer IS safe to dereference.
		inline operator b32() const{return !fNull();}

		// comparison
		inline b32 operator==( const tStrongPtr& other ) const { return mP==other.mP; }
		inline b32 operator!=( const tStrongPtr& other ) const { return mP!=other.mP; }
		inline b32 operator<( const tStrongPtr& other ) const { return mP<other.mP; }
		inline b32 operator<=( const tStrongPtr& other ) const { return mP<=other.mP; }
		inline b32 operator>( const tStrongPtr& other ) const { return mP>other.mP; }
		inline b32 operator>=( const tStrongPtr& other ) const { return mP>=other.mP; }
		inline b32 operator==( const t* other ) const { return mP==other; }
		inline b32 operator!=( const t* other ) const { return mP!=other; }
		inline b32 operator<( const t* other ) const { return mP<other; }
		inline b32 operator<=( const t* other ) const { return mP<=other; }
		inline b32 operator>( const t* other ) const { return mP>other; }
		inline b32 operator>=( const t* other ) const { return mP>=other; }
		inline friend b32 operator==( const t* a, const tStrongPtr& b ) { return a==b.mP; }
		inline friend b32 operator!=( const t* a, const tStrongPtr& b ) { return a!=b.mP; }
		inline friend b32 operator<( const t* a, const tStrongPtr& b ) { return a<b.mP; }
		inline friend b32 operator<=( const t* a, const tStrongPtr& b ) { return a<=b.mP; }
		inline friend b32 operator>( const t* a, const tStrongPtr& b ) { return a>b.mP; }
		inline friend b32 operator>=( const t* a, const tStrongPtr& b ) { return a>=b.mP; }


		inline t& operator*() const {return *mP;}		///< Dereference operator.
		inline t* operator->() const {return mP;}		///< Dereference for calling functions on underlying pointer.

		///
		/// \brief Access the raw pointers; note that this can be unsafe and interfere with the 'smart-'ness of the pointer...
		inline t* fGetRawPtr() const {return mP;}

		///
		/// \brief Reset the strong pointer (release current ref count and object, and start NEW ref count with NEW object).
		inline void fReset( t* p ) { fDestroy( ); mP = p; if(p){ mR = NEW u32(1); } }

		// misc
		inline void fRelease() { fDestroy(); }
		inline int fRefCount() const {return mR?*mR:0;}

	private:

		// copy data over from another ptr
		inline void fCopy(const tStrongPtr &h)
		{
			// copy over the pointers
			mP=h.mP;
			mR=h.mR;

			// if they were valid, then
			// increment the reference count
			if(mP) ++(*mR);
		}

		// release my pointers
		void fDestroy()
		{
			// if i have data
			if(mP)
			{
				sigassert((*mR) > 0);

				// optional callback mechanism, see above
				fOnStrongPtrDecRef( *mP, *mR-1 );

				// decr. ref. count
				--(*mR);

				// and if ref. count is zero
				if((*mR)==0)
				{
					// then delete the data itself
					delete mP;
					delete mR;
				}

				// either way, zero out my pointers to data
				mP=0;
				mR=0;
			}
		}

	};



	///
	/// \brief Create a strong pointer with a new instance of class 't'; 
	/// the reference count on this new object will be one.
	/// \return A valid strong pointer pointing to the newly created instance of 't'.
	template<class t>
	inline tStrongPtr<t> fNewStrongPtr() { return tStrongPtr<t>( NEW t ); }

}


#endif//__tStrongPtr__
