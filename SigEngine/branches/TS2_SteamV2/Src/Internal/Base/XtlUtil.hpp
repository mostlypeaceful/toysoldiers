#if defined( platform_xbox360 )
#ifndef __XtlUtil__
#define __XtlUtil__

#define XUTIL_ENUMERATE_BLOCKED

namespace Sig { namespace XtlUtil
{
	bool		fCreateDirectory( const char* path );
	HANDLE		fCreateReadOnlyFileHandle( const char* path, b32 directory );


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
		u32 fGetResult( u32 & result, b32 wait = false );
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
	/// \class tBlockingOp
	/// \brief Tries to look like tOverlappedOp, but doesn't do overlapped IO and assumes everything is done in fBegin.
	class tBlockingOp
	{
	public:
		b32 fIsComplete( ) const { return true; }
		void fWaitToComplete( ) {}

		u32 fGetResult( u32& result, b32 wait = false ) { result = ERROR_SUCCESS; return ERROR_SUCCESS; }
		DWORD fGetResultWithError( u32 & result, b32 wait = false ) { result = ERROR_SUCCESS; return ERROR_SUCCESS; }
		b32 fGetResultNoMoreFilesOk( u32& result, b32 wait= false ) { result = ERROR_SUCCESS; return true; }

		void fCancel( ) {}
		void fReset( ) {}
	protected:
		b32 fPreExecute( ) { return true; }
	};

	///
	/// \class tEnumerateOp
	/// \brief Blocking XEnumerateCrossTitle call, takes ownership of handle.  Could (should?) be replaced with multiple overlapped operations.
#ifdef XUTIL_ENUMERATE_BLOCKED
	class tEnumerateOp : public tBlockingOp
#else
	class tEnumerateOp : public tOverlappedOp // replace with a new class tMuliOverlappedOp? Might require refactoring out fGetResult*()
#endif
	{
	public:

		tEnumerateOp( );
		~tEnumerateOp( );

		void fInitialize( u32 bufferSize, HANDLE enumeratorHandle );
		b32 fBegin( );
		void fReset( );

		u32 fResultCount( ) const { return mResultCount; }
		
		template<class T>
		const T * fResults( ) const { return (T*)mResultBuffer.fBegin( ); }

	private:

		HANDLE mEnumeratorHandle;
		tGrowableBuffer mResultBuffer;
		DWORD mResultCount; // N.B. number of actual results, not mResultBuffer's capacity
	};
}}


#endif//__XtlUtil__

#else

#ifndef XCONTENT_MAX_FILENAME_LENGTH
	struct XCONTENT_CROSS_TITLE_DATA;
	#define XCONTENT_MAX_FILENAME_LENGTH 42
#endif

#endif//#if defined( platform_xbox360 )
