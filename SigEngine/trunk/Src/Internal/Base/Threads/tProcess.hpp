#ifndef __tProcess__
#define __tProcess__

namespace Sig { namespace Threads
{
	class base_export tProcess : public tUncopyable, public tRefCounter
	{
	public:

		///
		/// \return True if the process was successfully spawned, false otherwise.
		static b32 fSpawnAndForget( const char* procName, const char* procCmdLine = 0, const char* startDir = 0, b32 waitUntilFinished = false, b32 doNotPrependWhitespace = false );

		static b32 fBasicRawDangerousSpawnAndForget( const char* procName, const char* procCmdLine = NULL, const char* startDir = NULL );

	public:

		tProcess( const char* procName, const char* procCmdLine = 0, const char* startDir = 0, b32 doNotPrependWhitespace = false );
		~tProcess( );
		b32 fCreatedSuccessfully( ) const;
		b32 fIsRunning( ) const;
		u32 fProcessId( ) const;
		void fWaitUntilFinished( );

	private:

#if defined( platform_pcdx )
		PROCESS_INFORMATION mProcInfo;
#endif
	};

	define_smart_ptr( base_export, tRefCounterPtr, tProcess );

}}

#endif//__tProcess__
