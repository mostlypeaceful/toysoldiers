#include "BasePch.hpp"
#include "RttiDetail.hpp"
#include "tCallStack.hpp"
#include "tSymbolHelper.hpp"

namespace Sig { namespace Rtti { namespace Private
{

#if defined( target_tools ) || defined( platform_metro ) || defined( build_debug )

	///
	/// \brief We use full type string compares to be absolutely certain we're not
	/// being duped by different dlls; i.e., different dlls could conceivably return
	/// different type_info objects, which, when performing a pointer compare could fail
	struct tRawStringEqual
	{
		inline static b32 fEquals( const char* a, const char* b )
		{
			return strcmp( a, b ) == 0;
		}
	};

	namespace
	{
		struct tRttiEntry
		{
			tClassId mClassId;

#if defined( build_debug )
			void* mCidObjectAddress;
			tCallStack mCallStack;

			tRttiEntry(): mCallStack(32) {}
#endif
		};
	}

	typedef tHashTable<const char*, tRttiEntry, tHashTableExpandOnlyResizePolicy, tHash<const char*>, tRawStringEqual> tRuntimeClassIdTable;

	tRuntimeClassIdTable& fRuntimeClassIdTable()
	{
		static tRuntimeClassIdTable gRuntimeClassIdTable( 32 );
		return gRuntimeClassIdTable;
	}

	Threads::tCriticalSection& fRttiCS()
	{
		static Threads::tCriticalSection cs;
		return cs;
	}

	tRttiEntry* fAddEntry( const std::type_info& ti )
	{
		static tClassId gNextClassId = cInvalidClassId;

		sigassert(!fRuntimeClassIdTable().fFind(ti.name()));

		tRttiEntry entry;
		entry.mClassId = ++gNextClassId;
		return fRuntimeClassIdTable().fInsert( ti.name(), entry );
	}

	/// Bellow you'll see the pattern:
	/// 
	/// if ( initialized ) return;
	/// *aquire mutex*
	/// if ( initialized ) return;
	/// 
	/// While double checks should be setting off every alarm bell in your
	/// head around multithreaded code, the first check is merely an
	/// optimization for the more common case where we've already initialized.
	/// 
	/// This is sane because we atomically assign the value, and do so while
	/// locked, and check again that this hasn't been asigned after we've
	/// aquired our lock.

#if defined( build_debug )
	void fDebugRegisterClassId( b32& notFirstInit, b32& unique, const std::type_info& ti, void* uuid)
	{
		if( notFirstInit ) return; // already initialized, and we initialize atomically, bail out early.

		Threads::tMutex m( fRttiCS() );

		if( notFirstInit ) return; // This could happen if two threads take the same codepath and initialize the same static vars, since the first check isn't gated by the mutex.

		tRttiEntry* find = fRuntimeClassIdTable().fFind( ti.name( ) );

		if( !find )
		{
			// Completely new registration.
			find = fAddEntry( ti );
			find->mCidObjectAddress = uuid;

			unique = true;
		}
		else if ( find->mCidObjectAddress == uuid )
		{
			// Cause for concern as we should only be called once per template instance, but the addresses matches.
			log_warning( "Duplicate registration but of indentical CID" );

			unique = true;
		}
		else
		{
			// Uh oh, we had a duplicate.
			tSymbolHelper sh("");

			log_warning( "Duplicate registration detected!!!" );
			log_line( 0, "\tOriginal registration information: " << ti.name() << " was registered with a cid object @ " << find->mCidObjectAddress );
			log_line( 0, "\tStack trace of the original registration:" );

			std::string symbol, file;
			for ( u32 i=0 ; i<find->mCallStack.fData().mDepth ; ++i )
			{
				sh.fGetSymbolSummary( find->mCallStack.fData().mAddresses[i], symbol, file );
				log_line( 0, symbol << " @ " << file );
			}

			unique = false;
			sigassert( unique );
		}

		Threads::fAtomicExchange( &notFirstInit, true );
	}
#endif

	void fGetRuntimeClassId( tClassId& cid, const std::type_info& ti)
	{
		if ( cid ) return; // already initialized, and we initialize atomically

		Threads::tMutex m( fRttiCS() );

		if ( cid ) return; // This could happen if two threads take the same codepath and initialize the same static vars, since the first check isn't gated by the mutex.

		const tRttiEntry* find = fRuntimeClassIdTable().fFind( ti.name( ) );

		if( !find )
		{
			find = fAddEntry( ti );
		}

		Threads::fAtomicExchange( &cid, find->mClassId );
	}

#endif // defined( target_tools ) || defined( platform_metro )

	void fStfuAboutNoSymbolsPleaseMrLinker() {}
}}}
