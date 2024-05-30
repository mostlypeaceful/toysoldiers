#ifndef __tFilePathPtr__
#define __tFilePathPtr__
#include "tSharedStringBase.hpp"

namespace Sig
{
	class tStringPtr;
	class tFilePathPtr;

	class base_export tFilePathPtrString : public tSharedStringBase
	{
		define_class_pool_new_delete(tFilePathPtrString,256);
	public:
		static tSharedStringTable& fAccessTable( );
		static tSharedStringBase* fNewInstance( const char* str ) { return NEW tFilePathPtrString( str ); }
		explicit tFilePathPtrString( const char* str ) : tSharedStringBase( str ) { }
		~tFilePathPtrString( ) { fOnDestroy( fAccessTable( ) ); }
	};

	typedef tGrowableArray<tFilePathPtr> tFilePathPtrList;
	typedef tRefCounterPtr< tFilePathPtrString > tFilePathStringPtr;

	///
	/// \brief Encapsulates the idea of a const string-based file path identifier which can be compared
	/// in constant time (pointer comparison), and use very little memory (essentially
	/// the size of a pointer). The overhead comes in a global string table which stores
	/// one instance of any given unique string path.
	/// 
	/// Additionally, this class provides conversion from raw strings to safe path strings, correcting
	/// things like case, slashes, etc. This ensures that any two paths that are logically the same
	/// (i.e., C:\MyPath.txt, C:/mypath.txt) can be treated as "equal".
	class base_export tFilePathPtr
	{
		sig_make_stringstreamable(tFilePathPtr, ( fCStr( ) ? fCStr( ) : "(null)" ) );
	private:
		tFilePathStringPtr mStringRep;

	public:

		///
		/// \brief Passing this tag to the alternative constructor that accepts it informs
		/// the path object to skip sanitizing; essentially, you're saying you're 100% positive
		/// the path is good. Use at your own risk!
		struct tGuaranteeTag { };

		///
		/// \brief Construct a complete path from several fragments, i.e., you might construct the path
		/// a\b\c.txt from three fragments, a, b, and c.txt, each being their own tFilePathPtr.
		static tFilePathPtr fConstructPath( const tFilePathPtr* pathFragments, u32 numFragments, tPlatformId pid = cCurrentPlatform );

		///
		/// \brief Construct a complete path from two fragments. Provided for convenience, the same functionality
		/// can be achieved in full generality with the version that takes an array of fragments.
		static tFilePathPtr fConstructPath( const tFilePathPtr& frag0, const tFilePathPtr& frag1, tPlatformId pid = cCurrentPlatform );

		///
		/// \brief Construct a complete path from three fragments. Provided for convenience, the same functionality
		/// can be achieved in full generality with the version that takes an array of fragments.
		static tFilePathPtr fConstructPath( const tFilePathPtr& frag0, const tFilePathPtr& frag1, const tFilePathPtr& frag2, tPlatformId pid = cCurrentPlatform );

		///
		/// \brief Swap the extension of a path for a different extension.
		static tFilePathPtr fSwapExtension( const tFilePathPtr& path, const char* newExtension, tPlatformId pid = cCurrentPlatform );

		///
		/// \brief Make this path relative to the specified path
		tFilePathPtr fRelativeTo( const tFilePathPtr& path, tPlatformId pid = cCurrentPlatform ) const;

		///
		/// \brief Ctor, does nothing.
		inline tFilePathPtr( ) { }

		///
		/// \brief This constructor will search the global string table for a logical match of 'str'. 
		/// If found, this instance will be initialized with a pointer to the existing string; if not found, 
		/// a NEW "sanitized" string will be added to the global string table, and this instance will be set 
		/// to point to it. "Sanitizing" involves correcting things like case, slashes, etc.
		explicit tFilePathPtr( const char* str, tPlatformId pid = cCurrentPlatform );
		explicit tFilePathPtr( const std::string& str, tPlatformId pid = cCurrentPlatform );
		explicit tFilePathPtr( const tStringPtr& str, tPlatformId pid = cCurrentPlatform );

		///
		/// \brief This alternate constructor acts identically to the others, except that it foregoes
		/// path sanitization, as you're saying you're sure the path contained in 'str' is fine. This constructor
		/// is provided purely for the sake of optimization, it is not recommended you use this unless
		/// you are deserializing a path that was known to be safe at the time of serialization.
		tFilePathPtr( tGuaranteeTag, const char* str );

		~tFilePathPtr( );
		tFilePathPtr(const tFilePathPtr& other);
		tFilePathPtr& operator=(const tFilePathPtr& other);

		inline const char*	fCStr( ) const		{ return mStringRep.fNull( ) ? "" : mStringRep->fCStr( ); }
		inline u32			fLength( ) const	{ return mStringRep.fNull( ) ? 0  : mStringRep->fLength( ); }
		inline b32			fNull( ) const		{ return mStringRep.fNull( ); }
		inline b32			fExists( ) const	{ return mStringRep.fNull( ) ? false : mStringRep->fExists( ); }

		inline tHashTablePtrInt fGetHashValue( ) const { return ( tHashTablePtrInt )( size_t )mStringRep.fGetRawPtr( ); }	
		inline u32 fGetStableHashValue( ) const { return Hash::fGenericHash( reinterpret_cast< const byte* >( fCStr( ) ), fLength( ), ~0u ); }

#define overload_comparison_operator( opType ) \
		inline b32 operator opType( const tFilePathPtr& other ) const { return mStringRep opType other.mStringRep; }
		overload_comparison_operator( == );
		overload_comparison_operator( != );
		overload_comparison_operator( < );
		overload_comparison_operator( <= );
		overload_comparison_operator( > );
		overload_comparison_operator( >= );
#undef overload_comparison_operator

		static const tFilePathPtr cNullPtr;
	};

	template<>
	class base_export tHash<tFilePathPtr>
	{
	public:
		inline static u32 fHash( const tFilePathPtr& key, const u32 maxSize )
		{
			return tHash<tHashTablePtrInt>::fHash( key.fGetHashValue( ), maxSize );
		}
	};
}

#endif//__tFilePathPtr__
