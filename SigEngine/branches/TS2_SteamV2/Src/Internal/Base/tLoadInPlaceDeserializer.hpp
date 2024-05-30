#ifndef __tLoadInPlaceDeserializer__
#define __tLoadInPlaceDeserializer__
#include "tLoadInPlaceSerializerBase.hpp"

namespace Sig
{

	///
	/// \brief Serializes classes into a buffer that have saved using the tLoadInPlaceSerializer class.
	///
	/// The loading is very fast, using load-in-place conventions; the only real operations going
	/// on are pointer fixups and vtable initializations (if and where appropriate). As the last
	/// note implies, this class can properly handle classes with vtables, as long as they inherit
	/// from Rtti::tSerializableBaseClass; additionally, you must ensure that the statically bound
	/// class ids are unique.
	class base_export tLoadInPlaceDeserializer : public tLoadInPlaceSerializerBase
	{
		byte* mBaseAddress;

	public:

		tLoadInPlaceDeserializer( ) : mBaseAddress( 0 ) { }

		///
		/// \brief Loads an object that has been serialized out to the supplied data buffer. 
		///
		/// \note The object and all its pointed to data should already reside within 'dataBuffer'.
		///
		/// Usually, you will save a class to a file (using tLoadInPlaceSerializer), and then load it back
		/// in using a single allocation (using the file size to initialize the buffer); you would
		/// then pass that buffer to this method, in order to do all the pointer fixups and
		/// vtable initializations.
		template<class t>
		void fLoad( void* loadInPlaceBuffer );

	private:

		void fRelocatePointers( void* object, const Rtti::tReflector& reflector, b32 alreadyNewed );
		void fFixupPointers( void** address, const Rtti::tReflector& reflector, u32 numItems );
	};

	template<class t>
	void tLoadInPlaceDeserializer::fLoad( void* loadInPlaceBuffer )
	{
		mBaseAddress = ( byte* )loadInPlaceBuffer - 4;

		void* object = ( void* )( size_t )( tLoadInPlaceSerializerBase::cPointerOwnershipFlag + 4u );

		const Rtti::tReflector* reflector = Rtti::fGetReflectorFromClass<t>( );
		sigassert( reflector );

		fFixupPointers( &object, *reflector, 1 );

		mBaseAddress = 0;
	}

	template<class tLipType>
	b32 fLoadInPlaceLoadCallback( tResource& resource, tGenericBuffer* buffer )
	{
		sigassert( can_convert( tLipType, tLoadInPlaceFileBase ) );

#if sig_resource_logging
		Time::tStopWatch timer;
		if( tResource::fLoggingEnabled( ) )
			log_output( Log::cFlagResource, "Resource Load Callback for [" << resource.fGetPath( ) << "]... " );
#endif//sig_resource_logging

		tLoadInPlaceDeserializer des;
		des.fLoad<tLipType>( buffer->fGetBuffer( ) );

		tLoadInPlaceFileBase* lipfile = ( ( tLoadInPlaceFileBase* )buffer->fGetBuffer( ) );
		if( !lipfile->fVerifyResource( resource, &tResourceConvertPath<tLipType>::fConvertToSource, tLipType::cVersion, Rtti::fGetClassId<tLipType>( ) ) )
		{
			buffer->fFree( );
			return false;
		}
		lipfile->fInitializeLoadInPlaceTables( );
		if( !lipfile->fLoadSubResources( resource ) )
		{
			lipfile->fUnloadSubResources( resource );
			lipfile->fCleanUpLoadInPlaceTables( );
			buffer->fFree( );
			return false;
		}
		lipfile->fOnFileLoaded( resource );
		lipfile->fResizeAfterLoad( buffer );

#if sig_resource_logging
		if( tResource::fLoggingEnabled( ) )
			log_line( Log::cFlagResource, "complete (success) - took " << timer.fGetElapsedMs( ) << " ms." );
#endif//sig_resource_logging

		return true;
	}
	template<class tLipType>
	b32 fLoadInPlaceUnloadCallback( tResource& resource, tGenericBuffer* buffer )
	{
		sigassert( can_convert( tLipType, tLoadInPlaceFileBase ) );

		tLoadInPlaceFileBase* lipfile = ( ( tLoadInPlaceFileBase* )buffer->fGetBuffer( ) );
		lipfile->fOnFileUnloading( );
		lipfile->fUnloadSubResources( resource );
		lipfile->fCleanUpLoadInPlaceTables( );

		return true;
	}
	template<class tLipType>
	b32 fLoadInPlaceIsSubResourceCallback( const tResource& primary, const tResource& sub, tResource::tVisitedList& visited )
	{
		sigassert( can_convert( tLipType, tLoadInPlaceFileBase ) );

		return ( ( tLoadInPlaceFileBase* )primary.fGetResourceBuffer( ) )->fIsSubResource( sub, visited );
	}
	template<class tLipType>
	void fLoadInPlaceGatherSubResourcesCallback( const tResource& resource, tResourcePtrList& subs )
	{
		sigassert( can_convert( tLipType, tLoadInPlaceFileBase ) );

		( ( const tLoadInPlaceFileBase* )resource.fGetResourceBuffer( ) )->fGatherSubResources( subs );
	}


	///
	/// \brief Here's a pretty grody macro that's pretty handy when implementing
	/// a new Load-In-Place type;
#	define implement_lip_load_unload_callbacks( tLipType ) \
		namespace \
		{ \
			register_rtti_factory( tLipType, true ); \
			define_static_function( fRegister##tLipType##LoadUnloadCallbacks ) \
			{ \
				tResource::tTypeSpecificProperties props; \
				props.mPostLoad				= &fLoadInPlaceLoadCallback<tLipType>; \
				props.mPreDestroy			= &fLoadInPlaceUnloadCallback<tLipType>; \
				props.mIsSubResource		= &fLoadInPlaceIsSubResourceCallback<tLipType>; \
				props.mGatherSubResources	= &fLoadInPlaceGatherSubResourcesCallback<tLipType>; \
				tResource::tTypeSpecificPropertiesTable::fInstance( ).fInsert( Rtti::fGetClassId<tLipType>( ), props ); \
			} \
		}
}

#endif//__tLoadInPlaceDeserializer__
