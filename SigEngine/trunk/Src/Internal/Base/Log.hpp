#ifndef __Log__
#define __Log__
#ifndef __Core__
#error This file must be included via Core.hpp!
#endif//__Core__

#if defined( sig_logging )
#	define if_logging( x ) x
#else
#	define if_logging( x )
#endif//sig_logging

namespace Sig { namespace Log
{
	void base_export fFatalError( const char* lastTextUserWillSee = 0 );

#ifdef sig_logging
	void base_export fAssertPrint( const char* what, const char* file, const u32 line, const char* func );
	void base_export fPrintf( u32 flags, const char* formatString, ... );
	void base_export fSprintf( u32 flags, char buff[], u32 buffLen, const char* formatString, ... );
	void base_export fPrintNewLine( );
#else
	inline void fPrintf( u32 flags, const char* formatString, ... ) { }
	inline void fSprintf( u32 flags, char buff[], u32 buffLen, const char* formatString, ... ) { }
	inline void fPrintNewLine( ) { }
#endif

	struct tFlagList;

	struct base_export tFlagsType
	{
		u32			mFlag;
		const char* mShortName;		//use this name for Log_Flags in ini file
		const char* mOutputName;	//what shows up in log output
		const char* mScriptName;	//the symbol to use in script for LogOutput

		static u32  fFlagsTypeCount( );
		static const tFlagsType& fFlagsType( u32 index );

		tFlagsType( ) 
			: mFlag( 0 ), mShortName( NULL ), mOutputName( NULL ), mScriptName( NULL )
		{ }

		tFlagsType( u32 id, const char* shortName, const char* outputName, const char *scriptName ) 
			: mFlag( 1 << id ), mShortName(shortName), mOutputName(outputName), mScriptName( scriptName )
		{
			fRegisterFlagsType( *this, id );
		}

		inline operator u32( ) const { return mFlag; }
		
	private:
		static tFlagList& fGetFlagList( );
		static void fRegisterFlagsType( const tFlagsType& type, u32 id );
	};

	struct tFlagList
	{
		static const u32 cMaxFlags = 32;
		tFlagsType gFlagTypes[ cMaxFlags ];
		u32 gFlagTypeCount;

		tFlagList( ) : gFlagTypeCount( 0 ) { }
	};


#include "LogFlags.hpp"
	
	typedef void (*tOutputFunction)( const char* text, u32 flag );

	base_export void fStandardOutputFunction( const char* text, u32 flag );
	base_export void fAddOutputFunction( tOutputFunction pf );
	base_export void fRemoveOutputFunction( tOutputFunction pf );
	base_export u32  fGetLogFilterMask( );
	base_export void fSetLogFilterMask( u32 mask );
	base_export void fSetLogNoSpam( b32 noSpam );
	base_export b32  fFlagEnabled( u32 flag );
}}

namespace Sig { namespace Log { namespace Detail
{
	namespace LogOnlyOps
	{
#if defined( platform_metro )
		base_export std::ostream& operator<<( std::ostream& os, ::Platform::String^ ws );
		base_export std::ostream& operator<<( std::ostream& os, const std::wstring& ws );
		base_export std::ostream& operator<<( std::ostream& os, const wchar_t* ws );
#endif
	}

	///
	/// \brief Primary logging class, represents a single logging action; uses standard
	/// output operator to perform standard formatted output C++ style (i.e., ostream), rather
	/// than using printf style. Applications can register custom output functions to receive
	/// the results of each log action, in order to perform custom logging (i.e., to file).
	/// \note Should only be used via macros (i.e., log_line, log_output, etc) - never reference directly in code
	class base_export tOutput : public std::ostringstream
	{
	private:
		explicit tOutput( tOutput& );
		tOutput& operator=( const tOutput& );
	public:  
		
#ifdef sig_logging

		std::ostringstream& fStream( ) { return *this; }

	protected:

		u32			mFlagID;
		b16			mNewline;
		b8			mSkipLog;
		b8			mNoSpam;

		void fLog( );

	public:

		explicit tOutput( 
			u32 flagID = cFlagNone, 
			b32 appendNewLine = true,
			const char* prependText = 0,
			b32 noSpam = false );

		~tOutput( );

#else//sig_logging not defined

		inline tOutput( 
			u32 flagId = 0, 
			b32 appendNewLine = true,
			const char* prependText = 0,
			b32 noSpam = false )
		{
		}
		
#endif//sig_logging

#define overload_output_operator( outputType ) \
		inline tOutput& operator<<( outputType v ) { if_logging( fStream( ) << v ); return *this; } \
		friend inline tOutput& operator<<( outputType v, tOutput& o ) { return o << v; }

//		overload_output_operator( bool );
//		overload_output_operator( char );
//		overload_output_operator( signed char );
//		overload_output_operator( unsigned char );
//		overload_output_operator( signed short );
//		overload_output_operator( unsigned short );
//		overload_output_operator( signed int );
//		overload_output_operator( unsigned int );
//		overload_output_operator( signed long );
//		overload_output_operator( unsigned long );
//		overload_output_operator( signed long long );
//		overload_output_operator( unsigned long long );
//		overload_output_operator( float );
//		overload_output_operator( double );
//		overload_output_operator( char* );
//		overload_output_operator( const char* );
//		overload_output_operator( std::string& );
//		overload_output_operator( const std::string& );
#undef overload_output_operator

#define overload_output_operator_pf( outputType ) \
		inline tOutput& operator<<( outputType & (*pf)( outputType & ) ) { if_logging( fStream( ) << pf ); return *this; }

//		overload_output_operator_pf( std::ostream );
//		overload_output_operator_pf( std::ios );
//		overload_output_operator_pf( std::ios_base );
#undef overload_output_operator_pf
	};

	///
	/// \brief Wraps tOutput and pre-pends warning text to the output.
	/// \see tOutput.
	class base_export tWarning : public tOutput
	{
		static u32 mWarningCount;

	public:
		inline tWarning( u32 flagID = cFlagNone, b32 noSpam = false )
			: tOutput( flagID, true, "!WARNING!", noSpam ) { ++mWarningCount; }

		static inline void fResetErrors( ) { mWarningCount = 0; }
		static inline u32 fWarningCount( ) { return mWarningCount; }
	};
}}}

namespace Sig { namespace Log
{
	///
	/// \brief Wraps tOutput and pre-pends sigassert text to the output. Only fires
	/// if the argument to the constructor evalutes to 0 (false).
	/// \see tOutput.
	class base_export tAssert
	{
#if defined( sig_logging ) && defined( sig_assert )
	public:
		static void fSetOwnerWindow( u64 winHandle );
		static u64  fGetOwnerWindow( );
		static void fFire( const char* exp, const char* function, const char* file, u32 line );

		static void fSetShowDialog( b32 prompt );		///< Lets us show or supress dialog boxes on build servers.  (Default: prompt=true)
		static void fSetAllowContinue( b32 allow );		///< Lets us continue after an assertion occured, if the dialog prompt shows.  (Default: allow=true, unless release game)
		static void fSetBreakpoint( b32 breakpoint );	///< Lets us invoke a breakpoint for each assert.  (Default: breakpoint=true)
		static void fSetCrashHard( b32 crash );			///< Lets us crash via access violation for WER/MiniFuzz/???.  (Default: crash=false)

		static b32 fShouldShowDialog( );
		static b32 fShouldAllowContinue( );
		static b32 fShouldSetBreakpoint( );
		static b32 fShouldCrashHard( );

#else//sig_logging and sig_assert are not defined
	public:
		static inline void fSetOwnerWindow( u64 winHandle ) { }
		static inline u64  fGetOwnerWindow( ) { return 0; }
		static inline void fFire( const char* exp, const char* function, const char* file, u32 line ) { }

		static void fSetShowDialog( b32 prompt ){}		///< Lets us show or supress dialog boxes on build servers.  (Default: prompt=true)
		static void fSetAllowContinue( b32 allow ){}	///< Lets us continue after an assertion occured, if the dialog prompt shows.  (Default: allow=true, unless release game)
		static void fSetBreakpoint( b32 breakpoint ){}	///< Lets us invoke a breakpoint for each assert.  (Default: breakpoint=true 
		static void fSetCrashHard( b32 crash ){}		///< Lets us crash via access violation for WER/MiniFuzz/???.  (Default: crash=false)

		static b32 fShouldShowDialog( );
		static b32 fShouldAllowContinue( );
		static b32 fShouldSetBreakpoint( );
		static b32 fShouldCrashHard( );
#endif//sig_logging && sig_assert
	};

	base_export void fInitializeSystem( );
	base_export void fExportScriptInterface( tScriptVm& vm );
	base_export void fExportFuiInterface( );
}}

#if defined( sig_logging )
#	define log_output( flags, toOutput ) \
		do { using namespace ::Sig::Log::Detail::LogOnlyOps; ::Sig::Log::Detail::tOutput __o( flags, false ); __o << toOutput; } while( 0 )
#	define log_line( flags, toOutput ) \
		do { using namespace ::Sig::Log::Detail::LogOnlyOps; ::Sig::Log::Detail::tOutput __o( flags, true ); __o << toOutput; } while( 0 )
#	define log_newline( ) \
		do { ::Sig::Log::Detail::tOutput __o; } while( 0 )
#	define log_warning( toOutput ) \
		do { using namespace ::Sig::Log::Detail::LogOnlyOps; ::Sig::Log::Detail::tWarning __o( 0, false ); __o << toOutput; } while( 0 )
#	define log_warning_once( toOutput ) \
		do { using namespace ::Sig::Log::Detail::LogOnlyOps; static b32 once=false; if( once ) break; once=true; ::Sig::Log::Detail::tWarning __o( 0, false ); __o << toOutput; } while( 0 )
#	define log_warning_nospam( toOutput ) \
		do { using namespace ::Sig::Log::Detail::LogOnlyOps; ::Sig::Log::Detail::tWarning __o( 0, true ); __o << toOutput; } while( 0 )

#	define log_warning_nospam_time( toOutput, startFreq, backoff )				\
		do {																	\
			static f32 timeout = (startFreq) - (backoff);						\
			static ::Sig::Time::tStopWatch sinceLast( false );					\
			if( !sinceLast.fRunning( ) || sinceLast.fGetElapsedS() >= timeout )	\
			{																	\
				sinceLast.fRestart( );											\
				timeout += (backoff);											\
				log_warning_nospam( toOutput );									\
			}																	\
		} while( 0 ) /* ------------------------------------------------------ */
#else
#	define log_output( flags, toOutput ) \
		do { } while( 0 )
#	define log_line( flags, toOutput ) \
		do { } while( 0 )
#	define log_newline( ) \
		do { } while( 0 )
#	define log_warning( toOutput ) \
		do { } while( 0 )
#	define log_warning_once( toOutput ) \
		do { } while( 0 )
#	define log_warning_nospam( toOutput ) \
		do { } while( 0 )
#	define log_warning_nospam_time( toOutput, startFreq, backoff ) \
		do { } while( 0 )
#endif

#	define log_warning_unimplemented( ) \
		log_warning( "[" << __FUNCTION__ << "] at [" << __FILE__ << ":" << __LINE__ << "] is not implemented." )

#	define log_warning_banned_api( name ) \
		log_warning( "[" << __FUNCTION__ << "] at [" << __FILE__ << ":" << __LINE__ << "] uses banned API " << name << "." )

#if defined( sig_assert ) && defined( sig_logging )
#	define if_assert( x ) x
	#if defined( target_game )
		#define sigassert( x )				do{ if(!(x)) { ::Sig::Log::fAssertPrint( #x, __FILE__, __LINE__, __FUNCTION__ ); ::Sig::Log::fFatalError( #x ); } } while( 0 )
		#define log_assert( x, toOutput )	do{ if(!(x)) { using namespace Sig::Log::Detail::LogOnlyOps; std::stringstream __ss; __ss << "[" << #x << "] " << toOutput; const std::string __s = __ss.str( ); ::Sig::Log::fAssertPrint( __s.c_str( ), __FILE__, __LINE__, __FUNCTION__ ); ::Sig::Log::fFatalError( __s.c_str() ); } } while( 0 )
	#else
		#define sigassert( x ) \
			(void)( (!!(x)) || ((::Sig::Log::tAssert::fFire( #x, __FUNCTION__, __FILE__, __LINE__ ) ), 0) )
		#define log_assert( x, toOutput ) \
			do{ if(!(x)){ using namespace Sig::Log::Detail::LogOnlyOps; std::stringstream __ss; __ss << "[" << #x << "] " << toOutput; std::string __s = __ss.str( ); ::Sig::Log::tAssert::fFire( __s.c_str( ), __FUNCTION__, __FILE__, __LINE__ ); } } while( 0 )
	#endif//target_game
#	define sigassert_is_main_thread( )					sigassert( ::Sig::Threads::tThread::fMainThreadId( ) == ::Sig::Threads::tThread::fCurrentThreadId( ) )
#	define cmp_assert( lhs, cmp, rhs )					log_assert( (lhs) cmp (rhs), #lhs "=" << (lhs) << " " #rhs "=" << (rhs) )
#	define log_cmp_assert( lhs, cmp, rhs, toOutput )	log_assert( (lhs) cmp (rhs), #lhs "=" << (lhs) << " " #rhs "=" << (rhs) << " : " << toOutput )
#	define sig_nodefault( )								sigassert( !"Switch on unrecognized value!" )
#	define sigassert_and_analyze_assume( x )			sigassert(x)
#else
#	define if_assert( x )
#	define sigassert( x )								do{}while(0)
#	define log_assert( x, toOutput )					do{}while(0)
#	define sigassert_is_main_thread( )					do{}while(0)
#	define cmp_assert( lhs, cmp, rhs )					do{}while(0)
#	define log_cmp_assert( lhs, cmp, rhs, toOutput )	do{}while(0)
#	if defined( platform_msft )
#		define sig_nodefault( ) __assume( 0 )
#		define sigassert_and_analyze_assume( x ) __analysis_assume(x)
#	else
#		define sig_nodefault( ) do{}while(0)
#		define sigassert_and_analyze_assume( x ) do{}while(0)
#	endif
#endif//sig_assert

#define sigcheckfail( x, op )			if(!(x)){ sigassert( !"CheckFail: "#x ); op; } else (void)0
#define log_sigcheckfail( x, msg, op )	if(!(x)){ log_assert( !"CheckFail: "#x, msg ); op; } else (void)0


#define sig_make_stringstreamable( className, toOutput ) \
	public: \
		std::ostream& operator<<( std::ostream& os ) const { os << toOutput; return os; } \
		std::wostream& operator<<( std::wostream& os ) const { os << toOutput; return os; } \
		friend std::ostream& operator<<( std::ostream& os, const className& value ) { value << os; return os; } \
		friend std::wostream& operator<<( std::wostream& os, const className& value ) { value << os; return os; }

#endif//__Log__


