#include "BasePch.hpp"
#include "Fui.hpp"
#include "tDevVarLogFlag.hpp"
#include "Debug/tDebugger.hpp"

namespace Sig { namespace Log
{
	namespace
	{
		b32 gShowDialog = true;
#if defined( target_game ) && defined( build_release )
		b32 gAllowContinue = false;
		b32 gSetBreakpoint = false;
#else
		b32 gAllowContinue = true;
		b32 gSetBreakpoint = true;
#endif
		b32 gCrashHard = false;
		b32 gLogCallstack = true;
	}


	u32 Detail::tWarning::mWarningCount = 0;

#ifdef sig_logging
	void fAssertPrint( const char* what, const char* file, const u32 line, const char* func )
	{
		//NICE TRICK: if we output __FILE__(__LINE__) on its own line then we can double-click on it
		// in Visual Studio's output window and it will jump us to that file/line #. NEATO huH?
		fPrintf( 0, "@ASSERT@\n\t%s\n\t%s(%d)\n\tFUNCTION: %s\n@ENDASSERT@\n", what, file, line, func );
		tScriptVm::fDumpCallstack( );
		tErrorContext::fPrint( );
	}
	void fPrintf( u32 flags, const char* formatString, ... )
	{
		char buff[1024];

		va_list args;
		va_start( args, formatString );
		{
			const s32 count = vsnprintf( buff, sizeof(buff), formatString, args );
			buff[ count < 0 ? sizeof(buff) - 1 : count ] = 0;
		}
		va_end( args );

		fStandardOutputFunction( buff, flags );
	}
	void fSprintf( u32 flags, char buff[], u32 buffLen, const char* formatString, ... )
	{
		va_list args;
		va_start( args, formatString );
		{
			const s32 count = vsnprintf( buff, buffLen, formatString, args );
			buff[ count < 0 ? buffLen - 1 : count ] = 0;
		}
		va_end( args );
	}
	void fPrintNewLine( )
	{
		fStandardOutputFunction( "\n", 0 );
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

#ifdef sig_devmenu
	tGrowableArray< tRefCounterPtr< tDevVarLogFlag > > gLogDevvars;
#endif

	void fCreateDevvars( )
	{
#ifdef sig_devmenu
		gLogDevvars.fClear( );

		for( u32 i = 0; i < Log::tFlagsType::fFlagsTypeCount( ); ++i )
		{
			const Log::tFlagsType& flag = Log::tFlagsType::fFlagsType( i );
			if( flag.mScriptName )
			{
				std::stringstream ss;
				ss << "Debug_Log_Flags_" << flag.mShortName;
				tRefCounterPtr< tDevVarLogFlag > dv( NEW_TYPED( tDevVarLogFlag )( ss.str( ).c_str( ), flag ) );
				gLogDevvars.fPushBack( dv );
			}
		}
#endif
	}

	void fInitializeSystem( )
	{
#		define log_flag_register_types
#			include "LogFlags.hpp"
#		undef log_flag_register_types

		// Enable some flags by default
		u32 mask = fGetLogFilterMask( );
		mask |= tFlagsType::fFlagsType( cFlagNone ).mFlag;
		mask |= tFlagsType::fFlagsType( cFlagScript ).mFlag;
		mask |= tFlagsType::fFlagsType( cFlagDevMenu ).mFlag;
		fSetLogFilterMask( mask );
		fCreateDevvars( );
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

	namespace
	{
		static void fLogWarningFui( Fui::tFuiFuncParams& params )
		{
#ifdef sig_logging
			const char* warning = NULL;
			sigassert( params.fCount( ) == 1 );
			params.fGet( warning );
			log_warning( warning );
#endif//sig_logging
		}

		static void fBreakPointFui( Fui::tFuiFuncParams& params )
		{
			debug_break( );
		}
	}

	void fExportFuiInterface( )
	{
		// TODO: Export flag constants & log_output to FUI somehow?
		Fui::tFuiSystem::fInstance( ).fRegisterFunc( "Log.fLogWarning", fLogWarningFui );
		Fui::tFuiSystem::fInstance( ).fRegisterFunc( "Log.fBreakPoint", fBreakPointFui );
	}
}}


namespace Sig { namespace Log
{
	namespace
	{
		static u32 gLogFilterMask = 0;
		static b32 gLogNoSpam = 0;
	}

	class base_export tFuncList : public tGrowableArray<tOutputFunction>
	{
		declare_singleton( tFuncList );
	};

	define_static_function( _add_standard_output_function )
	{
#ifdef sig_logging
		fAddOutputFunction( fStandardOutputFunction );
#endif //sig_logging
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

	b32 fFlagEnabled( u32 flag )
	{
		const Log::tFlagsType& logFlagType = tFlagsType::fFlagsType( flag );
		const u32 logFlagMask = fGetLogFilterMask( );
		return ( logFlagMask & logFlagType.mFlag );
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
		static std::string gLastString = "";
		
		if( mSkipLog )
			return;

		if( mNoSpam ) 
			(*this) << " (NoSpam)"; // Warn that we may skip future logs of this message.

		if( mNewline )
			(*this) << std::endl;

		std::string text = fStream( ).str( );

		if( (mNoSpam || gLogNoSpam) && text == gLastString )
			return; // Skip due to spamming.

		if( mNoSpam )
			gLastString = text;

		for( tFuncList::tIterator i = tFuncList::fInstance( ).fBegin( );
			i != tFuncList::fInstance( ).fEnd( ); ++i )
		{
			(*i)( text.c_str( ), tFlagsType::fFlagsType( mFlagID ) );
		}
	}
}}}
#endif//sig_logging


namespace Sig { namespace Log
{
#if defined( sig_assert ) && defined( sig_logging )
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
#ifdef target_game
		if( gLogCallstack )
			tScriptVm::fDumpCallstack( );
#endif
		Log::fPrintf( 0, "Assertion Failed!\n\tContext: %s\n\tFunction: [%s]\n\tFile: [%s]\n\tLine: [%d]\n", exp, function, file, line );
		fFatalError( NULL );
	}

	void tAssert::fSetShowDialog( b32 prompt )
	{
		gShowDialog = prompt;
	}
	void tAssert::fSetAllowContinue( b32 allow )
	{
		gAllowContinue = allow;
	}
	void tAssert::fSetBreakpoint( b32 breakpoint )
	{
		gSetBreakpoint = breakpoint;
	}
	void tAssert::fSetCrashHard( b32 crash )
	{
		gCrashHard = crash;
	}
#endif//sig_assert && sig_logging

	b32 tAssert::fShouldShowDialog( )		{ return gShowDialog; }
	b32 tAssert::fShouldAllowContinue( )	{ return gAllowContinue; }
	b32 tAssert::fShouldSetBreakpoint( )	{ return gSetBreakpoint; }
	b32 tAssert::fShouldCrashHard( )		{ return gCrashHard; }
}}
