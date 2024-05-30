#include "BasePch.hpp"
#include "tSharedStringTable.hpp"
#include "tGlobalCriticalSectionSingleton.cpp"

namespace Sig
{
	tSharedStringTable& tFilePathPtrString::fAccessTable( ) { static tSharedStringTable table; return table; }
}

namespace Sig
{
	namespace
	{
		typedef tFixedArray<char,256> tScratchBuffer;

		inline b32 fIgnore( char ch )
		{
			return ch == '\"';
		}
		inline b32 fIsSlash( char ch )
		{
			return ch == '\\' || ch == '/';
		}
		static void fSanitizePath( tScratchBuffer& scratchBuffer, const char* path, tPlatformId pid )
		{
			sigassert( path );

			const u32 strLen = ( u32 )strlen( path );
			const char platformSlash = fPlatformFilePathSlash( pid );

			u32 iout = 0;

			for( u32 i = 0; i < strLen; ++i )
			{
				if( fIgnore( path[i] ) )
					continue;

				if( fIsSlash( path[i] ) )
				{
					if( fIsSlash( path[i+1] ) )
					{
						// two slashes in a row, skip to the next character
						continue;
					}

					// correct the slash; also, we don't accept slashes at the start or end
					if( iout > 0 && i != strLen - 1 )
						scratchBuffer[ iout++ ] = platformSlash;
				}
				else
				{
					// output lower-case value
					scratchBuffer[ iout++ ] = tolower( path[i] );
				}
			}

			scratchBuffer[ iout++ ] = '\0';
		}
		static tFilePathPtrString* fNewFilePathPtrString( const char* str )
		{
			tSharedStringMutex threadSafe( tGlobalCriticalSectionSingleton::fInstance( ) );
			return static_cast< tFilePathPtrString* >( tFilePathPtrString::fAccessTable( ).fFindString( tFilePathPtrString::fNewInstance, str ) );
		}
		static tFilePathPtrString* fNewFilePathPtrStringSanitized( const char* str, tPlatformId pid )
		{
			tSharedStringMutex threadSafe( tGlobalCriticalSectionSingleton::fInstance( ) );
			tScratchBuffer scratchBuffer;
			fSanitizePath( scratchBuffer, str, pid );
			return static_cast< tFilePathPtrString* >( tFilePathPtrString::fAccessTable( ).fFindString( tFilePathPtrString::fNewInstance, scratchBuffer.fBegin( ) ) );
		}
	}

	const tFilePathPtr tFilePathPtr::cNullPtr;

	tFilePathPtr tFilePathPtr::fConstructPath( const tFilePathPtr* pathFragments, u32 numFragments, tPlatformId pid )
	{
		tScratchBuffer scratchBuffer;

		u32 iout = 0;
		const char platformSlash = fPlatformFilePathSlash( pid );

		for( u32 i = 0; i < numFragments; ++i )
		{
			fMemCpy( &scratchBuffer[ iout ], pathFragments[ i ].fCStr( ), pathFragments[ i ].fLength( ) );
			iout += pathFragments[ i ].fLength( );

			if( i < numFragments - 1 && pathFragments[ i ].fExists( ) && pathFragments[ i + 1 ].fExists( ) )
				scratchBuffer[ iout++ ] = platformSlash;
		}

		scratchBuffer[ iout++ ] = '\0';

		const char* path = scratchBuffer.fBegin( );

		sigassert( scratchBuffer.fCount( ) >= iout );
		return tFilePathPtr( tGuaranteeTag( ), path );
	}

	tFilePathPtr tFilePathPtr::fConstructPath( const tFilePathPtr& frag0, const tFilePathPtr& frag1, tPlatformId pid )
	{
		tScratchBuffer scratchBuffer;

		u32 iout = 0;
		const char platformSlash = fPlatformFilePathSlash( pid );

		fMemCpy( &scratchBuffer[ iout ], frag0.fCStr( ), frag0.fLength( ) );
		iout += frag0.fLength( );
		if( frag0.fExists( ) && frag1.fExists( ) )
			scratchBuffer[ iout++ ] = platformSlash;

		fMemCpy( &scratchBuffer[ iout ], frag1.fCStr( ), frag1.fLength( ) );
		iout += frag1.fLength( );

		scratchBuffer[ iout++ ] = '\0';

		const char* path = scratchBuffer.fBegin( );

		sigassert( scratchBuffer.fCount( ) >= iout );
		return tFilePathPtr( tGuaranteeTag( ), path );
	}

	tFilePathPtr tFilePathPtr::fConstructPath( const tFilePathPtr& frag0, const tFilePathPtr& frag1, const tFilePathPtr& frag2, tPlatformId pid )
	{
		tScratchBuffer scratchBuffer;

		u32 iout = 0;
		const char platformSlash = fPlatformFilePathSlash( pid );

		fMemCpy( &scratchBuffer[ iout ], frag0.fCStr( ), frag0.fLength( ) );
		iout += frag0.fLength( );
		if( frag0.fExists( ) && frag1.fExists( ) )
			scratchBuffer[ iout++ ] = platformSlash;

		fMemCpy( &scratchBuffer[ iout ], frag1.fCStr( ), frag1.fLength( ) );
		iout += frag1.fLength( );
		if( frag1.fExists( ) && frag2.fExists( ) )
			scratchBuffer[ iout++ ] = platformSlash;

		fMemCpy( &scratchBuffer[ iout ], frag2.fCStr( ), frag2.fLength( ) );
		iout += frag2.fLength( );

		scratchBuffer[ iout++ ] = '\0';

		const char* path = scratchBuffer.fBegin( );

		sigassert( scratchBuffer.fCount( ) >= iout );
		return tFilePathPtr( tGuaranteeTag( ), path );
	}

	tFilePathPtr tFilePathPtr::fSwapExtension( const tFilePathPtr& path, const char* newExtension, tPlatformId pid )
	{
		std::string newPath = StringUtil::fStripExtension( path.fCStr( ) );
		newPath += newExtension;
		return tFilePathPtr( newPath.c_str( ), pid );
	}

	tFilePathPtr::tFilePathPtr( const char* str, tPlatformId pid )
		: mStringRep( fNewFilePathPtrStringSanitized( str, pid ) )
	{
	}
	tFilePathPtr::tFilePathPtr( const std::string& str, tPlatformId pid )
		: mStringRep( fNewFilePathPtrStringSanitized( str.c_str( ), pid ) )
	{
	}
	tFilePathPtr::tFilePathPtr( const tStringPtr& str, tPlatformId pid )
		: mStringRep( fNewFilePathPtrStringSanitized( str.fCStr( ), pid ) )
	{
	}
	tFilePathPtr::tFilePathPtr( tGuaranteeTag, const char* str )
		: mStringRep( fNewFilePathPtrString( str ) )
	{
	}
	tFilePathPtr::~tFilePathPtr( )
	{
		tSharedStringMutex threadSafe( tGlobalCriticalSectionSingleton::fInstance( ) );
		mStringRep.fRelease( );
	}
	tFilePathPtr::tFilePathPtr(const tFilePathPtr& other)
	{
		tSharedStringMutex threadSafe( tGlobalCriticalSectionSingleton::fInstance( ) );
		mStringRep = other.mStringRep;
	}
	tFilePathPtr& tFilePathPtr::operator=(const tFilePathPtr& other)
	{
		tSharedStringMutex threadSafe( tGlobalCriticalSectionSingleton::fInstance( ) );
		if( this != &other )
			mStringRep = other.mStringRep;
		return *this;
	}

}
