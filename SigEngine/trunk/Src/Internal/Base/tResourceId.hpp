#ifndef __tResourceId__
#define __tResourceId__
#include "tHashTable.hpp"

namespace Sig
{
	///
	/// \brief TODO document
	class base_export tResourceId
	{
		sig_make_stringstreamable( tResourceId, "[" << std::hex << fGetClassId( ) << ", " << fGetPath( ) << "]" );
	public:
		template<class t>
		static inline tResourceId fMake( const char* path )
		{
			return fMake<t>( tFilePathPtr( path ) );
		}

		template<class t>
		static inline tResourceId fMake( const tFilePathPtr& path )
		{
			return tResourceId( Rtti::fGetClassId<t>( ), t::fConvertToBinary( path ) );
		}

		static inline tResourceId fMake( Rtti::tClassId cid, const tFilePathPtr& path )
		{
			return tResourceId( cid, path );
		}

		inline tResourceId( )
			: mCid( Rtti::cInvalidClassId )
		{
		}

		inline Rtti::tClassId		fGetClassId	( ) const						{ return mCid; }
		inline const tFilePathPtr&	fGetPath	( ) const						{ return mPath; }

		inline b32 operator==( const tResourceId& other ) const
		{
			return mCid == other.mCid && mPath == other.mPath;
		}

		inline b32 operator!=( const tResourceId& other ) const
		{
			return mCid != other.mCid || mPath != other.mPath;
		}
		
	private:
		inline tResourceId( Rtti::tClassId cid, const tFilePathPtr& path )
			: mCid( cid )
			, mPath( path )
		{
		}
	
	private:
		Rtti::tClassId	mCid;
		tFilePathPtr	mPath;
	};

	template<>
	class tHash<tResourceId>
	{
	public:
		inline static u32 fHash( const tResourceId& key, const u32 maxSize )
		{
			const u64* pkey = ( u64* )&key;
			return tHash<u64>::fHash( *pkey, maxSize );
		}
	};
}


#endif//__tResourceId__

