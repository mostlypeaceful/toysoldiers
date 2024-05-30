#include "BasePch.hpp"

namespace Sig { namespace Log
{
#ifdef sig_logging
	void fPrintf( u32 flags, const char* formatString, ... )
	{
		char buff[1024];
		fZeroOut( buff );

		va_list args;
		va_start(args, formatString);
		//const int nSize = 
			vsnprintf( buff, sizeof(buff) - 1, formatString, args); // C4996

		fStandardOutputFunction( buff, flags );
	}
	void fSprintf( u32 flags, char buff[], u32 buffLen, const char* formatString, ... )
	{
		fMemSet( buff, 0, buffLen );

		va_list args;
		va_start(args, formatString);
		//const int nSize = 
			vsnprintf( buff, buffLen - 1, formatString, args); // C4996
	}
#else
	void fStandardOutputFunction( const char* text, u32 flag )
	{
		// do nothing
	}
#endif//sig_logging

	void tFlagsType::fRegisterFlagsType( const tFlagsType& type, u32 id )
	{
		// Make sure we didnt go past the end, and make sure it's sequential
		sigassert( id < tFlagList::cMaxFlags && id == fGetFlagList( ).gFlagTypeCount );
		fGetFlagList( ).gFlagTypes[ id ] = type;
		++fGetFlagList( ).gFlagTypeCount;
	}

	u32 tFlagsType::fFlagsTypeCount( )
	{
		return fGetFlagList( ).gFlagTypeCount;
	}

	const tFlagsType& tFlagsType::fFlagsType( u32 index )
	{
		sigassert( index < tFlagList::cMaxFlags );
		return fGetFlagList( ).gFlagTypes[ index ];
	}

	tFlagList& tFlagsType::fGetFlagList( ) 
	{ 
		static tFlagList list; 
		return list; 
	}

	void fInitializeSystem( )
	{
#		define log_flag_register_types
#			include "LogFlags.hpp"
#		undef log_flag_register_types

		// Add all the system log flags on by default
		u32 mask = fGetLogFilterMask( );

		for( u32 i = 0; i < tFlagsType::fFlagsTypeCount( ); ++i )
			mask |= tFlagsType::fFlagsType( i ).mFlag;

		// Disable the audio log by default
		mask = fClearBits( mask, tFlagsType::fFlagsType( Log::cFlagAudio ).mFlag );

		fSetLogFilterMask( mask );
	}

	namespace
	{

		static void fLogOutputScript( u32 flag, const char* text, bool noSpam )
		{
#ifdef sig_logging
			Detail::tOutput o( flag, noSpam );
			o << text;
#endif//sig_logging
		}

		static void fLogWarningScript( u32 flag, const char* text, bool noSpam )
		{
#ifdef sig_logging
			Detail::tWarning w( flag, noSpam );
			w << text;
#endif//sig_logging
		}

		static void fBreakPointScript( const char* reason )
		{
#ifdef sig_logging
			Log::tAssert::fFire( reason, __FUNCTION__, __FILE__, __LINE__ );
#endif//sig_logging
		}
	}

	void fExportScriptInterface( tScriptVm& vm )
	{
		// export log flags to script so they can be accessed as named constants
		for( u32 i = 0; i < Log::tFlagsType::fFlagsTypeCount( ); ++i )
		{
			const Log::tFlagsType& flag = Log::tFlagsType::fFlagsType( i );
			if( flag.mScriptName )
				vm.fConstTable( ).Const( _SC(flag.mScriptName), (int)i );
		}

		vm.fRootTable( ).Func( _SC("LogOutput"), &fLogOutputScript );
		vm.fRootTable( ).Func( _SC("LogWarning"), &fLogWarningScript );
		vm.fRootTable( ).Func( _SC("BreakPoint"), &fBreakPointScript );
	}

}}


namespace Sig { namespace Log
{
	namespace
	{
		static u32 gLogFilterMask = 0;
		static b32 gLogNoSpam = 0;
		static std::string gLastString = "";
	}

	class base_export tFuncList : public tGrowableArray<tOutputFunction>
	{
		declare_singleton( tFuncList );
	};

	define_static_function( _add_standard_output_function )
	{
#if defined( sig_logging )
		fAddOutputFunction( fStandardOutputFunction );
#endif //build_release
	}

	void fAddOutputFunction( tOutputFunction pf )
	{
		tFuncList::fInstance( ).fPushBack( pf );
	}

	void fRemoveOutputFunction( tOutputFunction pf )
	{
		tFuncList::fInstance( ).fFindAndErase( pf );
	}

	u32  fGetLogFilterMask( )
	{
		return gLogFilterMask;
	}

	void fSetLogFilterMask( u32 mask )
	{
		gLogFilterMask = mask;
	}

	void fSetLogNoSpam( b32 noSpam )
	{
		gLogNoSpam = noSpam;
	}

}}

#ifdef sig_logging
namespace Sig { namespace Log { namespace Detail
{
	tOutput::tOutput( u32 flagID, b32 appendNewLine, const char* prependText, b32 noSpam )
		: mFlagID( flagID )
		, mNewline( appendNewLine )
		, mNoSpam( noSpam )
	{
		const tFlagsType& flag = tFlagsType::fFlagsType( mFlagID );

		mSkipLog = (flag.mFlag & gLogFilterMask) == 0;
		if( mSkipLog )
			return;

		(*this) << std::dec;

		if( prependText )
			(*this) << prependText << " ";

		if( mFlagID )
			(*this) << flag.mOutputName << " ";

		if( prependText || mFlagID )
			(*this) << ">> ";
	}

	tOutput::~tOutput( )
	{
		fLog( );
	}

	void tOutput::fLog( )
	{
		if( mSkipLog )
			return;

		if( mNoSpam ) 
			(*this) << " (NoSpam)"; // Warn that we may skip future logs of this message.

		if( mNewline )
			(*this) << std::endl;

		std::string text = fStream( ).str( );

		if( (mNoSpam || gLogNoSpam) && text == gLastString )
			return; // Skip due to spamming.

		gLastString = text;

		for( tFuncList::tIterator i = tFuncList::fInstance( ).fBegin( );
			i != tFuncList::fInstance( ).fEnd( ); ++i )
		{
			(*i)( text.c_str( ), tFlagsType::fFlagsType( mFlagID ) );
		}
	}
}}}

#ifdef sig_assert
namespace Sig { namespace Log
{
	namespace
	{
		static u64 gWindowHandle = 0;
	}

	void tAssert::fSetOwnerWindow( u64 winHandle )
	{
		gWindowHandle = winHandle;
	}

	u64 tAssert::fGetOwnerWindow( )
	{
		return gWindowHandle;
	}

	void tAssert::fFire( const char* exp, const char* function, const char* file, u32 line )
	{
		char buff[1024]={0};
		fSprintf( 0, buff, sizeof( buff ), "Assertion Failed!\n\tContext: %s\n\tFunction: [%s]\n\tFile: [%s]\n\tLine: [%d]\n", exp, function, file, line );
#ifdef target_game
		tScriptVm::fDumpCallstack( );
#endif
		fFatalError( buff );
	}

}}
#endif//sig_assert

#endif//sig_logging

