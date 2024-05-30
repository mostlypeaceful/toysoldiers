#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#ifndef __tConsoleApp__
#define __tConsoleApp__
#include "Win32Util.hpp"

namespace Sig
{
	///
	/// \brief Simple wrapper on top of a pop-up console, makes it easier to write
	/// a "console" app that is actually a stand-alone win32 app. Also, keeps formatting
	/// consistent across these sorts of apps. See AssetGen or MaterialGen for examples of usage.
	class base_export tConsoleApp : tUncopyable
	{
		HANDLE		mOutputHandle;
		b16			mIsRealConsoleApp;
		WORD		mCurrentConsoleColor;
		u32			mWarningCount;
		b32			mBufferWarnings;
		tGrowableArray<std::string> mBufferedWarnings;

	public:
		tConsoleApp( );
		virtual ~tConsoleApp( );
		void fCreateConsole( const char* welcomeText, b32 isRealConsoleApp = false );
		void fDestroyConsole( const char* goodbyeText, b32 pauseOnExit );
		u32	fWarningCount( ) const { return mWarningCount; }
		b32 fBufferWarnings( ) const { return mBufferWarnings; }
		void fSetBufferWarnings( b32 bufferWarnings ) { mBufferWarnings = bufferWarnings; }
		tGrowableArray<std::string>& fBufferedWarnings( ) { return mBufferedWarnings; }
		const tGrowableArray<std::string>& fBufferedWarnings( ) const { return mBufferedWarnings; }

	protected:

		///
		/// \brief Return false to cancel the log.
		virtual b32 fLogFilter( const char* text ) { return true; }

	private:
		const char* fHandleSpecialText( const char* text, const std::string& special, const WORD color );
		const char* fHandleSpecialText( const char* text );
		void		fConsoleOutput( const char* text );
		void		fConsoleOutputForce( const char* text );
		static void fConsoleOutputStatic( const char* text, u32 flag );
	};

}

#endif//__tConsoleApp__
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
