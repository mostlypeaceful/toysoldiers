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
		inline t* fTypedPtr( ) const { return mP; }

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
	/// \brief Allows you to embed "runtime only" data inline with serialized data.
	///  1.) It reserves the required space.
	///  2.) It cannot be no op constructed. When LIP data is constructed, your data will be too.
	///  3.) It will be safe to use in all real runtime scenarios. it's intended for you to modify this data after the file is loaded.
	///       Assign it, copy, copy construct, destruct, heap allocate, etc, all uses intented to be safe in runtime scenarios.
	///  4.) However, for load in place files, you will need to call fDestroy explicitly. 
	///       Since no destructors are call on tLoadInplace data. (it would to be a tNoOpTag destruction.)
	template<class t>
	class tLoadInPlaceRuntimeObject
	{
		declare_reflector( );
	private:
		typedef tFixedArray<Sig::byte,sizeof(t)> tObjectBuffer;
		tObjectBuffer mObjectBuffer;

	public:
		// Basic c++ runtime behavior.
		inline tLoadInPlaceRuntimeObject( )											{ new ( mObjectBuffer.fBegin( ) ) t( ); }
		inline ~tLoadInPlaceRuntimeObject( )										{ fTreatAsObject( ).~t( ); }

		// tLoadInPlace data functionality.		
		inline tLoadInPlaceRuntimeObject( tNoOpTag )								{ new ( mObjectBuffer.fBegin( ) ) t( ); }
		inline void fDestroy( ) /* Be sure to call destroy in fOnFileUnloading*/	{ fTreatAsObject( ).~t( ); }

		inline tLoadInPlaceRuntimeObject( const tLoadInPlaceRuntimeObject& other )					{ new ( mObjectBuffer.fBegin( ) ) t( other.fTreatAsObject( ) ); }		
		inline tLoadInPlaceRuntimeObject& operator = ( const tLoadInPlaceRuntimeObject& other )		{ fTreatAsObject( ) = other.fTreatAsObject( ); return *this; }

		inline t&			fTreatAsObject( )		{ return *( t* )		mObjectBuffer.fBegin( ); }
		inline const t&		fTreatAsObject( ) const { return *( const t* )	mObjectBuffer.fBegin( ); }
	};

	///
	/// \brief This class provides an easy means to store typed pointers within load-in-place files
	/// while serializing them as simple opaque void*s.  N.B.: Doesn't relocate in place or anything.
	template < typename T >
	class tLoadInPlaceRuntimePtr
	{
		declare_reflector( );
	private:
		void* mP;
	public:
		inline tLoadInPlaceRuntimePtr( ) { }
		inline tLoadInPlaceRuntimePtr( T* p ) : mP( p ) { }
		inline tLoadInPlaceRuntimePtr( tNoOpTag ) { }
		inline operator T*( ) const { return (T*)mP; }
		inline T* operator->( ) const { return (T*)mP; }
		inline T& operator*( ) const { return *(T*)mP; }

		inline T* fPtr( ) const { return (T*)mP; }
	};
}

#endif//__tLoadInPlaceObjects__
