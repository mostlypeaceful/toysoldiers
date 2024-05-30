//------------------------------------------------------------------------------
// \file tDevVarLogFlag.cpp - 8 Oct 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#include "BasePch.hpp"
#include "tDevVarLogFlag.hpp"
#include <string>

namespace Sig
{

#ifdef sig_devmenu
	tDevVarLogFlag::tDevVarLogFlag( const char* path, const Log::tFlagsType& flag )
		: tDevMenuItem( path )
		, mFlag( flag )
	{
		fFindAndApplyIniOverride( );
	}

	std::string tDevVarLogFlag::fIthItemValueString( u32 i ) const
	{
		return fIsLogging( ) ? "LOG" : "hide";
	}

	void tDevVarLogFlag::fOnHighlighted( tDevMenuBase& menu, u32 ithItem, tUser& user, f32 absTime, f32 dt )
	{
		b32 value = fIsLogging( ) ? true : false; // necessary conversion to 1/0, fUpdateValue treats it as a number...
		fUpdateValue<b32>( user, absTime, dt, value, 0, false, true );
		fSetLogging( value );
	}

	void tDevVarLogFlag::fSetFromVector( const Math::tVec4f& v )
	{
		fSetLogging( v != Math::tVec4f::cZeroVector );
	}
		
	void tDevVarLogFlag::fSetItemValue( u32 itemIndex, const std::string & value )
	{
		const b32 isTrue = StringUtil::fStricmp( value.c_str( ), "true" ) == 0;
		const b32 isLog = StringUtil::fStricmp( value.c_str( ), "log" ) == 0;
		fSetLogging( isTrue || isLog );
	}

	b32 tDevVarLogFlag::fIsLogging( ) const
	{
		return Log::fGetLogFilterMask( ) & mFlag.mFlag;
	}

	void tDevVarLogFlag::fSetLogging( b32 value ) const
	{
		if( value )
			Log::fSetLogFilterMask( Log::fGetLogFilterMask( ) | mFlag.mFlag );
		else
			Log::fSetLogFilterMask( Log::fGetLogFilterMask( ) & ~mFlag.mFlag );
	}

#else
	void fIgnoreLinkerWarning_NoSymbolsDefinedByDevVarLogFlags( ) { }

#endif

}
