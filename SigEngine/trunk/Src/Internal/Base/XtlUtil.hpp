#if defined( platform_xbox360 )
#ifndef __XtlUtil__
#define __XtlUtil__

namespace Sig { namespace XtlUtil
{
	bool		fCreateDirectory( const char* path );
	HANDLE		fCreateReadOnlyFileHandle( const char* path, b32 directory );

#define sigcheckfail_xoverlapped_done_else_wait_complete( xo )	sigcheckfail( XHasOverlappedIoCompleted( (xo) ), (void)XGetOverlappedResult( (xo), NULL, TRUE ) );
#define sigcheckfail_xoverlapped_done_else_wait_cancel( xo )	sigcheckfail( XHasOverlappedIoCompleted( (xo) ), XCancelOverlapped( (xo) ) );

	///
	/// \class tOverlappedOp
	/// \brief Wraps common overlapped operation features
	class tOverlappedOp
	{
	public:

		tOverlappedOp( );
		~tOverlappedOp( );

		
		b32 fIsComplete( ) const;
		void fWaitToComplete( );

		// Returns whether the result was available, specify wait to ensure it is
		b32 fGetResult( u32 & result, b32 wait = false );
		DWORD fGetResultWithError( u32 & result, b32 wait = false );
		b32 fGetResultNoMoreFilesOk( u32 & result, b32 wait = false );

		// NB: Due to complex argument differences in types of overlapped operations derivations
		// are responsible for providing methods to begin execution
		
		void fCancel( );
		void fReset( );

	protected:

		b32 fPreExecute( ); // Must be called and verified by derivations before executing
		XOVERLAPPED * fOverlapped( ) { return &mOverlapped; }
		const XOVERLAPPED * fOverlapped( ) const { return &mOverlapped; }

	private:

		XOVERLAPPED mOverlapped;
	};

	///
	/// \class tEnumerateOp
	/// \brief Overlapped XEnumerate call, takes ownership of handle
	class tEnumerateOp : public tOverlappedOp
	{
	public:

		tEnumerateOp( );
		~tEnumerateOp( );

		void fInitialize( u32 bufferSize, HANDLE enumeratorHandle );
		b32 fBegin( );
		void fReset( );

		u32 fResultCount( ) const { return fOverlapped( )->InternalHigh; }
		
		template<class T>
		const T * fResults( ) const { return (T*)mResultBuffer.fBegin( ); }

	private:

		HANDLE mEnumeratorHandle;
		tGrowableBuffer mResultBuffer;
	};

	///
	/// \class tContentDeleteOp
	/// \brief Overlapped XEnumerate call, takes ownership of handle
	class tContentDeleteOp : public tOverlappedOp
	{
	public:

		tContentDeleteOp( );
		~tContentDeleteOp( );

		void fInitialize( u32 userIdx, const XCONTENT_DATA& contentData );
		b32 fBegin( );

	private:

		u32 mUserIdx;
		XCONTENT_DATA mContentData;
	};

	/// \brief WARNING: NO-OP
	inline void fOpenUrl( const char* url ) { log_warning_unimplemented( ); }

	base_export void fExportScriptInterface( tScriptVm& vm );
}}

namespace Sig { namespace OsUtil {
	using namespace ::Sig::XtlUtil;
}}

#endif//__XtlUtil__

#else

#ifndef XCONTENT_MAX_FILENAME_LENGTH
	struct XCONTENT_DATA;
	#define XCONTENT_MAX_FILENAME_LENGTH 42
#endif

#endif//#if defined( platform_xbox360 )
