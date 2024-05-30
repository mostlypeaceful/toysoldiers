#ifndef __tStringPtr__
#define __tStringPtr__
#include "tSharedStringBase.hpp"

namespace Sig
{
	class base_export tStringPtrString : public tSharedStringBase
	{
		define_class_pool_new_delete(tStringPtrString,256);
	public:
		static tSharedStringTable& fAccessTable( );
		static tSharedStringBase* fNewInstance( const char* str ) { return NEW tStringPtrString( str ); }
		explicit tStringPtrString( const char* str ) : tSharedStringBase( str ) { }
		~tStringPtrString( ) { fOnDestroy( fAccessTable( ) ); }
	};
	typedef tRefCounterPtr< tStringPtrString > tStringPtrStringPtr;

	///
	/// \brief Encapsulates the idea of a const string identifier which can be compared
	/// in constant time (pointer comparison), and use very little memory (essentially
	/// the size of a pointer). The overhead comes in a global string table which stores
	/// one instance of any given unique string.
	///
	/// \note Important! This class may or may not be case sensitive depending on the define sig_sharedstring_case_sensitive.
	/// If not case sensitive, then the two strings "sTrInG" and "StRiNg" are considered equivalent.
	/// If the former had already been inserted into the string table, and
	/// you then constructed a tStringPtr with the latter, and then called fCStr( ), you
	/// might be surprised to see that the former was returned. Just beware, this class is
	/// really intended for quick and lightweight identifiers with a lot of the flexibility of
	/// strings; NOT necessarily for human-readable text.
	class base_export tStringPtr
	{
		sig_make_stringstreamable( tStringPtr, ( fCStr( ) ? fCStr( ) : "(null)" ) );
	private:
		tStringPtrStringPtr mStringRep;

	public:

		inline tStringPtr( ) { }

		///
		/// \brief This constructor will search the global string table for a case-insensitive
		/// match of 'str'. If found, this instance will be initialized with a pointer to the
		/// existing string; if not found, a new string will be added to the global string table,
		/// and this instance will be set to point to it.
		explicit tStringPtr( const char* str );
		explicit tStringPtr( const std::string& str );

		~tStringPtr( );
		tStringPtr(const tStringPtr& other);
		tStringPtr& operator=(const tStringPtr& other);

		inline const char*	fCStr( ) const		{ return mStringRep.fNull( ) ? "" : mStringRep->fCStr( ); }
		inline u32			fLength( ) const	{ return mStringRep.fNull( ) ? 0  : mStringRep->fLength( ); }
		inline b32			fNull( ) const		{ return mStringRep.fNull( ); }
		inline b32			fExists( ) const	{ return mStringRep.fNull( ) ? false : mStringRep->fExists( ); }

		inline tHashTablePtrInt fGetHashValue( ) const { return ( tHashTablePtrInt )( size_t )mStringRep.fGetRawPtr( ); }	

#define overload_comparison_operator( opType ) \
		inline b32 operator opType( const tStringPtr& other ) const { return mStringRep opType other.mStringRep; }
		overload_comparison_operator( == );
		overload_comparison_operator( != );
		overload_comparison_operator( < );
		overload_comparison_operator( <= );
		overload_comparison_operator( > );
		overload_comparison_operator( >= );
#undef overload_comparison_operator

		static const tStringPtr cNullPtr;

	};

	template<>
	class base_export tHash<tStringPtr>
	{
	public:
		inline static u32 fHash( const tStringPtr& key, const u32 maxSize )
		{
			return tHash<tHashTablePtrInt>::fHash( key.fGetHashValue( ), maxSize );
		}
	};

}

#endif//__tStringPtr__
