//------------------------------------------------------------------------------
// \file MemoryUtil.hpp - 28 Sep 2010
// \author mwagner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef _tMemoryDump__
#define _tMemoryDump__

namespace Sig { 

	class tFilePathPtr;

namespace Memory
{

	struct base_export tBaseDump
	{
		virtual ~tBaseDump( ) { }
		virtual std::string fToSoftString( ) const { return "tBaseDump"; }
		virtual std::string fToString( ) const { return "No Data"; }
	};

	struct base_export tStampDump
	{
		std::string mFile;
		std::string mTypeName;
		std::string mContext;
		u32			mLineNum;
		u32			mAddress;
		u32			mSize;

		tStampDump( )
			: mLineNum( 0 )
			, mAddress( 0 )
			, mSize( 0 )
		{ }

		tStampDump( const tAllocStamp& stamp, u32 address )
			: mFile( fCharToStr( stamp.mFile ) )
			, mTypeName( fCharToStr( stamp.mTypeName ) )
			, mContext( fCharToStr( stamp.mContext ) )
			, mLineNum( stamp.mLineNum )
			, mAddress( address )
			, mSize( stamp.mSize )
		{ }

		b32 operator == ( const tStampDump& other )
		{
			return mFile == other.mFile && mTypeName == other.mTypeName && mContext == other.mContext && mLineNum == other.mLineNum;
			// SIZE IS EXCLUDED
		}

		std::string fToSoftString( ) const;
		std::string fToString( ) const;
		std::string fCharToStr( const char* c ) { return c ? std::string( c ) : ""; }

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mFile );
			archive.fSaveLoad( mTypeName );
			archive.fSaveLoad( mContext );
			archive.fSaveLoad( mLineNum );
			archive.fSaveLoad( mAddress );
			archive.fSaveLoad( mSize );
		}
	};

	struct base_export tAllocDump : public tBaseDump
	{
		tStampDump	mStamp;

		tAllocDump( const tStampDump& stamp = tStampDump( ) )
			: mStamp( stamp )
		{ }

		virtual std::string fToSoftString( ) const;
		virtual std::string fToString( ) const;

		b32 operator == ( const tAllocDump& other ) { return mStamp == other.mStamp; }

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

		tPageDump( u32 size = 0, f32 percentUsed = 0.f, f32 framgentation = 0.f, u32 address = 0 )
			: mSize( size )
			, mPercentUsed( percentUsed )
			, mPercentFragmentation( framgentation )
			, mAddress( address )
		{ }

		virtual std::string fToSoftString( ) const;
		virtual std::string fToString( ) const;

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

		std::string mName;
		u32 mType;
		u32 mTotalSize;
		f32 mPercentUsed;

		tGrowableArray<tPageDump> mPages;

		tHeapDump( const std::string& name = "", u32 type = cTypePool, u32 size = 0, f32 percentUsed = 0.f )
			: mName( name )
			, mType( type )
			, mTotalSize( size )
			, mPercentUsed( percentUsed )
		{ }

		std::string fTypeString( ) const;
		virtual std::string fToSoftString( ) const;
		virtual std::string fToString( ) const;

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
		tGrowableArray<tHeapDump> mHeaps;

		b32 fSaveXml( const tFilePathPtr& path );
		b32 fLoadXml( const tFilePathPtr& path );

		void fSave( const tFilePathPtr& path );
		b32 fLoad( const tFilePathPtr& path );

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			const u32 cVersion = 1;
			u32 version = cVersion;

			archive.fSaveLoad( version );
			if( version != cVersion )
			{
				archive.fFail( );
				return;
			}

			archive.fSaveLoad( mHeaps );
		}

		static f32 fComputePercentage( u32 some, u32 total )
		{
			return total > 0 ? (f32)some / total : 0.f;
		}
	};

	//struct base_export tAllocStack
	//{
	//	static tAllocStack& fInstance( )
	//	{
	//		static tAllocStack gGlobalAllocStack;
	//		return gGlobalAllocStack;
	//	}

	//	void fPushStack( const char* name, const char* function )
	//	{
	//		if( mStackDepth < mMaxStackDepth )
	//		{
	//			mStack[ mStackDepth ].mA = name;
	//			mStack[ mStackDepth ].mB = function;
	//			++mStackDepth;
	//		}
	//	}

	//	void fClearStack( )
	//	{
	//		mStackDepth = 0;
	//	}

	//	void fDumpStack( );

	//private:
	//	tAllocStack( )
	//	{
	//		fClearStack( );
	//	}

	//	static const u32 cMaxStackDepth = 10;
	//	tFixedArray< tPair<const char*, const char*>, cMaxStackDepth > mStack;
	//	u32 mStackDepth;
	//};

}}



#endif//_tMemoryDump__
