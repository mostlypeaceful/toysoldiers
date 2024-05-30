#include <BasePch.hpp>
#include <Memory/tMemoryDump.hpp>
#include <tXmlSerializer.hpp>
#include <tXmlDeserializer.hpp>
#include "tFileWriter.hpp"
#include "FileSystem.hpp"
#include "GameArchiveString.hpp"

#undef new


namespace Sig {
namespace Memory 
{
	namespace
	{
		static const f32 cOneMB = 1024.f * 1024.f;
	}

	/*void tAllocStack::fDumpStack( )
	{
		Log::fPrintf( Log::cFlagMemory, "---- fDumpStack( ) ---- \n");
		for( u32 i = mStackDepth; i > 0; --i )
			Log::fPrintf( Log::cFlagMemory, "----Stack: %s__%s \n", mStack[ i ].mA, mStack[ i ].mB );
	}*/

	std::string tStampDump::fToSoftString( ) const
	{
		std::string str;
		if( mTypeName.length( ) )
			str += mTypeName;
		else
			str += mFile;

		return str;
	}

	std::string tStampDump::fToString( ) const
	{
		std::string str( "Alloc:\n" );
		str += " File: " + mFile + "\n";
		str += " Line: " + StringUtil::fToString( mLineNum ) + "\n";
		if( mTypeName.length( ) )
			str += " Type: " + mTypeName;
		if( mContext.length( ) )
			str += " Context: " + mContext;

		return str;
	}

	std::string tAllocDump::fToSoftString( ) const
	{
		return mStamp.fToSoftString( );
	}

	std::string tAllocDump::fToString( ) const
	{
		return mStamp.fToString( );
	}

	std::string tPageDump::fToSoftString( ) const
	{
		std::string str;

		str += "%Used: " + StringUtil::fToString( mPercentUsed );

		return str;
	}

	std::string tPageDump::fToString( ) const
	{
		std::string str( "Page:\n" );
		str += " Size:  " + StringUtil::fToString( mSize / cOneMB )  + " MB\n";
		str += " %Used: " + StringUtil::fToString( mPercentUsed ) + "\n";
		str += " %Frag: " + StringUtil::fToString( mPercentFragmentation ) + "\n";

		return str;
	}

	std::string tHeapDump::fTypeString( ) const
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

	std::string tHeapDump::fToSoftString( ) const
	{
		std::string str;

		str += fTypeString( ) + ": " + mName;

		return str;
	}

	std::string tHeapDump::fToString( ) const
	{
		std::string str = fTypeString( ) + ":\n";

		str += " Name:  " + mName + "\n";
		str += " Size:  " + StringUtil::fToString( mTotalSize / cOneMB )  + " MB\n";
		str += " Pages: " + StringUtil::fToString( mPages.fCount( ) )  + "\n";
		str += " %Used: " + StringUtil::fToString( mPercentUsed ) + "\n";

		return str;
	}


	// Serialization
	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tStampDump& o )
	{
		s( "F", o.mFile );
		s( "T", o.mTypeName );
		s( "C", o.mContext );
		s( "L", o.mLineNum );
		s( "A", o.mAddress );
		s( "S", o.mSize );
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tAllocDump& o )
	{
		s( "St", o.mStamp );
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tPageDump& o )
	{
		s( "S", o.mSize );
		s( "Pu", o.mPercentUsed );
		s( "Pf", o.mPercentFragmentation );
		s( "As", o.mAllocations );
		s( "Fd", o.mFreed );
		s( "Ad", o.mAddress );
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tHeapDump& o )
	{
		s( "Nm", o.mName );
		s( "Tp", o.mType );
		s( "Ts", o.mTotalSize );
		s( "Pu", o.mPercentUsed );
		s( "Pages", o.mPages );
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tMemoryDump& o )
	{
		s( "Heaps", o.mHeaps );
	}


	b32 tMemoryDump::fSaveXml( const tFilePathPtr& path )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Dmpml", *this, false ) )
		{
			log_warning( 0, "Couldn't save Dmpml file [" << path << "]" );
			return false;
		}

		return true;
	}

	b32 tMemoryDump::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Dmpml", *this ) )
		{
			log_warning( 0, "Couldn't load Dmpml file [" << path << "]" );
			return false;
		}

		return true;
	}

	void tMemoryDump::fSave( const tFilePathPtr& path )
	{
		tFileWriter o( path );

		tGameArchiveSave save;
		save.fBuffer( ).fSetCapacity( 50 * 1024 * 1024 );

		save.fSaveLoad( *this );

		o( save.fBuffer( ).fBegin( ), save.fBuffer( ).fCount( ) );
	}

	b32 tMemoryDump::fLoad( const tFilePathPtr& path )
	{
		tDynamicBuffer dataBuffer;

		if( !FileSystem::fReadFileToBuffer( dataBuffer, path ) )
			return false;

		tGameArchiveLoad load( dataBuffer.fBegin( ), dataBuffer.fCount( ) );
		load.fSaveLoad( *this );

		return !load.fFailed( );
	}

} }