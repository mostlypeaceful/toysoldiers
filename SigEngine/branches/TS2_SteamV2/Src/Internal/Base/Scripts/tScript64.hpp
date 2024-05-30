//------------------------------------------------------------------------------
// \file tScript64.hpp - 03 Dec 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tScript64__
#define __tScript64__

namespace Sig
{
	///
	/// \class tScript64
	/// \brief 
	class base_export tScript64
	{
	public:

		tScript64( ) { }
		explicit tScript64( u64 value ) : mValue( value ) { }
		explicit tScript64( s64 value ) : mValue( ( u64 )value ) { }

		u64 fGet( ) const { return mValue; }
		s64 fGetSigned( ) const { return ( s64 )mValue; }

		void fSet( u64 value ) { mValue = value; }
		void fSet( s64 value ) { mValue = ( u64 )value; }

	private:
		u64 mValue;

	public:

		static void fExportScriptInterface( tScriptVm & vm );
	};
}

#endif//__tScript64__
