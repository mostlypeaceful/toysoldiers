#ifndef _tMemoryDump__
#define _tMemoryDump__

namespace Sig { 

	class tFilePathPtr;

namespace Memory
{

	struct tMemoryDump;

	struct base_export tBaseDump
	{
		virtual ~tBaseDump( ) { }
		virtual std::string fToSoftString( const tMemoryDump& ) const { return "tBaseDump"; }
		virtual std::string fToString( const tMemoryDump& ) const { return "No Data"; }
	};

	struct base_export tStampDump
	{
		// These are indexes into the serialized string table.
		u32 mCPPFile;
		u32 mTypeName;
		u32 mContext;
		u32	mLineNum;
		u32	mAddress;
		u32	mSize;
		u32 mUserString; // for resources and things.

		tStampDump( ) { }
		tStampDump( const tAllocStamp& stamp, u32 address, tMemoryDump& dump );

		b32 operator == ( const tStampDump& other ) const
		{
			return mCPPFile == other.mCPPFile && mTypeName == other.mTypeName && mContext == other.mContext && mLineNum == other.mLineNum;
			// SIZE IS EXCLUDED
		}

		std::string fSourceFileName( const tMemoryDump& dump ) const;
		std::string fTypeString( const tMemoryDump& dump ) const;
		std::string fContextString( const tMemoryDump& dump ) const;
		std::string fLineNumString( ) const;
		//std::string fAddress( ... )
		std::string fSizeString( ) const;
		std::string fUserString( const tMemoryDump& dump ) const;

		std::string fToSoftString( const tMemoryDump& dump ) const;
		std::string fToString( const tMemoryDump& dump ) const;

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mCPPFile );
			archive.fSaveLoad( mTypeName );
			archive.fSaveLoad( mContext );
			archive.fSaveLoad( mLineNum );
			archive.fSaveLoad( mAddress );
			archive.fSaveLoad( mSize );
			archive.fSaveLoad( mUserString );
		}
	};

	struct base_export tAllocDump : public tBaseDump
	{
		tStampDump	mStamp;

		tAllocDump( const tStampDump& stamp = tStampDump( ) )
			: mStamp( stamp )
		{ }

		virtual std::string fToSoftString( const tMemoryDump& dump ) const;
		virtual std::string fToString( const tMemoryDump& dump ) const;

		b32 operator == ( const tAllocDump& other ) const { return mStamp == other.mStamp; }

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mStamp );
		}
	};

	struct base_export tPageDump : public tBaseDump
	{
		u32 mSize;
		f32 mPercentUsed;
		f32 mPercentFragmentation;
		u32 mAddress;

		tGrowableArray<tAllocDump> mAllocations;
		tGrowableArray<tAllocDump> mFreed;

		tPageDump( )
		{ }

		tPageDump( u32 size, f32 percentUsed, f32 framgentation, u32 address )
			: mSize( size )
			, mPercentUsed( percentUsed )
			, mPercentFragmentation( framgentation )
			, mAddress( address )
		{ }

		virtual std::string fToSoftString( const tMemoryDump& dump ) const;
		virtual std::string fToString( const tMemoryDump& dump ) const;

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mSize );
			archive.fSaveLoad( mPercentUsed );
			archive.fSaveLoad( mPercentFragmentation );
			archive.fSaveLoad( mAllocations );
			archive.fSaveLoad( mAddress );
			archive.fSaveLoad( mFreed );
		}
	};

	struct base_export tHeapDump : public tBaseDump
	{
		enum tType 
		{
			cTypePool,
			cTypeChunky
		};

		u32 mName;
		u32 mType;
		u32 mTotalSize;
		f32 mPercentUsed;

		tGrowableArray<tPageDump> mPages;

		tHeapDump( ) { }
		tHeapDump( const char* name, u32 type, u32 size, f32 percentUsed, tMemoryDump& dump );

		std::string fName( const tMemoryDump& dump ) const;
		std::string fTypeString( const tMemoryDump& dump ) const;
		virtual std::string fToSoftString( const tMemoryDump& dump ) const;
		virtual std::string fToString( const tMemoryDump& dump ) const;

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mName );
			archive.fSaveLoad( mType );
			archive.fSaveLoad( mTotalSize );
			archive.fSaveLoad( mPercentUsed );
			archive.fSaveLoad( mPages );
		}
	};

	struct base_export tMemoryDump : public tBaseDump
	{
		// the u32 here is a pointer to the static string. 
		tGrowableArray< std::string > mStringTable;
		tGrowableArray<tHeapDump> mHeaps;

		// this isn't serialized, it's only used to make dumping fast.
		tHashTable<std::string, u32> mTempHash;

		// assumes that like strings have the same memory address.
		u32 fAddString( const char* str );
		const std::string& fGetString( u32 key ) const;

		void fSave( const tFilePathPtr& path );
		b32 fLoad( const tFilePathPtr& path );
		b32 fLoad( const tDynamicArray<byte>& data );

		template<class tArchive>
		void fSaveLoad( tArchive& archive );

		static f32 fComputePercentage( u32 some, u32 total )
		{
			return total > 0 ? (f32)some / total : 0.f;
		}
	};

}}



#endif//_tMemoryDump__
