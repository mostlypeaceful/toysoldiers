#ifndef __tAsyncFileReader__
#define __tAsyncFileReader__

namespace Sig
{

	class tAsyncFileReadQueue;
	class tAsyncFileReader;
	typedef tRefCounterPtr< tAsyncFileReader > tAsyncFileReaderPtr;

	///
	/// \brief Encapsulates asynchronous file read I/O operations.
	class base_export tAsyncFileReader : public tUncopyable, public tRefCounter
	{
		friend class tAsyncFileReaderQueue;

	public:

		enum tState
		{
			cStateNull, 
			cStateOpening,
			cStateOpenSuccess,
			cStateOpenFailure,
			cStateReading,
			cStateReadSuccess,
			cStateReadFailure,
			cStateCount
		};

		struct tReadParams;

		//typedef void (*tFreeRecvBuffer)( byte* buffer );

		///
		/// \brief Encapsulates the receiving read output buffer, as well as
		/// the function necessary for deallocating the read buffer.
		class base_export tRecvBuffer
		{
			byte*				mBuffer;
			u32					mBytesAllocated;
			//tFreeRecvBuffer		mFreeBuffer;

		public:

			///
			/// \param buffer The already allocated buffer to receive data from the file read. We dont own this
			/// \param bytesAllocated The number of bytes allocated for reading, or, if decompression
			///			is requested, this must be big enough to handle the decompressed buffer.
			inline explicit tRecvBuffer( byte* buffer=0, u32 bytesAllocated=0 )
				: mBuffer( buffer )
				, mBytesAllocated( bytesAllocated )
			{
			}

			inline byte*	fGetBuffer( ) const { return mBuffer; }
			inline u32		fGetBytesAllocated( ) const { return mBytesAllocated; }
			inline byte*	fForgetBuffer( ) { byte* o = mBuffer; mBuffer = 0; mBytesAllocated = 0; return o; }
		};

		typedef void (*tOnReadComplete)( const tAsyncFileReader* theFile, const tReadParams& readParams );

		///
		/// \brief Parameters and options supplied by the call used for file reading.
		///
		/// The same params that are passed in to fRead will be passed back to the caller
		/// in the supplied tOnReadComplete function (if a function is supplied).
		struct base_export tReadParams
		{
			tRecvBuffer			mRecvBuffer;
			u32					mReadByteCount;
			u32					mReadByteOffset;
			b32					mDecompressAfterRead;

			///
			/// \param recvBuffer This buffer must be sized large enough for the read
			///			request.  In the case of an un-compressed (normal) read, the
			///			size of this buffer should be >= readByteCount.  In the case
			///			of a compressed read (which will be decompressed at the end
			///			of the read), this buffer must be large enough for the decompressed 
			///			data.
			///	\param readByteCount If zero, the whole file is read.
			///	\param readByteOffset The offset from the start of the file to read.
			/// \param decompressAfterRead Decompress the results of the read after it is complete.
			tReadParams( 
				const tRecvBuffer& recvBuffer=tRecvBuffer(),
				u32 readByteCount=0,
				u32 readByteOffset=0,
				b32 decompressAfterRead=false ) 
			: mRecvBuffer( recvBuffer )
			, mReadByteCount( readByteCount )
			, mReadByteOffset( readByteOffset )
			, mDecompressAfterRead( decompressAfterRead )
			{
				sigassert( mReadByteCount <= mRecvBuffer.fGetBytesAllocated( ) );
			}
		};

	private:

		u64				mPlatformFileHandle;
		u32				mFileSize;
		tState			mState;
		//b32				mCancel;
		b32				mOwnsFile;
		tReadParams		mReadParams;
		tFilePathPtr	mFileName;

#ifdef sig_logging
		std::string		mDebugContext;
	public:
		void fSetDebugContext( const std::string& context ) { mDebugContext = context; }
		const std::string& fDebugContext( ) const { return mDebugContext; }
#endif//sig_logging

	public:

		///
		/// \brief Create a smart pointer for performing asynchronous reads on a file.
		///
		/// Begins an asynchronous file open operation.
		static tAsyncFileReaderPtr fCreate( const tFilePathPtr& path );

		///
		/// \brief Enqueue an asynchronous read operation. Begin reading from the file, using the supplied readParams.
		/// \note It is invalid to call fRead if fInValidStateForNewRead( ) returns false.
		void fRead( const tReadParams& readParams );


		///
		/// \brief Spawn a 'child' reader (the child will be marked as not owning the file, and hence
		/// will not try to close the file when it goes out of scope.
		/// \note The current object must not be in a busy state, and must have successfully opened
		/// the file, otherwise this method will sigassert.  Additionally, you must ensure that the
		/// parent object stays in scope at least as long as all its children, otherwise the behavior
		/// is undefined (read: crash).
		tAsyncFileReaderPtr fSpawnChild( ) const;

		///
		/// \brief Block (spin idly) until the file is open, or some failure has occurred.
		void			fBlockUntilOpen( );

		///
		/// \brief Block (spin idly) until the file read is complete, or some failure has occurred.
		void			fBlockUntilReadComplete( );

		///
		/// \brief Query whether the file is open or not
		/// \return true if the file is open
		inline b32		fFileIsOpen	( ) const { return mPlatformFileHandle!=0; }

		///
		/// \brief Retrieve the size in bytes of the entire file.
		/// \note This function will only return valid information after 
		/// the file reader has reached the cStateOpenSuccess state, or
		/// if fFileIsOpen( ) returns true.
		inline u32		fGetFileSize( ) const { return mFileSize; }

		///
		/// \brief Retrieve the time-stamp of when the file was last modified.
		u64 fGetLastModifiedTimeStamp( ) const;

		///
		/// \brief Retrieve the current state of the file reader object.
		inline tState	fGetState( ) const { return mState; }

		///
		/// \brief Find out if the file reader is in a valid state to start a new read.
		/// \return true if it is valid to call fRead( ).
		b32				fInValidStateForNewRead( ) const;

		///
		/// \brief Find out if the file reader is currently enqueued for some action 
		/// or undertaking that action (i.e.. opening or reading).
		b32				fInBusyState( ) const;

		///
		/// \brief Find out if the file reader is in some form of failed state.
		b32				fInFailedState( ) const;

		/////
		///// \brief Cancel the current read (if any), and reset to prepare for another read.
		///// \note It is valid to call fCancel at any time.  However, it will have no effect
		///// if the file reader is not currently in cStateOpening or cStateReading.
		//void			fCancel( );

		///
		/// \brief Reclaim the recv buffer that was allocated and passed in to fRead.
		/// \note It is invalid to call this method while the file reader is in a 'busy'
		/// state (i.e., fInBusyState).
		///
		/// If this method is not called, the buffer will be deallocated internally.  This
		/// also means that there is a limited window in which the user can reclaim or
		/// receive the output of the file read.  If a new read is initiated, or if the
		/// file reader goes out of scope and the recv buffer has not been reclaimed, then
		/// it will be deleted.
		byte*			fForgetBuffer( u32* optionalNumBytesAllocatedOut=0 );

	private:

		tAsyncFileReader( );

	public:
		~tAsyncFileReader( );

	private:
		b32				fCreateFileForPlatform( );

		void			fCloseFileForPlatform( );

		b32				fHandleCancelForOpen( );

		b32				fHandleCancelForRead( );

		void			fOpenFileInThread( );

		void			fReadFileInThread( );

		b32				fReadFileInThreadForPlatform( );

		void			fCleanupDanglingAllocations( );
	};

}

#endif//__tAsyncFileReader__

