#ifndef __tResourceId__
#define __tResourceId__
#include "tHashTable.hpp"

namespace Sig
{
	template<class t>
	class tResourceConvertPath
	{
	public:
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return path; }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tFilePathPtr( ); }
	};

	// NOTE! Anyone who implements this, needs to also add their class id 
	//  to tBinaryFileBaseReader::fWillResize (ghetto)
	template<class t>
	class tResourceLoadBehavior
	{
	public:
		static b32 fWillResizeAfterLoad( ) { return false; }
	};

	///
	/// \brief TODO document
	class base_export tResourceId
	{
		sig_make_loggable( tResourceId, "[" << std::hex << fGetClassId( ) << ", " << fGetPath( ) << "]" );
	public:
		template<class t>
		static inline tResourceId fMake( const char* path )
		{
			return fMake<t>( tFilePathPtr( path ) );
		}

		template<class t>
		static inline tResourceId fMake( const tFilePathPtr& path )
		{
			return tResourceId( Rtti::fGetClassId<t>( ), tResourceConvertPath<t>::fConvertToBinary( path ), tResourceLoadBehavior<t>::fWillResizeAfterLoad( ) );
		}

		static inline tResourceId fMake( Rtti::tClassId cid, const tFilePathPtr& path, b32 willResize )
		{
			return tResourceId( cid, path, willResize );
		}

		inline tResourceId( )
			: mCid( Rtti::cInvalidClassId )
		{
		}

		inline Rtti::tClassId		fGetClassId	( ) const						{ return mCid; }
		inline const tFilePathPtr&	fGetPath	( ) const						{ return mPath; }

		///
		/// \brief True if this type of resource is going to relocate on load.
		inline b32 fWillResizeAfterLoad( ) const								{ return mWillResize; }

		inline b32 operator==( const tResourceId& other ) const
		{
			return mCid == other.mCid && mPath == other.mPath;
		}

		inline b32 operator!=( const tResourceId& other ) const
		{
			return mCid != other.mCid || mPath != other.mPath;
		}
		
	private:
		inline tResourceId( Rtti::tClassId cid, const tFilePathPtr& path, b32 willResize )
			: mCid( cid )
			, mPath( path )
			, mWillResize( willResize )
		{
		}
	
	private:
		Rtti::tClassId	mCid;
		tFilePathPtr	mPath;
		b32				mWillResize;
	};

	template<>
	class tHash<tResourceId> : public tHash<u64>
	{
	public:
		inline u32 operator( )( const tResourceId& key, const u32 maxSize ) const
		{
			const u64* pkey = ( u64* )&key;
			return tHash<u64>::operator( )( *pkey, maxSize );
		}
	};
}


#endif//__tResourceId__

