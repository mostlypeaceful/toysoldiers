#include "BasePch.hpp"
#include "tLoadInPlaceDeserializer.hpp"

namespace Sig
{
	void tLoadInPlaceDeserializer::fRelocatePointers( void* object, const Rtti::tReflector& reflectorBase, b32 alreadyNewed )
	{
		const Rtti::tReflector* reflector = &reflectorBase;

		b32 skipNewingChildren = alreadyNewed;

		if( !alreadyNewed && reflectorBase.fIsPolymorphic( ) )
		{
			// we have to call the proper operator 'new' on the class to initialize the vtable
			Rtti::fNewClassInPlace( *( Rtti::tClassId* )object, object, true );

			// cast to serializable base class
			const Rtti::tSerializableBaseClass* baseClass = fCastToSerializableBaseClass( object, *reflector );

			// now get the proper derived reflector object
			reflector = &baseClass->fGetDerivedReflector( );

			// once a parent class has been 'newed', we don't want to new anything inside it
			skipNewingChildren = true;
		}

		for( const Rtti::tBaseClassDesc* i = reflector->fBaseClasses( ); i && !i->fNull( ); ++i )
		{
			// recurse on base classes
			sigassert( i->fReflector( ) );
			fRelocatePointers( i->fComputeAddress( object ), *i->fReflector( ), skipNewingChildren );
		}

		for( const Rtti::tClassMemberDesc* i = reflector->fClassMembers( ); i && !i->fNull( ); ++i )
		{
			const u32 numItems = ( u32 )i->fComputeArrayCount( object );

			if( i->fIsPointer( ) )
			{
				// send to fFixupPointersInternal
				fFixupPointers( ( void** )i->fComputeAddress( object ), *i->fReflector( ), numItems );
			}
			else if( !i->fIsBuiltIn( ) )
			{
				Sig::byte* address = ( Sig::byte* )i->fComputeAddress( object );
				for( u32 arraySlot = 0; arraySlot < numItems; ++arraySlot )
				{
					// recurse on non-built-in-type members
					sigassert( i->fReflector( ) );
					fRelocatePointers( address, *i->fReflector( ), skipNewingChildren );

					sigassert( numItems == 1 || !i->fReflector( )->fIsPolymorphic( ) );
					address += i->fReflector( )->fClassSizeof( );
				}
			}
		}

	}

	void tLoadInPlaceDeserializer::fFixupPointers( void** address, const Rtti::tReflector& reflector, u32 numItems )
	{
		if( !address || !*address )
			return;

		const u32 offset = *( u32* )address;
		const b32 firstRelocate = ( offset & tLoadInPlaceSerializerBase::cPointerOwnershipFlag ) ? true : false;
		*address = ( void* )( mBaseAddress + ( offset & ~tLoadInPlaceSerializerBase::cPointerOwnershipFlag ) );

		if( firstRelocate && reflector.fContainsPointers( ) )
		{
			sigassert( numItems == 1 || !reflector.fIsPolymorphic( ) );

			for( u32 i = 0; i < numItems; ++i )
			{
				//log_line( Log::cFlagResource, "Relocating [" << reflector.fTypeName( ) << "]" );
				fRelocatePointers( ( byte* )*address + i * reflector.fClassSizeof( ), reflector, false );
			}
		}
	}

}

