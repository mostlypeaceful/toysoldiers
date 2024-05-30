//------------------------------------------------------------------------------
// \file tSymbolHelper.hpp - 21 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tSymbolHelper__
#define __tSymbolHelper__
#include "tModuleHelper.hpp"

#if defined( target_tools ) || !defined( build_release )
#define sig_use_symbolhelper
#endif

#if defined( sig_use_symbolhelper )
namespace Sig
{
	///
	/// \class tSymbolHelper
	/// \brief 
	class base_export tSymbolHelper
	{
	public: // Construction
		
		tSymbolHelper( const char * symbolSearchPath, b32 currentProcess );
		~tSymbolHelper( );

	public: // Methods

		b32 fLoadSymbolsForModule( const tModule & module );
		b32 fLoadSymbolsForModule( 
			const char * moduleName,
			u64 baseAddress,
			u32 size,
			u32 timeStamp,
			const tSymbolsSignature & signature );

		b32 fGetSymbolSummary( u64 address, std::string & symbol, std::string & file );

	private: // Data

		u64 mProcessHandle;
		tDynamicArray< tModule > mModules;
	};
}
#endif
#endif//__tSymbolHelper__
