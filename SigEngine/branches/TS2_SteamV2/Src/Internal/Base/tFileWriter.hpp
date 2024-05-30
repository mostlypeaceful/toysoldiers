#ifndef __tFileWriter__
#define __tFileWriter__

namespace Sig
{
	class tFilePathPtr;

	///
	/// \brief Encapsulates simple blocking file writing semantics, along
	/// with nice OO stuff like auto-closing the file on destruction, etc.
	class base_export tFileWriter : public tUncopyable
	{
		void* mFile;

	public:

		tFileWriter( );
		explicit tFileWriter( const tFilePathPtr& path, b32 append = false );
		~tFileWriter( );

		b32	 fIsOpen( ) const;
		b32  fOpen( const tFilePathPtr& path, b32 append = false );
		void fClose( );
		void fSeek( u32 absolutePos );
		void fSeekFromCurrent( u32 relativePos );
		void fSeekFromEnd( u32 relativePos );
		void fFlush( );

		///
		/// \brief Write raw data to file.
		void operator()( const void* data, u32 numBytes );

	};

}


#endif//__tFileWriter__
