#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#include "tConsoleApp.hpp"

namespace Sig
{
	namespace
	{
		const WORD	cNormalColor			= FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY;
		const WORD	cTitleColor				= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY;
		const WORD	cHeaderColor			= FOREGROUND_RED | BACKGROUND_INTENSITY;
		const WORD	cNewFolderColor			= BACKGROUND_INTENSITY;
		const WORD	cProcessFileColor		= FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY;
		const WORD	cOutputFromFileColor	= FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY;
		const WORD	cWarningColor			= FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY;

		tConsoleApp* gTheApp = 0;
	}

	tConsoleApp::tConsoleApp( )
		: mOutputHandle( 0 )
		, mIsRealConsoleApp( false )
		, mCurrentConsoleColor( cNormalColor )
		, mWarningCount( 0 )
		, mBufferWarnings( false )
	{
		sigassert( !gTheApp && "Only one instance of the tConsoleApp class is allowed at one time!" );
		gTheApp = this;
	}

	tConsoleApp::~tConsoleApp( )
	{
		gTheApp = 0;
	}


	const char* tConsoleApp::fHandleSpecialText( const char* text, const std::string& special, const WORD color )
	{
		if( strncmp( text, special.c_str( ), special.length( ) ) != 0 )
			return text;

		SetConsoleTextAttribute( mOutputHandle, color );

		if( mIsRealConsoleApp )
			printf( special.c_str( ) );
		else
		{
			DWORD numWritten = 0;
			WriteConsole( mOutputHandle, special.c_str( ), (DWORD)special.length( ), &numWritten, 0 );
		}

		return text + special.length( );
	}

	const char* tConsoleApp::fHandleSpecialText( const char* text )
	{
		if( !mOutputHandle )
			return text;

		const char* out;

		out = fHandleSpecialText( text, "@ ", cHeaderColor );
		if( out != text )
			return out;

		out = fHandleSpecialText( text, ">> ", cNewFolderColor );
		if( out != text )
			return out;

		out = fHandleSpecialText( text, "-> ", cProcessFileColor );
		if( out != text )
			return out;

		out = fHandleSpecialText( text, "Output from file", cOutputFromFileColor );
		if( out != text )
			return out;

		out = fHandleSpecialText( text, "!WARNING!", cWarningColor );
		if( out != text )
		{
			++mWarningCount;
			return out;
		}

		return text;
	}

	void tConsoleApp::fConsoleOutput( const char* text )
	{
		if( !fLogFilter( text ) )
			return;

		fConsoleOutputForce( text );
	}

	void tConsoleApp::fConsoleOutputForce( const char* text )
	{
		if( !mOutputHandle )
			return;

		const u32 warningCountPre = mWarningCount;
		text = fHandleSpecialText( text );
		const u32 warningCountPost = mWarningCount;

		if( mBufferWarnings && warningCountPost > warningCountPre )
			mBufferedWarnings.fFindOrAdd( text );

		SetConsoleTextAttribute( mOutputHandle, mCurrentConsoleColor );

		if( mIsRealConsoleApp )
			printf( text );
		else
		{
			DWORD numWritten = 0;
			WriteConsole( mOutputHandle, text, ( DWORD )strlen( text ), &numWritten, 0 );
		}
	}

	void tConsoleApp::fConsoleOutputStatic( const char* text, u32 flag )
	{
		if( gTheApp )
			gTheApp->fConsoleOutput( text );
	}

	void tConsoleApp::fCreateConsole( const char* welcomeText, b32 isRealConsoleApp )
	{
		mIsRealConsoleApp = isRealConsoleApp;

		if( !mIsRealConsoleApp )
			AllocConsole( );
		mOutputHandle = GetStdHandle( STD_OUTPUT_HANDLE );

		Log::fAddOutputFunction( fConsoleOutputStatic );

		COORD size={0}; size.X = 196; size.Y = 9999;
		SetConsoleScreenBufferSize( GetStdHandle( STD_OUTPUT_HANDLE ), size );
		SetConsoleScreenBufferSize( GetStdHandle( STD_INPUT_HANDLE ), size );
		SetConsoleScreenBufferSize( GetStdHandle( STD_ERROR_HANDLE ), size );

		if( !mIsRealConsoleApp )
		{
			SMALL_RECT rect={0}; rect.Right = size.X-1; rect.Bottom = 32-1;
			SetConsoleWindowInfo( GetStdHandle( STD_OUTPUT_HANDLE ), TRUE, &rect );
			SetConsoleWindowInfo( GetStdHandle( STD_INPUT_HANDLE ), TRUE, &rect );
			SetConsoleWindowInfo( GetStdHandle( STD_ERROR_HANDLE ), TRUE, &rect );
		}

		DWORD numAttrsWritten=0;
		COORD upperLeft; upperLeft.X = 0; upperLeft.Y = 0;
		FillConsoleOutputAttribute( mOutputHandle, cNormalColor, size.X*size.Y, upperLeft, &numAttrsWritten );

		if( !mIsRealConsoleApp )
		{
			HWND hwnd = GetConsoleWindow( );
			RECT winRect;
			GetWindowRect( hwnd, &winRect );
			RECT desktopRect = Win32Util::fGetDesktopWorkAreaRect( );
			SetWindowPos( hwnd, HWND_TOP, 0, desktopRect.bottom - 420, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW );
			SetParent( hwnd, GetDesktopWindow( ) );
		}

		mCurrentConsoleColor = cTitleColor;
		fConsoleOutputForce( welcomeText );
		mCurrentConsoleColor = cNormalColor;
	}

	void tConsoleApp::fDestroyConsole( const char* goodbyeText, b32 pauseOnExit )
	{
		mCurrentConsoleColor = cTitleColor;
		log_newline( );
		log_line( 0, goodbyeText );
		mCurrentConsoleColor = cNormalColor;
		SetConsoleTextAttribute( mOutputHandle, mCurrentConsoleColor );

		if( pauseOnExit )
			system( "pause" );

		FreeConsole( );
		
		Log::fRemoveOutputFunction( fConsoleOutputStatic );
	}

}
#endif//#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )

