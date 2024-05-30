//------------------------------------------------------------------------------
// \file tSymbolHelper_pc.cpp - 21 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "tSymbolHelper.hpp"
#if defined( sig_use_symbolhelper )

#include <DbgHelp.h>
#pragma comment( lib, "dbghelp.lib" )

namespace Sig
{
	static u64 gProcessHandle = 1;

	//------------------------------------------------------------------------------
	tSymbolHelper::tSymbolHelper( const char * symbolSearchPath, b32 currentProcess )
		: mProcessHandle( 0 )
	{
		if( currentProcess )
			mProcessHandle = (u64)GetCurrentProcess( );
		else
			mProcessHandle = gProcessHandle++;

		// Enable DbgHelp debug messages, make sure that DbgHelp only loads symbols that
		// exactly match, and do deferred symbol loading for greater efficiency.
		SymSetOptions( SYMOPT_DEBUG | SYMOPT_EXACT_SYMBOLS | SYMOPT_DEFERRED_LOADS );

		BOOL result = SymInitialize( ( HANDLE )mProcessHandle, symbolSearchPath, FALSE );
		if( !result )
			log_warning( 0, "tSymbolHelper::tSymbolHelper - SymInitialize failed: " << symbolSearchPath );
	}

	//------------------------------------------------------------------------------
	tSymbolHelper::~tSymbolHelper( )
	{
		const u32 modCount = mModules.fCount( );
		for( u32 m = 0; m < modCount; ++m )
			SymUnloadModule64( ( HANDLE )mProcessHandle, ( DWORD64 )mModules[ m ].mBaseAddress );
	}

	//------------------------------------------------------------------------------
	b32 tSymbolHelper::fLoadSymbolsForModule( 
		const char * moduleName,
		u64 baseAddress,
		u32 size,
		u32 timeStamp,
		const tSymbolsSignature & signature )
	{
		const char * path = signature.mPath.fCStr( );

		// See if the sym server can find the file
		char resultPath[MAX_PATH];
		BOOL found = SymFindFileInPath( 
			( HANDLE )mProcessHandle,
			NULL,
			path,
			(GUID*)&signature.mGuid,
			signature.mAge,
			0,
			SSRVOPT_GUIDPTR,
			resultPath,
			NULL, NULL );

		if( found )
			path = resultPath;

		DWORD64 result = SymLoadModuleEx(
			( HANDLE )mProcessHandle, //process
			NULL, // file for the image
			path, // name of the image
			moduleName, // shortcut name
			( DWORD64 )baseAddress, // base of dll
			size,	// size of dll
			NULL, // MODLOAD_DATA
			0 ); // flags

		if( result == 0 )
			return false;

		sigassert( result == baseAddress );
		
		tModule module;
		module.mName = moduleName;
		module.mBaseAddress = baseAddress;
		module.mSize = size;
		module.mTimeStamp = timeStamp;
		module.mSymbolsSignature = signature;

		mModules.fPushBack( module );
		return true;
	}

	//------------------------------------------------------------------------------
	b32 tSymbolHelper::fGetSymbolSummary( 
		u64 address, std::string & symbol, std::string & file )
	{
		//static const u32 cMaxSymbolNameLength = 512;
		char buffer[ sizeof( SYMBOL_INFO ) + MAX_SYM_NAME];
		SYMBOL_INFO * info = ( SYMBOL_INFO * )buffer;
		info->SizeOfStruct = sizeof( SYMBOL_INFO );
		info->MaxNameLen = MAX_SYM_NAME;

		BOOL success = SymFromAddr( ( HANDLE )mProcessHandle, ( DWORD64 )address, NULL, info );
		if( !success )
			return false;

		std::stringstream ssSym;
		std::stringstream ssFile;

		// Find the module name
		const u32 moduleCount = mModules.fCount( );
		for( u32 m = 0; m < moduleCount; ++m )
		{
			const tModule & module = mModules[ m ];
			if( module.mBaseAddress == info->ModBase )
			{
				ssSym << module.mName;
				break;
			}
		}

		// Append separator and symbol name
		ssSym << "!" << info->Name;

		// Retrieve and append line information
		IMAGEHLP_LINE64 lineInfo;
		DWORD lineDisp = 0;
		lineInfo.SizeOfStruct = sizeof( lineInfo );
		success = SymGetLineFromAddr64( ( HANDLE )mProcessHandle, ( DWORD64 )address, &lineDisp, &lineInfo );
		if( success )
		{
			ssSym << " Line " << lineInfo.LineNumber;
			if( lineDisp )
				ssSym << " + 0x" << std::hex << lineDisp << " bytes";

			ssFile << lineInfo.FileName << "(" << lineInfo.LineNumber << ")";
		}

		// <imageName>!<symbolName> Line <line> + 0x<bytes>
		symbol = ssSym.str( );
		file = ssFile.str( );
		return true;
	}
}

#endif // defined( sig_use_symbolhelper )
#endif // defined( platform_pcdx9 ) || defined( platform_pcdx10 )