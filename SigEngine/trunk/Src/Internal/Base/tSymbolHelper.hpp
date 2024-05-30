//------------------------------------------------------------------------------
// \file tSymbolHelper.hpp - 21 Jan 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tSymbolHelper__
#define __tSymbolHelper__
#include "tModuleHelper.hpp"

namespace Sig
{
	///
	/// \class tSymbolHelper
	/// \brief 
	class base_export tSymbolHelper
	{
	public: // Construction
		
		tSymbolHelper( const char * symbolSearchPath );
		~tSymbolHelper( );

	public: // Methods

		b32 fLoadSymbolsForModule( const tModule & module );
		b32 fLoadSymbolsForModule( 
			const char * moduleName,
			void * baseAddress,
			u32 size,
			u32 timeStamp,
			const tSymbolsSignature & signature );

		b32 fGetSymbolSummary( 
			void * address, std::string & symbol, std::string & file );

	private: // Data

		u64 mProcessHandle;
		tDynamicArray< tModule > mModules;
	};
}

#endif//__tSymbolHelper__
