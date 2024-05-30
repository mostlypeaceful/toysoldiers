//------------------------------------------------------------------------------
// \file tModuleHelper.cpp - 31 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tModuleHelper.hpp"
#include "EndianUtil.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	template< class t >
	void fWrite( tGrowableArray< byte > & buffer, const t * objects, u32 count )
	{
		sigassert( tIsBuiltInType< t >::cIs );
		buffer.fInsert( buffer.fCount( ), ( const byte * )objects, count * sizeof( t ) );
	}

	//------------------------------------------------------------------------------
	template< class t >
	void fWrite( tGrowableArray< byte > & buffer, const t & obj )
	{
		fWrite( buffer, &obj, 1 );
	}

	//------------------------------------------------------------------------------
	template< >
	void fWrite(  tGrowableArray< byte > & buffer, const std::string & str )
	{
		u32 len = str.length( );
		fWrite( buffer, len );
		fWrite( buffer, str.c_str( ), len );
	}

	//------------------------------------------------------------------------------
	template< >
	void fWrite( tGrowableArray< byte > & buffer, const tFilePathPtr & path )
	{
		u32 len = path.fLength( );
		fWrite( buffer, len );
		fWrite( buffer, path.fCStr( ), len );
	}

	//------------------------------------------------------------------------------
	template< class t >
	const byte * fRead( tPlatformId source, const byte * data, u32 dataLength, t * objects, u32 count )
	{
		sigassert( dataLength >= sizeof( t ) * count );
		fMemCpy( objects, data, count * sizeof( t ) );
		EndianUtil::fSwapForTargetPlatform( objects, sizeof( t ), source, count );
		
		return data + count * sizeof( t );
	}

	//------------------------------------------------------------------------------
	template< class t >
	const byte * fRead( tPlatformId source, const byte * data, u32 dataLength, t & obj )
	{
		sigassert( tIsBuiltInType< t >::cIs );
		return fRead( source, data, dataLength, &obj, 1 );
	}

	//------------------------------------------------------------------------------
	template< >
	const byte * fRead( tPlatformId source, const byte * data, u32 dataLength, std::string & str )
	{
		const byte * term = data + dataLength;

		u32 len;
		data = fRead( source, data, term - data, len );

		malloca_array( char, mem, len );
		data = fRead( source, data, term - data, mem.fBegin(), len );

		str = std::string( mem.fBegin(), mem.fEnd() );
		return data;
	}

	//------------------------------------------------------------------------------
	template< >
	const byte * fRead( tPlatformId source, const byte * data, u32 dataLength, tFilePathPtr & path )
	{
		const byte * term = data + dataLength;

		u32 len;
		data = fRead( source, data, term - data, len );

		malloca_array( char, mem, len );
		data = fRead( source, data, term - data, mem.fBegin(), len );

		path = tFilePathPtr( std::string( mem.fBegin(), mem.fEnd() ) );
		return data;
	}

	//------------------------------------------------------------------------------
	// tModuleHelper
	//------------------------------------------------------------------------------
	tModuleHelper::tModuleHelper( b32 refresh )
	{
		if( refresh )
			fRefreshModules( );
	}

	//------------------------------------------------------------------------------
	void tModuleHelper::fSave( tGrowableArray< byte > & buffer )
	{
		// Store the platform in a uniform endianess so it can be read by any platform
		u32 currentPlatform = ( u32 )cCurrentPlatform;
		EndianUtil::fSwapForTargetPlatform( &currentPlatform, sizeof( currentPlatform ), cPlatformPcDx9 );
		fWrite( buffer, currentPlatform );

		const u32 moduleCount = mModules.fCount( );
		fWrite( buffer, moduleCount );

		for( u32 m = 0; m < moduleCount; ++m )
		{
			const tModule & mod = mModules[ m ];
			fWrite( buffer, mod.mName );

			u64 baseAddress = (u64)mod.mBaseAddress;
			fWrite( buffer, (u64)baseAddress );

			fWrite( buffer, mod.mSize );
			fWrite( buffer, mod.mTimeStamp );
			fWrite( buffer, mod.mSymbolsSignature.mGuid.D1 );
			fWrite( buffer, mod.mSymbolsSignature.mGuid.D2 );
			fWrite( buffer, mod.mSymbolsSignature.mGuid.D3 );
			fWrite( buffer, mod.mSymbolsSignature.mGuid.D4.fBegin( ), mod.mSymbolsSignature.mGuid.D4.fCount( ) );
			fWrite( buffer, mod.mSymbolsSignature.mAge );
			fWrite( buffer, mod.mSymbolsSignature.mPath );
		}
	}

	//------------------------------------------------------------------------------
	const byte * tModuleHelper::fLoad( const byte * data, u32 dataLength )
	{
		const byte * term = data + dataLength;

		tPlatformId sourcePlatform;
		{
			u32 spU32;
			data = fRead( cPlatformPcDx9, data, term - data, spU32 );
			sourcePlatform = ( tPlatformId )spU32;
		}

		u32 moduleCount;
		data = fRead( sourcePlatform, data, term - data, moduleCount );
		mModules.fNewArray( moduleCount );

		for( u32 m = 0; m < moduleCount; ++m )
		{
			tModule & mod = mModules[ m ];

			data = fRead( sourcePlatform, data, term - data, mod.mName );
			
			u64 baseAddress;
			data = fRead( sourcePlatform, data, term - data, baseAddress );
			mod.mBaseAddress = ( void * )baseAddress;

			data = fRead( sourcePlatform, data, term - data, mod.mSize );
			data = fRead( sourcePlatform, data, term - data, mod.mTimeStamp );
			data = fRead( sourcePlatform, data, term - data, mod.mSymbolsSignature.mGuid.D1 );
			data = fRead( sourcePlatform, data, term - data, mod.mSymbolsSignature.mGuid.D2 );
			data = fRead( sourcePlatform, data, term - data, mod.mSymbolsSignature.mGuid.D3 );
			data = fRead( 
				sourcePlatform, data, term - data, 
				mod.mSymbolsSignature.mGuid.D4.fBegin( ), 
				mod.mSymbolsSignature.mGuid.D4.fCount( ) );
			data = fRead( sourcePlatform, data, term - data, mod.mSymbolsSignature.mAge );
			data = fRead( sourcePlatform, data, term - data, mod.mSymbolsSignature.mPath );
		}

		return data;
	}
}
