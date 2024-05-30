#include "ToolsPch.hpp"
#include "tLoadInPlaceSerializer.hpp"
#include "EndianUtil.hpp"
#include "tPlatform.hpp"

namespace Sig
{
	const Rtti::tReflector* fGetReflector( const void* object, const Rtti::tReflector* baseReflector )
	{
		if( object && baseReflector->fIsPolymorphic( ) )
		{
			// the base pointer type is polymorphic, we need to get the derived reflector
			const Rtti::tSerializableBaseClass* realObject = tLoadInPlaceSerializerBase::fCastToSerializableBaseClass( object, *baseReflector );
			return &realObject->fGetDerivedReflector( );
		}

		return baseReflector;
	}

	///
	/// \section tLoadInPlaceSerializer
	///

	void tLoadInPlaceSerializer::fCopyData( 
		tLoadInPlaceSerializer::tPointee* ptee, 
		const void* object, 
		const Rtti::tReflector& reflector, 
		u32 numItems, 
		u32 realClassSize, 
		Rtti::tClassId realCid, 
		u32 fullDataSize,
		tPlatformId pid )
	{
		static_assert( sizeof(realCid)==sizeof(void*) );

		// we must have only one item, or else not be polymorphic:
		// i.e., if we are polymorphic, we only allow one item in a row
		sigassert( numItems == 1 || !reflector.fIsPolymorphic( ) );

		// either way, we copy all the data into the data buffer
		const u32 alignment = fullDataSize > 1024 ? 16 : 8;
		const u32 alignedFullDataSize = fAlignHigh( fullDataSize, alignment );
		ptee->mCid = realCid;
		ptee->mDataBuffer.fNewArray( alignedFullDataSize );
		//fZeroOut( ptee->mDataBuffer.fBegin( ) + fullDataSize, alignedFullDataSize - fullDataSize );
		fZeroOut( ptee->mDataBuffer.fBegin( ), ptee->mDataBuffer.fCount( ) );
		fMemCpy( ptee->mDataBuffer.fBegin( ), object, fullDataSize );

		if( reflector.fIsPolymorphic( ) )
		{
			// stuff class id into vtable pointer
			fMemCpy( ptee->mDataBuffer.fBegin( ), &realCid, sizeof(realCid) );
		}
	}

	void tLoadInPlaceSerializer::fComputeRealClassSizeAndClassId( 
		u32& realClassSize, 
		Rtti::tClassId& realCid, 
		const void* address, 
		const Rtti::tReflector& reflector )
	{
		if( reflector.fIsPolymorphic( ) )
		{
			const Rtti::tSerializableBaseClass* baseClass = fCastToSerializableBaseClass( address, reflector );
			sigassert( baseClass );
			realClassSize = baseClass->fClassSizeof( );
			realCid = baseClass->fClassId( );
		}
		else
		{
			realClassSize = reflector.fClassSizeof( );
			realCid = reflector.fClassId( );
		}

		sigassert( realClassSize < 4 || fIsEven( realClassSize ) );
	}

	tLoadInPlaceSerializer::tPointee* tLoadInPlaceSerializer::fCreatePointee( 
		const void* address, 
		const Rtti::tReflector& reflector, 
		u32 numItems,
		u32 realClassSize, 
		Rtti::tClassId realCid, 
		u32 fullDataSize,
		tPlatformId pid )
	{
		tPointee* ptee = new tPointee( address, fullDataSize, reflector );

		fCopyData( ptee, address, reflector, numItems, realClassSize, realCid, fullDataSize, pid );

		// we need to see if any existing pointees are actually contained within this new pointee;
		// if so, we let the new pointee consume the contained pointees, which means fixing up
		// back pointers and finally erasing the pointee itself
		for( u32 ip = 0; ip < mPointees.fCount( ); ++ip )
		{
			tPointee* consumed = mPointees[ip];
			if( !ptee->fContains( consumed->mAddress, consumed->mFullDataSize ) )
				continue;

			ptee->fConsume( consumed );

			mPointees.fErase( ip );

			fFixupEmbeddedPointers( ptee, consumed );

			delete consumed;
			--ip;
		}

		mPointees.fPushBack( ptee );

		return ptee;
	}

	u32 tLoadInPlaceSerializer::fGenerateAllPointees( const void* objectPtr, const Rtti::tReflector& reflector, tPlatformId pid )
	{
		// collect all pointers recursively from root object

		fStorePointers( &objectPtr, reflector, 1, false, pid );

		// compute the stream position corresponding to each output pointee

		u32 fileSize = 0;

		for( u32 ip = 0; ip < mPointees.fCount( ); ++ip )
		{
			tPointee* ptee = mPointees[ip];
			ptee->mFilePos = fileSize;
			fileSize += ptee->mDataBuffer.fCount( );
		}

		// all primary data conversion is complete: endian swap all built-in types within the pointees' data blocks

		if( fPlatformNeedsEndianSwap( cCurrentPlatform, pid ) )
		{
			for( u32 ip = 0; ip < mPointees.fCount( ); ++ip )
			{
				tPointee* ptee = mPointees[ip];
				ptee->fEndianSwap( pid );
			}
		}

		// go through and fixup all back pointers in each pointee

		for( u32 ip = 0; ip < mPointees.fCount( ); ++ip )
		{
			tPointee* ptee = mPointees[ip];

			if( ip==0 )
				ptee->mOwned = true;

			for( std::list<tPointer>::iterator	backptr  = ptee->mEmbeddedPtrs.begin( );
												backptr != ptee->mEmbeddedPtrs.end( );
											  ++backptr )
			{
				fFixupOutputPointer( ptee, *backptr, pid );
			}
		}

		return fileSize;
	}

	void tLoadInPlaceSerializer::fCollectPointers( const void* object, const Rtti::tReflector& reflector, tPlatformId pid )
	{
		for( const Rtti::tBaseClassDesc* i = reflector.fBaseClasses( ); i && !i->fNull( ); ++i )
		{
			// recurse on base classes
			sigassert( i->fReflector( ) );
			fCollectPointers( i->fComputeAddress( object ), *i->fReflector( ), pid );
		}

		for( const Rtti::tClassMemberDesc* i = reflector.fClassMembers( ); i && !i->fNull( ); ++i )
		{
			const u32 numItems = ( u32 )i->fComputeArrayCount( object );

			if( i->fIsPointer( ) )
			{
				// send to fStorePointers
				const void** memberPtrAddr = ( const void** )i->fComputeAddress( object );
				fStorePointers( memberPtrAddr, *fGetReflector( *memberPtrAddr, i->fReflector( ) ), numItems, true, pid );
			}
			else if( !i->fIsBuiltIn( ) )
			{
				Sig::byte* address = ( Sig::byte* )i->fComputeAddress( object );
				for( u32 arraySlot = 0; arraySlot < numItems; ++arraySlot )
				{
					// recurse on non-built-in-type members
					sigassert( i->fReflector( ) );
					fCollectPointers( address, *i->fReflector( ), pid );

					sigassert( numItems == 1 || !i->fReflector( )->fIsPolymorphic( ) );
					address += i->fReflector( )->fClassSizeof( );
				}
			}
		}
	}

	void tLoadInPlaceSerializer::fStorePointers( 
		const void** address, 
		const Rtti::tReflector& reflector, 
		u32 numItems, 
		b32 storePointer,
		tPlatformId pid )
	{
		if( !address || !*address )
			return;

		u32 realClassSize = 0;
		Rtti::tClassId realCid = Rtti::cInvalidClassId;
		fComputeRealClassSizeAndClassId( realClassSize, realCid, *address, reflector );

		const u32 fullDataSize = realClassSize * numItems;

		// see if we already have an object representing what is being pointed to
		tPointee* ptee = fFindPointee( *address, fullDataSize );
		const b32 newPtee = ptee ? false : true;
		if( !ptee )
			ptee = fCreatePointee( *address, reflector, numItems, realClassSize, realCid, fullDataSize, pid );

		if( storePointer )
		{
			tPointee* containingPointee = fFindPointee( address, sizeof(void*) );
			sigassert( containingPointee );

			const u32 o0 = ptee->fComputeOffsetFromBase( *address );
			const u32 o1 = containingPointee->fComputeOffsetFromBase( address );

			containingPointee->mEmbeddedPtrs.push_back( tPointer( realCid, ptee, o0, o1, &reflector ) );
		}

		if( newPtee && ( reflector.fBaseClasses( ) || reflector.fClassMembers( ) ) )
		{
			for( u32 isub = 0; isub < numItems; ++isub )
			{
				fCollectPointers( ( const byte* )*address + isub * realClassSize, reflector, pid );
			}
		}
	}

	tLoadInPlaceSerializer::tPointee* tLoadInPlaceSerializer::fFindPointee( const void* address, u32 dataSize )
	{
		for( u32 ip = 0; ip < mPointees.fCount( ); ++ip )
		{
			if( mPointees[ip]->fContains( address, dataSize ) )
				return mPointees[ip];
		}

		return 0;
	}

	void tLoadInPlaceSerializer::fErasePointee( u32 ip )
	{
		delete mPointees[ip];
		mPointees.fErase( ip );
	}

	void tLoadInPlaceSerializer::fFixupEmbeddedPointers( tPointee* consumer, tPointee* consumed )
	{
		for( u32 ip = 0; ip < mPointees.fCount( ); ++ip )
			mPointees[ip]->fFixupEmbeddedPointers( consumer, consumed );
	}

	void tLoadInPlaceSerializer::fFixupOutputPointer( tPointee* ptee, tPointer& backptr, tPlatformId pid )
	{
		u32* p = ( u32* ) &ptee->mDataBuffer[ backptr.mOffsetToPtr ];

		*p = backptr.mTarget->mFilePos + backptr.mOffsetFromPointeeBase + 4;

		if( backptr.mOffsetFromPointeeBase == 0 && !backptr.mTarget->mOwned )
		{
			backptr.mTarget->mOwned = true;
			*p |= tLoadInPlaceSerializerBase::cPointerOwnershipFlag;
		}

		EndianUtil::fSwapForTargetPlatform( p, sizeof(*p), pid );
	}

	void tLoadInPlaceSerializer::fCleanup( )
	{
		while( mPointees.fCount( ) > 0 )
			fErasePointee( ( u32 )mPointees.fCount( )-1 );
	}


	///
	/// \section tLoadInPlaceSerializer::tPointee
	///

	void tLoadInPlaceSerializer::tPointee::fConsume( tPointee* consumed )
	{
		// run through all the back-pointers that were pointing to the pointee we're about to consume;
		// adjust each offset so that it is now relative to the new base address (on 'this')

		const u32 offset = fComputeOffsetFromBase( consumed->mAddress );

		for( std::list<tPointer>::iterator	oldpointer  = consumed->mEmbeddedPtrs.begin( );
											oldpointer != consumed->mEmbeddedPtrs.end( );
										  ++oldpointer )
		{
			mEmbeddedPtrs.push_back( 
				tPointer(	oldpointer->mCid,
							oldpointer->mTarget,
							oldpointer->mOffsetFromPointeeBase,
							oldpointer->mOffsetToPtr + offset,
							oldpointer->mContext ) );
		}

		fFixupEmbeddedPointers( this, consumed );
	}

	void tLoadInPlaceSerializer::tPointee::fFixupEmbeddedPointers( tPointee* consumer, tPointee* consumed )
	{
		const u32 offset = consumer->fComputeOffsetFromBase( consumed->mAddress );

		for( std::list<tPointer>::iterator	backptr  = mEmbeddedPtrs.begin( );
											backptr != mEmbeddedPtrs.end( );
											++backptr )
		{

			if( backptr->mTarget != consumed )
				continue;

			backptr->mOffsetFromPointeeBase	+= offset;
			backptr->mTarget				 = consumer;
		}
	}

	void tLoadInPlaceSerializer::tPointee::fEndianSwap( tPlatformId pid )
	{
		static_assert( sizeof(mCid)==sizeof(void*) );

		const Rtti::tReflector& reflector = mReflector;//*fGetReflector( mDataBuffer.fBegin( ), &mReflector );

		// now that we've safely copied the data, we need to endian-swap the copied buffer
		const u32 numObjs = mFullDataSize / reflector.fClassSizeof( );

		// For built in types we can endian swap here and save a lot of hassle
		if( reflector.fIsBuiltIn( ) )
		{
			EndianUtil::fSwapForTargetPlatform( mDataBuffer.fBegin( ), reflector.fClassSizeof( ), pid, numObjs );
			mEndianSwappedPtrs.fInsert( mDataBuffer.fBegin( ), true );
			return;
		}

		for( u32 i = 0; i < numObjs; ++i )
		{
			const u32 offset = i * reflector.fClassSizeof( );

			fEndianSwapObject( mDataBuffer.fBegin( ) + offset, reflector, pid );

			if( reflector.fIsPolymorphic( ) )
			{
				Sig::byte* cidToSwap = mDataBuffer.fBegin( ) + offset;
				if( !mEndianSwappedPtrs.fFind( cidToSwap ) )
				{
					// endian swap the class id
					EndianUtil::fSwapForTargetPlatform( cidToSwap, sizeof(mCid), pid );
					mEndianSwappedPtrs.fInsert( cidToSwap, true );
				}
			}
		}
	}

	void tLoadInPlaceSerializer::tPointee::fEndianSwapObject( void* object, const Rtti::tReflector& reflector, tPlatformId pid, u32 count )
	{
		// we endian swap the entire object by iterating over all base classes, members,
		// and recursing; when we hit a built-in type, we actually do the real endian-swapping
		// and return, thus terminating the recursion

		// we check if the object itself is a built-in type; this is the termination condition
		if( reflector.fIsBuiltIn( ) )
		{
			if( fContainsInDataBuffer( object, reflector.fClassSizeof( ) * count ) && !mEndianSwappedPtrs.fFind( object ) )
			{
				mEndianSwappedPtrs.fInsert( object, true );

				// we've arrived at a built-in type, so endian swap
				EndianUtil::fSwapForTargetPlatform( object, reflector.fClassSizeof( ), pid, count );
			}

			return;
		}

		// iterate over base classes
		for( const Rtti::tBaseClassDesc* i = reflector.fBaseClasses( ); i && !i->fNull( ); ++i )
		{
			// recurse on base classes
			sigassert( i->fReflector( ) );
			fEndianSwapObject( i->fComputeAddress( object ), *i->fReflector( ), pid );
		}

		// iterate over members
		for( const Rtti::tClassMemberDesc* i = reflector.fClassMembers( ); i && !i->fNull( ); ++i )
		{
			const u32 numItems = ( u32 )i->fComputeArrayCount( object );
			if( i->fIsPointer( ) )
			{
				sigassert( i->fReflector( ) );

				// we ignore the pointer variable itself (these get endian-swapped later, after they've been fixed up),
				// but we still need to endian-swap the data pointed to
				Sig::byte* address = ( Sig::byte* )*( const void** )i->fComputeAddress( object );
				if( fContainsInDataBuffer( address, 1 ) )
				{
					if( i->fReflector( )->fIsBuiltIn( ) )
					{
						fEndianSwapObject( address, *i->fReflector( ), pid, numItems );
					}
					else
					{
						for( u32 arraySlot = 0; arraySlot < numItems; ++arraySlot )
						{
							// recurse on both built-in and non-built-in-types
							fEndianSwapObject( address, *fGetReflector( address, i->fReflector( ) ), pid );
							address += i->fReflector( )->fClassSizeof( );
						}
					}
				}
			}
			else
			{
				sigassert( i->fReflector( ) );

				Sig::byte* address = ( Sig::byte* )i->fComputeAddress( object );
				if( i->fReflector( )->fIsBuiltIn( ) )
				{
					fEndianSwapObject( address, *i->fReflector( ), pid, numItems );
				}
				else
				{
					for( u32 arraySlot = 0; arraySlot < numItems; ++arraySlot )
					{
						// recurse on both built-in and non-built-in-types
						fEndianSwapObject( address, *i->fReflector( ), pid );

						sigassert( numItems == 1 || !i->fReflector( )->fIsPolymorphic( ) );
						address += i->fReflector( )->fClassSizeof( );
					}
				}
			}
		}
	}

}
