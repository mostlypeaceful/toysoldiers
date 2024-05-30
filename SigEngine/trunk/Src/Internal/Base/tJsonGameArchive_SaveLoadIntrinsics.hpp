//------------------------------------------------------------------------------
// \file tJsonGameArchive_SaveLoadIntrinsics.hpp - 1 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tJsonGameArchive_SaveLoadIntrinsics__
#define __tJsonGameArchive_SaveLoadIntrinsics__

#include "tJsonGameArchiveLoader.hpp"
#include "tJsonGameArchiveSaver.hpp"
#include "tPeekingJsonTokenizer.hpp"

namespace Sig
{
	///	\class	tJsonGameArchive_SaveLoadIntrinsics
	///
	/// \brief	Default implementation of tJsonGameArchive_SaveLoadIntrinsics, used to implement tJsonGameArchive*.
	///
	///			Assumes t::fSaveLoad is defined and defers the majority of the operation to it.
	///			Adds array braces for structure.  A struct with three integers (de)serialized with this might look
	///			like: [1,2,3]
	///
	///			N.B. this type has many specializations!
	///
	template< class t > struct tJsonGameArchive_SaveLoadIntrinsics
	{
		/// \brief	Implements tJsonGameArchiveSaver::fSaveLoad( object ), you should probably not call this directly.
		static void fSave( tJsonGameArchiveSaver& archive, t& object )
		{
			sigcheckfail( archive.fWriter( ).fBeginArray( ), archive.fFail( ); return );
			object.fSaveLoad( archive );
			if( archive.fFailed( ) )
				return;
			sigcheckfail( archive.fWriter( ).fEndArray( ), archive.fFail( ); return );
		}
		
		/// \brief	Implements tJsonGameArchiveLoader::fSaveLoad( object ), you should probably not call this directly.
		static void fLoad( tJsonGameArchiveLoader& archive, t& object )
		{
			sigcheckfail( archive.fReader( ).fBeginArray( ), archive.fFail( ); return );
			object.fSaveLoad( archive );
			if( archive.fFailed( ) )
				return;
			sigcheckfail( archive.fReader( ).fEndArray( ), archive.fFail( ); return );
		}
	};


	/// \class	tJsonGameArchive_SaveLoadIntrinsics_Number
	///
	/// \brief	Base class for tJsonGameArchive_SaveLoadIntrinsics< ...integers... >.
	///			serializes C++ integers and floating point values as JSON numbers.
	template< class t > struct tJsonGameArchive_SaveLoadIntrinsics_Number
	{
		/// \brief	Implements tJsonGameArchiveSaver::fSaveLoad( number ), you should probably not call this directly.
		static void fSave( tJsonGameArchiveSaver& archive, t& number )
		{
			if( !archive.fWriter( ).fWriteValue( (f64)number ) )
				archive.fFail( );
		}
		
		/// \brief	Implements tJsonGameArchiveLoader::fSaveLoad( number ), you should probably not call this directly.
		static void fLoad( tJsonGameArchiveLoader& archive, t& number )
		{
			f64 value;
			if( !archive.fReader( ).fReadValue( value ) )
				archive.fFail( );
			else
				number = (t)value;
		}
	};


	// Specialize tJsonGameArchive_SaveLoadIntrinsics for <64bit integer types and floating point types.
	#define number( t ) template<> struct tJsonGameArchive_SaveLoadIntrinsics< t > : tJsonGameArchive_SaveLoadIntrinsics_Number< t > {}
	number( char );
	number( u8 );
	number( s8 );
	number( u16 );
	number( s16 );
	number( u32 );
	number( s32 );
	//number( u64 ); // XXX: number is a double, XJSON can't (de)serialize 64 bit integers without losing bits.
	//number( s64 ); // XXX: number is a double, XJSON can't (de)serialize 64 bit integers without losing bits.
	number( f32 );
	number( f64 );
	#undef number


	/// \class	tJsonGameArchive_SaveLoadIntrinsics< tDynamicArray< t > >
	///
	/// \brief	Specialization for (de)serializing tDynamicArray< ... >s.
	///			Specifies the count to minimize the number of reallocations or lookaheads required.  This might look like:
	///			{ "n": 3, "data": [ 1, 2, 3 ] }
	///
	template< class t > struct tJsonGameArchive_SaveLoadIntrinsics< tDynamicArray< t > >
	{
		/// \brief	Implements tJsonGameArchiveSaver::fSaveLoad( dynamicArray ), you should probably not call this directly.
		static void fSave( tJsonGameArchiveSaver& archive, tDynamicArray< t >& dynamicArray )
		{
			u32 n = dynamicArray.fCount( );
			sigcheckfail( archive.fWriter( ).fBeginObject( ),			archive.fFail( ); return );
			sigcheckfail( archive.fWriter( ).fWriteField( "n", n ),		archive.fFail( ); return );
			sigcheckfail( archive.fWriter( ).fBeginField( "data" ),		archive.fFail( ); return );
			sigcheckfail( archive.fWriter( ).fBeginArray( ),			archive.fFail( ); return );
			for( u32 i = 0; i < n; ++i )
			{
				archive.fSaveLoad( dynamicArray[ i ] );
				if( archive.fFailed( ) )
					return;
			}
			sigcheckfail( archive.fWriter( ).fEndArray( ),				archive.fFail( ); return );
			sigcheckfail( archive.fWriter( ).fEndField( ),				archive.fFail( ); return ); // end of "data" field
			sigcheckfail( archive.fWriter( ).fEndObject( ),				archive.fFail( ); return );
		}

		/// \brief	Implements tJsonGameArchiveLoader::fSaveLoad( dynamicArray ), you should probably not call this directly.
		static void fLoad( tJsonGameArchiveLoader& archive, tDynamicArray< t >& dynamicArray )
		{
			u32 n = 0;
			sigcheckfail( archive.fReader( ).fBeginObject( ),				archive.fFail( ); return );
			sigcheckfail( archive.fReader( ).fGetField( "n", n ),			archive.fFail( ); return );
			sigcheckfail( archive.fSanityCheckArrayAlloc( sizeof(t), n ),	archive.fFail( ); return );
			dynamicArray.fResize( n );
			sigcheckfail( archive.fReader( ).fGetField( "data" ),			archive.fFail( ); return );
			sigcheckfail( archive.fReader( ).fBeginArray( ),				archive.fFail( ); return );
			for( u32 i = 0; i < n; ++i )
			{
				archive.fSaveLoad( dynamicArray[ i ] );
				if( archive.fFailed( ) )
					return;
			}
			sigcheckfail( archive.fReader( ).fEndArray( ),					archive.fFail( ); return ); // end of "data" field
			sigcheckfail( archive.fReader( ).fEndObject( ),					archive.fFail( ); return );
		}
	};


	/// \class	tJsonGameArchive_SaveLoadIntrinsics< tGrowableArray< t > >
	///
	/// \brief	Specialization for (de)serializing tGrowableArray< ... >s.
	///			Specifies the count to minimize the number of reallocations or lookaheads required.  This might look like:
	///			{ "n": 3, "data": [ 1, 2, 3 ] }
	///
	template< class t > struct tJsonGameArchive_SaveLoadIntrinsics< tGrowableArray< t > >
	{
		/// \brief	Implements tJsonGameArchiveSaver::fSaveLoad( growableArray ), you should probably not call this directly.
		static void fSave( tJsonGameArchiveSaver& archive, tGrowableArray< t >& growableArray )
		{
			u32 n = growableArray.fCount( );
			sigcheckfail( archive.fWriter( ).fBeginObject( ),			archive.fFail( ); return );
			sigcheckfail( archive.fWriter( ).fWriteField( "n", n ),		archive.fFail( ); return );
			sigcheckfail( archive.fWriter( ).fBeginField( "data" ),		archive.fFail( ); return );
			sigcheckfail( archive.fWriter( ).fBeginArray( ),			archive.fFail( ); return );
			for( u32 i = 0; i < n; ++i )
			{
				archive.fSaveLoad( growableArray[ i ] );
				if( archive.fFailed( ) )
					return;
			}
			sigcheckfail( archive.fWriter( ).fEndArray( ),				archive.fFail( ); return );
			sigcheckfail( archive.fWriter( ).fEndField( ),				archive.fFail( ); return );
			sigcheckfail( archive.fWriter( ).fEndObject( ),				archive.fFail( ); return );
		}

		/// \brief	Implements tJsonGameArchiveLoader::fSaveLoad( growableArray ), you should probably not call this directly.
		static void fLoad( tJsonGameArchiveLoader& archive, tGrowableArray< t >& growableArray )
		{
			u32 n = 0;
			sigcheckfail( archive.fReader( ).fBeginObject( ),				archive.fFail( ); return );
			sigcheckfail( archive.fReader( ).fGetField( "n", n ),			archive.fFail( ); return );
			sigcheckfail( archive.fSanityCheckArrayAlloc( sizeof(t), n ),	archive.fFail( ); return );
			growableArray.fSetCount( n );
			sigcheckfail( archive.fReader( ).fGetField( "data" ),			archive.fFail( ); return );
			sigcheckfail( archive.fReader( ).fBeginArray( ),				archive.fFail( ); return );
			for( u32 i = 0; i < n; ++i )
			{
				archive.fSaveLoad( growableArray[ i ] );
				if( archive.fFailed( ) )
					return;
			}
			sigcheckfail( archive.fReader( ).fEndArray( ),					archive.fFail( ); return ); // end of "data" field
			sigcheckfail( archive.fReader( ).fEndObject( ),					archive.fFail( ); return );
		}
	};


	/// \class	tJsonGameArchive_SaveLoadIntrinsics< tFixedArray< t, fixedN > >
	///
	/// \brief	Specialization for (de)serializing tFixedArray< ... >s.
	///			Specifies the count to maintain compatibility with tDynamicArray or tGrowableArray s.  This might look like:
	///			{ "n": 3, "data": [ 1, 2, 3 ] }
	///
	template< class t, size_t fixedN > struct tJsonGameArchive_SaveLoadIntrinsics< tFixedArray< t, fixedN > >
	{
		/// \brief	Implements tJsonGameArchiveSaver::fSaveLoad( fixedArray ), you should probably not call this directly.
		static void fSave( tJsonGameArchiveSaver& archive, tFixedArray< t, fixedN >& fixedArray )
		{
			u32 n = fixedArray.fCount( );
			sigcheckfail( archive.fWriter( ).fBeginObject( ),			archive.fFail( ); return );
			sigcheckfail( archive.fWriter( ).fWriteField( "n", n ),		archive.fFail( ); return );
			sigcheckfail( archive.fWriter( ).fBeginField( "data" ),		archive.fFail( ); return );
			sigcheckfail( archive.fWriter( ).fBeginArray( ),			archive.fFail( ); return );
			for( u32 i = 0; i < n; ++i )
			{
				archive.fSaveLoad( fixedArray[ i ] );
				if( archive.fFailed( ) )
					return;
			}
			sigcheckfail( archive.fWriter( ).fEndArray( ),				archive.fFail( ); return );
			sigcheckfail( archive.fWriter( ).fEndField( ),				archive.fFail( ); return );
			sigcheckfail( archive.fWriter( ).fEndObject( ),				archive.fFail( ); return );
		}

		/// \brief	Implements tJsonGameArchiveLoader::fSaveLoad( fixedArray ), you should probably not call this directly.
		static void fLoad( tJsonGameArchiveLoader& archive, tFixedArray< t, fixedN >& fixedArray )
		{
			u32 n = 0;
			sigcheckfail( archive.fReader( ).fBeginObject( ),						archive.fFail( ); return );
			sigcheckfail( archive.fReader( ).fGetField( "n", n ),					archive.fFail( ); return );
			sigcheckfail( n <= fixedN,												archive.fFail( ); return );
			sigcheckfail( archive.fReader( ).fGetField( "data" ),					archive.fFail( ); return );
			sigcheckfail( archive.fReader( ).fBeginArray( ),						archive.fFail( ); return );
			for( u32 i = 0; i < n; ++i )
			{
				archive.fSaveLoad( fixedArray[ i ] );
				if( archive.fFailed( ) )
					return;
			}
			sigcheckfail( archive.fReader( ).fEndArray( ),							archive.fFail( ); return ); // end of "data" field
			sigcheckfail( archive.fReader( ).fEndObject( ),							archive.fFail( ); return );
		}
	};


	/// \class	tJsonGameArchive_SaveLoadIntrinsics< std::string >
	///
	/// \brief	Specialization for (de)serializing std::string.
	///			(De)serializes to/from a plain JSON string.  This might look like:
	///			"Please pardon my \"Air Quotes\""
	template<> struct tJsonGameArchive_SaveLoadIntrinsics< std::string >
	{
		/// \brief	Implements tJsonGameArchiveSaver::fSaveLoad( stdString ), you should probably not call this directly.
		static void fSave( tJsonGameArchiveSaver& archive, std::string& stdString )
		{
			if( !archive.fWriter( ).fWriteValue( stdString ) )
				archive.fFail( );
		}
		
		/// \brief	Implements tJsonGameArchiveLoader::fSaveLoad( stdString ), you should probably not call this directly.
		static void fLoad( tJsonGameArchiveLoader& archive, std::string& stdString )
		{
			if( !archive.fReader( ).fReadValue( stdString ) )
				archive.fFail( );
		}
	};


	/// \class	tJsonGameArchive_SaveLoadIntrinsics< tStringPtr >
	///
	/// \brief	Specialization for (de)serializing tStringPtr
	///			(De)serializes to/from a plain JSON string or JSON null literal.  This might look like:
	///			"Please pardon my \"Air Quotes\""
	template<> struct tJsonGameArchive_SaveLoadIntrinsics< tStringPtr >
	{
		/// \brief	Implements tJsonGameArchiveSaver::fSaveLoad( stringPtr ), you should probably not call this directly.
		static void fSave( tJsonGameArchiveSaver& archive, tStringPtr& stringPtr )
		{
			if( stringPtr == tStringPtr::cNullPtr )
				sigcheckfail( archive.fWriter( ).fWriteNullValue( ),		archive.fFail( ); return );
			else
				sigcheckfail( archive.fWriter( ).fWriteValue( stringPtr ),	archive.fFail( ); return );
		}
		
		/// \brief	Implements tJsonGameArchiveLoader::fSaveLoad( stringPtr ), you should probably not call this directly.
		static void fLoad( tJsonGameArchiveLoader& archive, tStringPtr& stringPtr )
		{
			const Json::tTokenType tt = archive.fReader( ).fPeekNextValueTokenType( );
			switch( tt )
			{
			case Json::cTokenNull:
				sigcheckfail( archive.fReader( ).fReadNullValue( ),			archive.fFail( ); return );
				stringPtr = tStringPtr::cNullPtr;
				return;
			case Json::cTokenString:
				sigcheckfail( archive.fReader( ).fReadValue( stringPtr ),	archive.fFail( ); return );
				return;
			default:
				sigcheckfail( !"tJsonGameArchive_SaveLoadIntrinsics< tStringPtr >::fLoad: Expected null or string token", archive.fFail( ) );
				return;
			}
		}
	};


	/// \class	tJsonGameArchive_SaveLoadIntrinsics< tFilePathPtr >
	///
	/// \brief	Specialization for (de)serializing std::string.
	///			(De)serializes to/from a plain JSON string or JSON null literal.  This might look like:
	///			"foo\\bar\\baz.pngb"
	template<> struct tJsonGameArchive_SaveLoadIntrinsics< tFilePathPtr >
	{
		/// \brief	Implements tJsonGameArchiveSaver::fSaveLoad( filePathPtr ), you should probably not call this directly.
		static void fSave( tJsonGameArchiveSaver& archive, tFilePathPtr& filePathPtr )
		{
			if( filePathPtr == tFilePathPtr::cNullPtr )
			{
				if( !archive.fWriter( ).fWriteNullValue( ) )
					archive.fFail( );
			}
			else
			{
				if( !archive.fWriter( ).fWriteValue( filePathPtr.fCStr( ) ) )
					archive.fFail( );
			}
		}
		
		/// \brief	Implements tJsonGameArchiveLoader::fSaveLoad( filePathPtr ), you should probably not call this directly.
		static void fLoad( tJsonGameArchiveLoader& archive, tFilePathPtr& filePathPtr )
		{
			const Json::tTokenType tt = archive.fReader( ).fPeekNextValueTokenType( );
			switch( tt )
			{
			case Json::cTokenNull:
				sigcheckfail( archive.fReader( ).fReadNullValue( ),		archive.fFail( ); return );
				filePathPtr = tFilePathPtr::cNullPtr;
				return;
			case Json::cTokenString:
				{
					std::string value;
					if( !archive.fReader( ).fReadValue( value ) )
						archive.fFail( );
					else
						filePathPtr = tFilePathPtr( value );
				}
				return;
			default:
				sigcheckfail( !"tJsonGameArchive_SaveLoadIntrinsics< tFilePathPtr >::fLoad: Expected null or string token", archive.fFail( ) );
				return;
			}
		}
	};


	/// \class	tJsonGameArchive_SaveLoadIntrinsics< tRefCounterPtr< t > >
	///
	/// \brief	Specialization for (de)serializing ref counted pointers to serializable types.
	///			Maps to a simple object with a field p representing the inner value.  Note there
	///			is NO built in support for deduplication, so multiple ref counted pointers referencing
	///			the same object will serialize the instance multiple times and deserialize as seperate
	///			instances.
	///
	///			Example object, given struct t : tRefCounter { int mMember; void fSaveLoad( ... ) { ... } }:
	///			{ p: [ 1 ] }
	///
	template< class t > struct tJsonGameArchive_SaveLoadIntrinsics< tRefCounterPtr< t > >
	{
		/// \brief	Implements tJsonGameArchiveSaver::fSaveLoad( refCounterPtr ), you should probably not call this directly.
		static void fSave( tJsonGameArchiveSaver& archive, tRefCounterPtr< t >& refCounterPtr )
		{
			sigcheckfail( archive.fWriter( ).fBeginObject( ),			archive.fFail( ); return );
			sigcheckfail( archive.fWriter( ).fBeginField( "p" ),		archive.fFail( ); return );
			if( t* raw = refCounterPtr.fGetRawPtr( ) )
			{
				archive.fSaveLoad( *raw );
			}
			else
			{
				sigcheckfail( archive.fWriter( ).fWriteNullValue( ),	archive.fFail( ); return );
			}
			sigcheckfail( archive.fWriter( ).fEndField( ),				archive.fFail( ); return );
			sigcheckfail( archive.fWriter( ).fEndObject( ),				archive.fFail( ); return );
		}

		/// \brief	Implements tJsonGameArchiveLoader::fSaveLoad( refCounterPtr ), you should probably not call this directly.
		static void fLoad( tJsonGameArchiveLoader& archive, tRefCounterPtr< t >& refCounterPtr )
		{
			sigcheckfail( archive.fReader( ).fBeginObject( ),			archive.fFail( ); return );
			sigcheckfail( archive.fReader( ).fGetField( "p" ),			archive.fFail( ); return );
			const Json::tTokenType tt = archive.fReader( ).fPeekNextValueTokenType( );
			switch( tt )
			{
			case Json::cTokenInvalid:
				archive.fFail( );
				break;
			case Json::cTokenNull:
				sigcheckfail( archive.fReader( ).fReadNullValue( ),		archive.fFail( ); return );
				refCounterPtr.fRelease( );
				break;
			default:
				refCounterPtr.fReset( NEW_TYPED( t )( ) );
				archive.fSaveLoad( *refCounterPtr );
				if( archive.fFailed( ) )
					return;
				break;
			}
			sigcheckfail( archive.fReader( ).fEndObject( ),				archive.fFail( ); return );
		}
	};
}

#endif //ndef __tJsonGameArchive_SaveLoadIntrinsics__
