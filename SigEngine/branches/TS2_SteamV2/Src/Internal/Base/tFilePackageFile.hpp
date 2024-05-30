#ifndef __tFilePackageFile__
#define __tFilePackageFile__

namespace Sig
{

	///
	/// \brief Can store one or more logical files into a single file package.
	///
	/// Improves streaming performance, enables compressed files, as well as 
	/// simplifies the resource management side of distributing a build.
	class base_export tFilePackageFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		implement_rtti_serializable_base_class(tFilePackageFile, 0xF28E6F5E);

	public:

		static const char* fGetFileExtension( );
		static const u32 cVersion;

		struct base_export tFileHeader
		{
			declare_reflector( );

		public:
			enum tFlagsType
			{
				cCompressed			=(1<<0),
				cWillResizeAfterLoad=(1<<1),
			};

			u64					mLastModifiedTimeStamp;
			tDynamicArray<char>	mFileName;
			Rtti::tClassId		mClassId;
			u32					mFlags;
			u32					mRawFileOffset; ///< Offset to this file's raw bytes (starting from the end of the header table)
			u32					mNumRawFileBytes;
			u32					mNumRawFileBytesUncompressed;
			u32					reservedu32[4];

			tFileHeader( );
			tFileHeader( tNoOpTag );
		};

	protected:

		u32							mHeaderTableSize; ///< The size of the file from [0,endOfHeaderTable)
		tDynamicArray<tFileHeader>	mFileHeaders;

	public:

		tFilePackageFile( );
		tFilePackageFile( tNoOpTag );

		inline u32					fGetNumFiles		( )			const { return mFileHeaders.fCount( ); }
		inline const tFileHeader&	fGetFileHeader		( u32 i )	const { return mFileHeaders[i]; }
		inline u32					fGetHeaderTableSize	( )			const { return mHeaderTableSize; }
		u32							fFindFile( const tFilePathPtr& path ) const;

		void fQueryFilePaths( tFilePathPtrList& output, const tFilePathPtr& folder, const tFilePathPtrList& extList = tFilePathPtrList( ) ) const;

	};

}

#endif//__tFilePackageFile__
