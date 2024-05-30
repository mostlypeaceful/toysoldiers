//------------------------------------------------------------------------------
// \file tModuleHelper_pc.cpp - 31 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "tModuleHelper.hpp"

#if defined( target_tools ) || !defined( build_release )
#define sig_use_modulehelper
#endif

#if defined( sig_use_modulehelper )
#include <DbgHelp.h>
#pragma comment( lib, "dbghelp.lib" )
#endif

namespace Sig
{
#if defined( sig_use_modulehelper )
	namespace
	{
		struct tEnumerateModuleContext
		{
			tGrowableArray<tModule> mModules;
			HANDLE mProcessHandle;
		};

		BOOL CALLBACK fEnumerateModulesProc64( PCTSTR moduleName, DWORD64 moduleBase, PVOID userContext )
		{
			tEnumerateModuleContext* context = (tEnumerateModuleContext*)userContext;
			IMAGEHLP_MODULE64 symInfo;
			symInfo.SizeOfStruct = sizeof(symInfo );
			SymGetModuleInfo64( context->mProcessHandle, moduleBase, &symInfo );
			
			tModule newModule;
			newModule.mName = moduleName;
			newModule.mBaseAddress = moduleBase;
			newModule.mSize = symInfo.ImageSize;
			newModule.mTimeStamp = symInfo.TimeDateStamp;
			newModule.mSymbolsSignature.mAge = symInfo.PdbAge;
			newModule.mSymbolsSignature.mPath = tFilePathPtr( symInfo.LoadedImageName );
			sig_static_assert( sizeof( newModule.mSymbolsSignature.mGuid ) == sizeof( symInfo.PdbSig70 ) );
			fMemCpy( &newModule.mSymbolsSignature.mGuid, &symInfo.PdbSig70, sizeof( newModule.mSymbolsSignature.mGuid ) );

			context->mModules.fPushBack( newModule );

			return TRUE;
		}
	}

#endif // defined( sig_use_modulehelper )

	//------------------------------------------------------------------------------
	void tModuleHelper::fRefreshModules( )
	{
#if defined( sig_use_modulehelper )
		tEnumerateModuleContext context;
		context.mProcessHandle = GetCurrentProcess( );
		
		BOOL result = SymInitialize( context.mProcessHandle, NULL, TRUE );
		result = SymEnumerateModules64( context.mProcessHandle, fEnumerateModulesProc64, &context );
		result = SymCleanup( context.mProcessHandle );

		mModules.fInitialize( context.mModules.fBegin( ), context.mModules.fCount( ) );
#endif
	}
}

#endif // #if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
