#ifndef __tLoadInPlaceSerializer__
#define __tLoadInPlaceSerializer__
#include "tLoadInPlaceSerializerBase.hpp"
#include <list>

namespace Sig
{

	///
	/// \brief Saves an instance of a class to a user-supplied stream-out function.
	///
	/// the tLoadInPlaceSerializer class serializes an instance of a class, along with all its
	/// contained data (whether directly embedded or pointed to externally), to an
	/// arbitrary stream function (the caller can provide a functor that writes to file
	/// or simply to a byte stream).
	///
	/// the class instance can later be serialized back in (load-in-place style) using a 
	/// single block of memory, with all pointers fixed up.
	///
	/// will properly handle serializing polymorphic classes with vtables, PROVIDED these
	/// types derive from rtti::tSerializableBaseClass.
	class tools_export tLoadInPlaceSerializer : public tLoadInPlaceSerializerBase
	{
		class tPointee;

		tGrowableArray<tPointee*>	mPointees;

	public:

		///
		/// \brief fSave is the primary method and purpose of the tLoadInPlaceSerializer class;
		/// quite simply, it serializes an object out to a stream (it 'saves' the object).
		///
		/// \param object The object to serialize.
		/// \param output The signature of the output function should look like this:
		///			void operator( )( const void* data, u32 numBytes );
		/// \return The size in bytes of all the data.
		///
		/// Actually, the fSave function is a little more general; it writes the object
		/// to a user supplied put function, which allows the user to decide how it wants
		/// to write the stream data; could be to file, could be to a data buffer, etc.
		template<class t, class tPutFunction>
		u32 fSave( const t& object, tPutFunction& output, tPlatformId pid );

	private:

		///
		/// \brief Represents an instance of a pointer within an object (i.e., a T*)
		/// stores relevant information to aid in serializing the class instance.
		class tPointer
		{
		public:

			Rtti::tClassId		mCid;
			tPointee*			mTarget;
			u32					mOffsetFromPointeeBase;
			u32					mOffsetToPtr;
			const Rtti::tReflector*	mContext;

			tPointer( Rtti::tClassId id, tPointee* tgt, u32 ofpb, u32 otp, const Rtti::tReflector* context ) 
				: mCid(id), mTarget(tgt), mOffsetFromPointeeBase(ofpb), mOffsetToPtr(otp), mContext( context ) { }
		};

		///
		/// \brief Represents the data pointed to by one or more pointers within the
		/// primary instance being serialized (or another instance within the primary instance).
		class tools_export tPointee
		{
		public:

			const void*			mAddress;
			u32					mFullDataSize;
			const Rtti::tReflector& mReflector; 
			Rtti::tClassId		mCid;
			b32					mOwned;
			u32					mFilePos;
			tDynamicBuffer		mDataBuffer;
			std::list<tPointer>	mEmbeddedPtrs;
			tHashTable< void*,b32,tHashTableExpandOnlyResizePolicy > mEndianSwappedPtrs;

			tPointee( const void* addr, u32 dataSize, const Rtti::tReflector& reflector ) 
				: mAddress( addr ), mFullDataSize( dataSize ), mReflector( reflector ), mCid( Rtti::cInvalidClassId ), mOwned( false ), mFilePos( 0 ), mEndianSwappedPtrs( 128 ) { }

			inline b32 fContains( const void* addr, const u32 dataSize )
			{
				const byte* minAddr = ( const byte* )mAddress;
				const byte* maxAddr = ( const byte* )mAddress + mFullDataSize;
				return ( ( const byte* )addr >= minAddr ) && ( ( const byte* )addr + dataSize <= maxAddr );
			}

			inline b32 fContainsInDataBuffer( const void* addr, const u32 dataSize )
			{
				const byte* minAddr = ( const byte* )mDataBuffer.fBegin( );
				const byte* maxAddr = ( const byte* )mDataBuffer.fBegin( ) + mFullDataSize;
				return ( ( const byte* )addr >= minAddr ) && ( ( const byte* )addr + dataSize <= maxAddr );
			}

			inline u32 fComputeOffsetFromBase( const void* addr )
			{
				return ( u32 )( size_t( addr ) - size_t( mAddress ) );
			}

			void fConsume( tPointee* consumed );
			void fFixupEmbeddedPointers( tPointee* consumer, tPointee* consumed );
			void fEndianSwap( tPlatformId pid );
			void fEndianSwapObject( void* object, const Rtti::tReflector& reflector, tPlatformId pid, u32 count = 1 );
		};

		void fCopyData( 
			tPointee* ptee, 
			const void* object, 
			const Rtti::tReflector& reflector, 
			u32 numItems, 
			u32 realClassSize, 
			Rtti::tClassId realCid, 
			u32 fullDataSize,
			tPlatformId pid );

		///
		/// \brief Computes the "real" size and class id of the object at 'address'.
		///
		/// This means it determins what it's fully derived type is, if applicable,
		/// in order to determine these values; if not polymorphic, it just uses
		/// the values supplied by the class reflector.
		void fComputeRealClassSizeAndClassId( 
			u32& realClassSize, 
			Rtti::tClassId& realCid, 
			const void* address, 
			const Rtti::tReflector& reflector );

		///
		/// \brief Creates a new pointee object, encapsulating the data pointed to at 'address',
		/// and, if an array, continuing for 'numItems'.
		tPointee*	fCreatePointee( 
			const void* address, 
			const Rtti::tReflector& reflector, 
			u32 numItems,
			u32 realClassSize, 
			Rtti::tClassId realCid, 
			u32 fullDataSize,
			tPlatformId pid );

		u32			fGenerateAllPointees( const void* objectPtr, const Rtti::tReflector& reflector, tPlatformId pid );
		void		fCollectPointers( const void* object, const Rtti::tReflector& reflector, tPlatformId pid );
		void		fStorePointers( const void** object, const Rtti::tReflector& reflector, u32 numItems, b32 storePointer, tPlatformId pid );
		tPointee*	fFindPointee( const void* address, u32 dataSize );
		void		fErasePointee( u32 ip );
		void		fFixupEmbeddedPointers( tPointee* consumer, tPointee* consumed );
		void		fFixupOutputPointer( tPointee* ptee, tPointer& backptr, tPlatformId pid );
		void		fCleanup( );
	};

	template<class t, class tPutFunction>
	u32 tLoadInPlaceSerializer::fSave( const t& object, tPutFunction& output, tPlatformId pid )
	{
		sigassert( !fIsPointer<t>( ) );

		// walk all data connected to 'object', converting to 'pointees' and copied data blocks
		const u32 fileSize = fGenerateAllPointees( &object, *Rtti::fGetReflectorFromClass<t>( ), pid );

		// output each pointee's data block now that all the pointers have been fixed up
		for( u32 ip = 0; ip < mPointees.fCount( ); ++ip )
		{
			tPointee* ptee = mPointees[ip];
			output( ptee->mDataBuffer.fBegin( ), ptee->mDataBuffer.fCount( ) );
		}

		// cleanup, so that we're back in a state for this function to be called again
		fCleanup( );

		return fileSize;
	}
}


#endif//__tLoadInPlaceSerializer__

