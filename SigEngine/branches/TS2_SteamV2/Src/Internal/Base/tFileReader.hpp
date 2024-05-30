#ifndef __tFileReader__
#define __tFileReader__

namespace Sig
{
	class tFilePathPtr;

	///
	/// \brief Encapsulates simple blocking file reading semantics, along
	/// with nice OO stuff like auto-closing the file on destruction, etc.
	class base_export tFileReader : public tUncopyable
	{
		void* mFile;
	public:
		static u32 fFileSize( const tFilePathPtr& path );
	public:
		tFileReader( );
		explicit tFileReader( const tFilePathPtr& path );
		~tFileReader( );

		b32	 fIsOpen( ) const;
		b32  fOpen( const tFilePathPtr& path );
		void fClose( );
		void fSeek( u32 absolutePos );
		void fSeekFromCurrent( u32 relativePos );
		void fSeekFromEnd( u32 relativePos );
		u32  fTell( ) const;

		///
		/// \brief Read raw data from file.
		/// \brief Returns the number of bytes read.
		u32 operator()( void* data, u32 numBytes );

	};

}


#endif//__tFileReader__

