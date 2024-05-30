#include "BasePch.hpp"

namespace Sig
{
	devvar( bool, Debug_Watcher_SortByCount, false );

#ifdef sig_devmenu
	void fDumpDebugWatchers( tDevCallback::tArgs& args )
	{
		tDebugWatcherList::fDumpAllWatchers( );
	}
	devcb( Debug_Watcher_DumpToLog, "Dump", make_delegate_cfn( tDevCallback::tFunction, fDumpDebugWatchers ) );
#endif

	//------------------------------------------------------------------------------
	namespace
	{
		tDebugWatcherList*& fHead( )
		{
			static tDebugWatcherList* gHead = NULL;
			return gHead;
		}
	}

	//------------------------------------------------------------------------------
	// tDebugWatcherList
	//------------------------------------------------------------------------------	
	tDebugWatcherList::tDebugWatcherList( u32 size, const char* name )
		: cSize( size )
		, cName( name )
		, mCount( 0 )
		, mNext( NULL )
	{
		mNext = fHead( );
		fHead( ) = this;
#ifdef sig_devmenu
		std::stringstream ss;
		ss << "Debug_Watcher_Visible_" << name;
		mDevVarVisible.fReset( NEW_TYPED( tDevVar< bool > )( ss.str().c_str(), false ) );
#endif
	}

	//------------------------------------------------------------------------------
	void tDebugWatcherList::fAdd( void* p )
	{
		interlocked_inc( &mCount );
	}

	//------------------------------------------------------------------------------
	void tDebugWatcherList::fRemove( void* p )
	{
		interlocked_dec( &mCount );
	}

	//------------------------------------------------------------------------------
	void tDebugWatcherList::fDump( std::wstringstream* ss ) const
	{
#ifdef sig_devmenu
		if( !*mDevVarVisible )
			return;
#endif

		const u32 num = mCount;
		if( ss )
			*ss << "[" << cName << "] x" << num << " = " << std::fixed << std::setprecision( 2 ) << Memory::fToMB<f32>( cSize * num ) << "MB" << std::endl;
		else
			log_line( 0, "[" << cName << "] x" << num << " = " << std::fixed << std::setprecision( 2 ) << Memory::fToMB<f32>( cSize * num ) << "MB" );
	}

	//------------------------------------------------------------------------------
	// tDebugWatcherList::fDumpAllWatchers
	//------------------------------------------------------------------------------
	namespace
	{
		struct tDBL
		{
			u32 size;
			tDebugWatcherList* w;
			tDBL( )
				: size( ~0 )
				, w( NULL )
			{
			}
		};
		static b32 fSortDBL( const tDBL& a, const tDBL& b )
		{
			return a.size > b.size;
		}
	}

	//------------------------------------------------------------------------------
	void tDebugWatcherList::fDumpAllWatchers( std::wstringstream* ss )
	{
		tGrowableArray<tDBL> dbls;

		tDebugWatcherList* list = fHead( );
		while( list )
		{
			tDBL& b = dbls.fPushBack( );
			b.w = list;
			if( Debug_Watcher_SortByCount )
				b.size = list->mCount;
			else
				b.size = list->mCount * list->cSize;
			list = list->mNext;
		}

		std::sort(dbls.fBegin(), dbls.fEnd(), fSortDBL);

		for( u32 i = 0; i < dbls.fCount( ); ++i)
			dbls[i].w->fDump( ss );
	}
}
