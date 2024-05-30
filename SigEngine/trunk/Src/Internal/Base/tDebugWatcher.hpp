#ifndef __tDebugWatcher__
#define __tDebugWatcher__

namespace Sig
{
	class base_export tDebugWatcherList
	{
	public:
		tDebugWatcherList( u32 size, const char* name );
		void fAdd( void* );
		void fRemove( void* );
		void fDump( ) const;
		void fDump( std::wstringstream* ss = NULL ) const;
		static void fDumpAllWatchers( std::wstringstream* ss = NULL );
	private:
		const u32 cSize;
		const char* cName;
		volatile u32 mCount;
		tDebugWatcherList* mNext;
#ifdef sig_devmenu
		tRefCounterPtr< tDevVar< bool > > mDevVarVisible;
#endif
	};

	template< class T >
	class tDebugWatcher
	{
	public:
		tDebugWatcher( ) { sList.fAdd( this ); }
		tDebugWatcher( const tDebugWatcher& dw ) { sList.fAdd( this ); }
		~tDebugWatcher( ) { sList.fRemove( this ); }
	private:
		static tDebugWatcherList sList;
	};
	template< class T>
	tDebugWatcherList tDebugWatcher<T>::sList( sizeof(T), typeid(T).name() );

#ifdef sig_logging
#define debug_watch( classtype ) ::Sig::tDebugWatcher<classtype> mDebugWatcher;
#else
#define debug_watch( classtype )
#endif //sig_logging
}

#endif //__tDebugWatcher__
