#ifndef __tDevMenuIni__
#define __tDevMenuIni__
#ifndef __tDevMenu__
#error Must not be included directly! Include tDevMenu.hpp instead.
#endif//__tDevMenu__

namespace Sig
{

	///
	/// \brief A tDevMenuIni object represents a list of dev menu variable (name, override value) pairs.
	///
	/// These values can be specified directly in code, from string, or from file. Native support for ini file loading
	/// is provided via tApplication. An example of an ini file/string might be:
	///		DevMenu_Colors_Background = rgba( 0, 1, 0.5, 1 ) # this is a comment; the "rgba( )" part is optional, but is informative
	///		SomeMenu_SomeInteger = 3
	///		SomeMenu_SomeVector3 = 1, 2, 3 # this will initialize x,y,z to 1,2,3 respectively
	///		#This line is commented out
	/// ...etc...
	/// Variable names and their values should appear on the same line. The value specified for a given variable
	/// in a tDevMenuIni object will be used to override the value that is specified in code using the "devvar" macro. Adding
	/// unrecognized variable names to the ini file will not actually cause those values to be added to the menu, they will
	/// just be ignored.
	class tDevMenuIni : public tUncopyable, public tRefCounter
	{
	public:
		struct tEntry
		{
			tStringPtr		mName;
			tStringPtr		mStringValue;
			Math::tVec4f	mValue;
		};
#ifdef sig_devmenu
	private:
		typedef tHashTable<tStringPtr, tEntry, tHashTableNoResizePolicy> tEntryTable;
		tEntryTable mEntries;
	public:
		tDevMenuIni( );
		~tDevMenuIni( );
		b32 fReadFile( const tFilePathPtr& path );
		b32 fParseString( char* text );
		void fStoreEntries( const tGrowableArray<tEntry>& entries );
		const tEntry* fFindEntry( const tStringPtr& fullPath ) const { return mEntries.fGetItemCount( ) == 0 ? 0 : mEntries.fFind( fullPath ); }
#else//sig_devmenu
	public:
		inline b32 fReadFile( const tFilePathPtr& path ) { return true; }
		inline b32 fParseString( char* text ) { return true; }
		inline void fStoreEntries( const tGrowableArray<tEntry>& entries ) { }
		inline const tEntry* fFindEntry( const tStringPtr& fullPath ) const { return 0; }
#endif//sig_devmenu
	};

	typedef tRefCounterPtr< tDevMenuIni > tDevMenuIniPtr;

}

#endif//__tDevMenuIni__
