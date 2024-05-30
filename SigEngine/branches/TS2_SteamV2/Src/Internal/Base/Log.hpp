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
	void fFatalError( const char* lastTextUserWillSee = 0 );

#ifdef sig_logging
	void fPrintf( u32 flags, const char* formatString, ... );
	void fSprintf( u32 flags, char buff[], u32 buffLen, const char* formatString, ... );
#else
	inline void fPrintf( u32 flags, const char* formatString, ... ) { }
	inline void fSprintf( u32 flags, char buff[], u32 buffLen, const char* formatString, ... ) { }
#endif

	struct tFlagList;

	struct tFlagsType
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
}}

namespace Sig { namespace Log { namespace Detail
{
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
	public:
		inline tWarning( u32 flagID = cFlagNone, b32 noSpam = false )
			: tOutput( flagID, true, "!WARNING!", noSpam ) { }
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
#else//sig_logging and sig_assert are not defined
	public:
		static inline void fSetOwnerWindow( u64 winHandle ) { }
		static inline u64  fGetOwnerWindow( ) { return 0; }
		static inline void fFire( const char* exp, const char* function, const char* file, u32 line ) { }
#endif//sig_logging && sig_assert
	};

	void fInitializeSystem( );
	void fExportScriptInterface( tScriptVm& vm );
}}

#if defined( sig_logging )
#	define log_output( flags, toOutput ) \
		{ Log::Detail::tOutput __o( flags, false ); __o << toOutput; }
#	define log_line( flags, toOutput ) \
		{ Log::Detail::tOutput __o( flags, true ); __o << toOutput; }
#	define log_newline( ) \
		{ Log::Detail::tOutput __o; }
#	define log_warning( flags, toOutput ) \
		do { Log::Detail::tWarning __o( flags, false ); __o << toOutput; } while( 0 )
#	define log_warning_nospam( flags, toOutput ) \
		{ Log::Detail::tWarning __o( flags, true ); __o << toOutput; }
#else
#	define log_output( flags, toOutput ) \
		{ }
#	define log_line( flags, toOutput ) \
		{ }
#	define log_newline( ) \
		{ }
#	define log_warning( flags, toOutput ) \
		do { } while( 0 )
#	define log_warning_nospam( flags, toOutput ) \
		{ }
#endif

#	define log_warning_unimplemented( flags ) \
		log_warning( flags, "[" << __FUNCTION__ << "] at [" << __FILE__ << ":" << __LINE__ << "] is not implemented." )


#if defined( sig_assert ) && defined( sig_logging )
#	define if_assert( x ) x
#	define sigassert( x ) \
		(void)( (!!(x)) || ((::Sig::Log::tAssert::fFire( #x, __FUNCTION__, __FILE__, __LINE__ ) ), 0) )
#	define log_assert( x, toOutput ) \
		do{ if(!(x)){ std::stringstream __ss; __ss << "[" << #x << "] " << toOutput; std::string __s = __ss.str( ); ::Sig::Log::tAssert::fFire( __s.c_str( ), __FUNCTION__, __FILE__, __LINE__ ); } } while( 0 )
#else
#	define if_assert( x )
#	define sigassert( x ) \
		((void)0)
#	define log_assert( x, toOutput ) \
		{ }
#endif//sig_assert


#define sig_make_loggable( className, toOutput ) \
	public: \
		std::ostream& operator<<( std::ostream& os ) const { if_logging( os << toOutput ); return os; } \
		friend std::ostream& operator<<( std::ostream& os, const className& value ) { if_logging( value << os ); return os; }

#define sig_make_stringstreamable( className, toOutput ) \
	public: \
		std::ostream& operator<<( std::ostream& os ) const { os << toOutput; return os; } \
		std::wostream& operator<<( std::wostream& os ) const { os << toOutput; return os; } \
		friend std::ostream& operator<<( std::ostream& os, const className& value ) { value << os; return os; } \
		friend std::wostream& operator<<( std::wostream& os, const className& value ) { value << os; return os; }

#endif//__Log__


