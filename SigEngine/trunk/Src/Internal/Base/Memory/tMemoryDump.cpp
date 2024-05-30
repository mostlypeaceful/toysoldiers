#include <BasePch.hpp>
#include <Memory/tMemoryDump.hpp>
#include <tXmlSerializer.hpp>
#include <tXmlDeserializer.hpp>
#include "tFileWriter.hpp"
#include "FileSystem.hpp"
#include "GameArchiveString.hpp"

namespace Sig { namespace Memory 
{
	namespace
	{
		static const f32 cOneMB = 1024.f * 1024.f;
	}

	tStampDump::tStampDump( const tAllocStamp& stamp, u32 address, tMemoryDump& dump )
		: mCPPFile( dump.fAddString( stamp.mCppFile ) )
		, mTypeName( dump.fAddString( stamp.mTypeName ) )
		, mContext( dump.fAddString( stamp.mContext ) )
		, mLineNum( stamp.mLineNum )
		, mAddress( address )
		, mSize( stamp.mSize )
		, mUserString( dump.fAddString( stamp.mUserString.fStr( ) ? stamp.mUserString.fStr( )->c_str( ) : "" ) )
	{ }

	std::string tStampDump::fSourceFileName( const tMemoryDump& dump ) const
	{
		return dump.fGetString( mCPPFile );
	}

	std::string tStampDump::fTypeString( const tMemoryDump& dump ) const
	{
		return dump.fGetString( mTypeName );
	}

	std::string tStampDump::fContextString( const tMemoryDump& dump ) const
	{
		return dump.fGetString( mContext );
	}

	std::string tStampDump::fLineNumString( ) const
	{
		return StringUtil::fToString( mLineNum );
	}

	std::string tStampDump::fSizeString( ) const
	{
		return StringUtil::fToString( mSize / cOneMB )  + " MB";
	}

	std::string tStampDump::fUserString( const tMemoryDump& dump ) const
	{
		return dump.fGetString( mUserString );
	}

	std::string tStampDump::fToSoftString( const tMemoryDump& dump ) const
	{
		std::string str;

		if( fTypeString( dump ).length( ) )
			str += fTypeString( dump );
		else
			str += fSourceFileName( dump );

		return str;
	}

	std::string tStampDump::fToString( const tMemoryDump& dump ) const
	{
		std::string str;
		
		str += "Alloc:\n";
		str += " File: " + fSourceFileName( dump ) + "\n";
		str += " Line: " + fLineNumString( ) + "\n";

		if( fTypeString( dump ).length( ) )
			str += " Type: " + fTypeString( dump ) + "\n";

		if( fContextString( dump ).length( ) )
			str += " Context: " + fContextString( dump ) + "\n";

		str += " Size: " + fSizeString( ) + "\n";

		if( fUserString( dump ).length( ) )
			str += " UserString: " + fUserString( dump ) + "\n";

		return str;
	}

	std::string tAllocDump::fToSoftString( const tMemoryDump& dump ) const
	{
		return mStamp.fToSoftString( dump );
	}

	std::string tAllocDump::fToString( const tMemoryDump& dump ) const
	{
		return mStamp.fToString( dump );
	}

	std::string tPageDump::fToSoftString( const tMemoryDump& dump ) const
	{
		std::string str;

		str += "%Used: " + StringUtil::fToString( mPercentUsed );

		return str;
	}

	std::string tPageDump::fToString( const tMemoryDump& dump ) const
	{
		std::string str( "Page:\n" );
		str += " Size:  " + StringUtil::fToString( mSize / cOneMB )  + " MB\n";
		str += " %Used: " + StringUtil::fToString( mPercentUsed ) + "\n";
		str += " %Frag: " + StringUtil::fToString( mPercentFragmentation ) + "\n";

		return str;
	}


	tHeapDump::tHeapDump( const char* name, u32 type, u32 size, f32 percentUsed, tMemoryDump& dump )
		: mName( dump.fAddString( name ) )
		, mType( type )
		, mTotalSize( size )
		, mPercentUsed( percentUsed )
	{ }

	std::string tHeapDump::fName( const tMemoryDump& dump ) const
	{
		return dump.fGetString( mName );
	}

	std::string tHeapDump::fTypeString( const tMemoryDump& dump ) const
	{
		std::string str;

		if( mType == cTypeChunky ) 
			str += "Chunky Heap";
		else if( mType == cTypePool )
			str += "Pool Heap";
		else
			str += "Unknown Pool Type";

		return str;
	}

	std::string tHeapDump::fToSoftString( const tMemoryDump& dump ) const
	{
		std::string str;
		std::string name = dump.fGetString( mName );

		str += fTypeString( dump ) + ": " + name;

		return str;
	}

	std::string tHeapDump::fToString( const tMemoryDump& dump ) const
	{
		std::string str = fTypeString( dump ) + ":\n";
		std::string name = dump.fGetString( mName );

		str += " Name:  " + name + "\n";
		str += " Size:  " + StringUtil::fToString( mTotalSize / cOneMB )  + " MB\n";
		str += " Pages: " + StringUtil::fToString( mPages.fCount( ) )  + "\n";
		str += " %Used: " + StringUtil::fToString( mPercentUsed ) + "\n";

		return str;
	}

	namespace
	{
		struct tStringEntry
		{
			u32 mKey;
			std::string mString;

			tStringEntry( )
			{ }

			tStringEntry( u32 key, const std::string& str )
				: mKey( key )
				, mString( str )
			{ }


			template<class tArchive>
			void fSaveLoad( tArchive& archive )
			{
				archive.fSaveLoad( mKey );
				archive.fSaveLoad( mString );
			}
		};

		template<class tSerializer>
		void fSerializeXmlObject( tSerializer& s, tStringEntry& o )
		{
			s( "K", o.mKey );
			s( "S", o.mString );
		}
	}

	template<class tArchive>
	void tMemoryDump::fSaveLoad( tArchive& archive )
	{
		archive.fSaveLoad( mHeaps );
		archive.fSaveLoad( mStringTable );
	}



	u32 tMemoryDump::fAddString( const char* str )
	{
		if( !str )
			str = "";

		// see if it's already in table via hash.
		std::string strstr( str ); 
		u32* id = mTempHash.fFind( strstr );
		if( id )
			return *id;

		// Add it to table and hash.
		u32 index = mStringTable.fCount( );
		mStringTable.fPushBack( strstr );
		mTempHash.fInsert( strstr, index );
		return index;
	}

	const std::string& tMemoryDump::fGetString( u32 key ) const
	{
		sigassert( key < mStringTable.fCount( ) && "String wasn't in string table!" );
		return mStringTable[ key ];
	}

	void tMemoryDump::fSave( const tFilePathPtr& path )
	{
		tFileWriter o( path );

		tGameArchiveSave save;
		save.fBuffer( ).fSetCapacity( 10 * 1024 * 1024 );

		save.fSave( *this, 0 );
		o( save.fBuffer( ).fBegin( ), save.fBuffer( ).fCount( ) );

		//// have to save this out in 1mb chunks apparent or it wont all get saved.
		//const u32 cPassSize = 1024 * 1024;
		//u32 passes = fRoundUp<u32>( save.fBuffer( ).fCount( ) / (f32)cPassSize );

		//u32 offset = 0;
		//for( u32 i = 0; i < passes; ++i )
		//{
		//	u32 saveAmount = fMin( save.fBuffer( ).fCount( ) - offset, cPassSize );
		//	o( save.fBuffer( ).fBegin( ) + offset, saveAmount );
		//	offset += saveAmount;
		//}
	}

	b32 tMemoryDump::fLoad( const tFilePathPtr& path )
	{
		tDynamicBuffer dataBuffer;

		if( !FileSystem::fReadFileToBuffer( dataBuffer, path ) )
			return false;

		return fLoad( dataBuffer );
	}

	b32 tMemoryDump::fLoad( const tDynamicArray<byte>& data )
	{
		tGameArchiveLoad load( data.fBegin( ), data.fCount( ) );
		load.fLoad( *this );

		return !load.fFailed( );
	}

} }