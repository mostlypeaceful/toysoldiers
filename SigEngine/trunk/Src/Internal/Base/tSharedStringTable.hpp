#ifndef __tSharedStringTable__
#define __tSharedStringTable__
#include "tSharedStringBase.hpp"
#include "Threads/tMutex.hpp"

#define sig_sharedstring_case_sensitive

#ifdef sig_sharedstring_thread_safe
namespace Sig
{
	typedef Threads::tCriticalSection tSharedStringCriticalSection;
	typedef Threads::tMutex tSharedStringMutex;
}
#else
namespace Sig
{
	typedef Threads::tDummyMutex::tCriticalSection tSharedStringCriticalSection;
	typedef Threads::tDummyMutex tSharedStringMutex;
}
#endif//sig_sharedstring_thread_safe


namespace Sig
{
	///
	/// \brief Compare two strings case-insensitively and return true if they match.
	struct tCompareStringsCaseInsensitive
	{
		inline static b32 fEquals( const char* a, const char* b )
		{
			return StringUtil::fStricmp( a, b )==0;
		}
	};

	///
	/// \brief Create a hash value from a string (case-insensitive, i.e. two
	/// string sTrInG and StRiNg should produce the same hash).
	struct tHashStringsCaseInsensitive
	{
		inline static u32 fHash( const char* key, const u32 maxSize )
		{
			u32 hash = 5381, c;
			while(( c = tolower( *key++ ) ))
				hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
			return hash % maxSize;
		}
	};

	///
	/// \brief Compare two strings case-sensitively and return true if they match.
	struct tCompareStringsCaseSensitive
	{
		inline static b32 fEquals( const char* a, const char* b )
		{
			return strcmp( a, b )==0;
		}
	};

	///
	/// \brief Create a hash value from a string (case-sensitive, i.e. two
	/// strings sTrInG and StRiNg will NOT produce the same hash).
	struct tHashStringsCaseSensitive
	{
		inline static u32 fHash( const char* key, const u32 maxSize )
		{
			u32 hash = 5381, c;
			while(( c = *key++ ))
				hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
			return hash % maxSize;
		}
	};

	///
	/// \brief The paramaterized hash table type used for storing strings in shared string tables.
	typedef tHashTable<
		const char*,
		tSharedStringBase*,
		tHashTableExpandAndShrinkResizePolicy,
#ifdef sig_sharedstring_case_sensitive
		tHashStringsCaseSensitive,
		tCompareStringsCaseSensitive
#else//sig_sharedstring_case_sensitive
		tHashStringsCaseInsensitive,
		tCompareStringsCaseInsensitive
#endif//sig_sharedstring_case_sensitive
	> tSharedStringTableBase;

	///
	/// \brief Provides global access to the string hash table.
	class base_export tSharedStringTable : public tSharedStringTableBase
	{
	public:
		tSharedStringTable( );
		~tSharedStringTable( );
		tSharedStringBase* fFindString( tNewSharedStringInstance newSharedString, const char* str );
		void fRemoveString( const char* str );
	};
}

#endif//__tSharedStringTable__
