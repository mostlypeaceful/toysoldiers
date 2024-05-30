#ifndef __tLoadInPlaceObjects__
#define __tLoadInPlaceObjects__

namespace Sig
{
	///
	/// \brief This class provides a work around for the fact that pointers-to-pointers
	/// break the auto-load-in-place serialization system.
	///
	/// An example would be if you wanted your load in place class to contain an element declared like so:
	///		tDynamicArray<tSomeType*> mArray;
	/// The problem here is that tDynamicArray is implemented as a T* under the hood, which,
	/// when combined with tSomeType*, gives you tSomeType**. Currently the reflection system
	/// can only handle pointers of a single level of depth. To get around this, just use
	/// the tLoadInPlacePtrWrapper, making your declaration look like:
	///		tDynamicArray< tLoadInPlacePtrWrapper<tSomeType> > mArray;
	/// This allows you to still treat mArray like it contains an array of pointers to tSomeType,
	/// and none of your syntax has to change.
	template<class t>
	struct tLoadInPlacePtrWrapper
	{
		declare_reflector( );
	private:
		t* mP;
	public:
		inline tLoadInPlacePtrWrapper( ) { }
		inline tLoadInPlacePtrWrapper( t* p ) : mP( p ) { }
		inline tLoadInPlacePtrWrapper( tNoOpTag ) { }
		inline operator t*( ) const { return mP; }
		inline t* operator->( ) const { return mP; }
		inline Sig::byte* fRawPtr( ) const { return ( Sig::byte* )mP; }

		///
		/// \brief You probably shouldn't be using this method. Only useful (or safe) for
		/// relocatin of load-in-place resources. Any other use will cause memory leaks
		/// and general memory corruption.
		inline void fRelocateInPlace( ptrdiff_t delta )
		{
			mP = ( t* )( ( Sig::byte* )mP + delta );
		}
	};

	///
	/// \brief Helps out in embedding objects that require runtime construction/initialization/destruction
	/// within load-in-place files. I.e., certain load-in-place objects might require certain items to be
	/// initialized at run-time immediately following load. This object allows you to wrap those objects
	/// so that they can be safely embedded within the load-in-place file. You will need to manually
	/// call the fConstruct and fDestroy methods, presumably in the fOnFileLoaded( ) and fOnFileUnloading( )
	/// callbacks. After you call fConstruct, you can treat the object as normal, and perform your
	/// custom intialization.
	template<class t>
	class tLoadInPlaceRuntimeObject
	{
		declare_reflector( );
	private:
		typedef tFixedArray<Sig::byte,sizeof(t)> tObjectBuffer;
		tObjectBuffer mObjectBuffer;
	public:

		inline tLoadInPlaceRuntimeObject( )
		{
			fZeroOut( mObjectBuffer );
		}

		inline tLoadInPlaceRuntimeObject( tNoOpTag )
		{
		}

		inline t&			fTreatAsObject( )	
												{ return *( t* )		mObjectBuffer.fBegin( ); }
		inline const t&		fTreatAsObject( ) const
												{ return *( const t* )	mObjectBuffer.fBegin( ); }

		template<class tArg>
		void		fConstruct( const tArg& arg )
												{ new ( mObjectBuffer.fBegin( ) ) t( arg ); }
		void		fConstruct( )				{ new ( mObjectBuffer.fBegin( ) ) t( ); }
		void		fDestroy( )					{ fTreatAsObject( ).~t( ); }
	};

}

#endif//__tLoadInPlaceObjects__
