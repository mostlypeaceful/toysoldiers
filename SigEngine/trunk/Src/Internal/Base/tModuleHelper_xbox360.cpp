//------------------------------------------------------------------------------
// \file tModuleHelper_xbox360.cpp - 31 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#if defined( platform_xbox360 )
#include "tModuleHelper.hpp"

#ifndef build_release
	#include <xbdm.h>
#endif

namespace Sig
{
	//------------------------------------------------------------------------------
	void tModuleHelper::fRefreshModules( )
	{
#ifndef build_release

		HRESULT error;
		PDM_WALK_MODULES walker = NULL;
		DMN_MODLOAD modLoad;

		tGrowableArray< tModule > newModules;
		while( XBDM_NOERR == ( error = DmWalkLoadedModules( &walker, &modLoad ) ) )
		{
			tModule module;
			module.mBaseAddress = modLoad.BaseAddress;
			module.mName = modLoad.Name;
			module.mSize = modLoad.Size;
			module.mTimeStamp = modLoad.TimeStamp;

			DM_PDB_SIGNATURE signature = { 0 };
			if( XBDM_NOERR == DmFindPdbSignature( modLoad.BaseAddress, &signature ) )
			{
				static_assert( sizeof( module.mSymbolsSignature.mGuid ) == sizeof( signature.Guid ) );
				fMemCpy( &module.mSymbolsSignature.mGuid, &signature.Guid, sizeof( module.mSymbolsSignature.mGuid ) );
				module.mSymbolsSignature.mAge = signature.Age;
				module.mSymbolsSignature.mPath = tFilePathPtr( signature.Path ); 
			}

			newModules.fPushBack( module );
		}

		if( error != XBDM_ENDOFLIST )
		{
			// Handle errors
		}

		DmCloseLoadedModules( walker );
		mModules.fInitialize( newModules.fBegin( ), newModules.fCount( ) );

#else
		mModules.fDeleteArray( );
#endif

	}
}

#endif // #if defined( platform_xbox360 )
