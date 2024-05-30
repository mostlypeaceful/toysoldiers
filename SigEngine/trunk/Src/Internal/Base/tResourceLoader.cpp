#include "BasePch.hpp"
#include "tResourceLoader.hpp"
#include "tResource.hpp"

namespace Sig
{
	tResourceLoader::tResourceLoader( tResource* res )
		: mResource( res )
		, mCancel( false )
	{
		sigassert( !mResource.fNull( ) );
	}

	void tResourceLoader::fSetFileTimeStamp( u64 timeStamp )
	{
		mResource->mFileTimeStamp = timeStamp;
	}

	void tResourceLoader::fSetResourceBuffer( tGenericBuffer* newDerivedBuffer )
	{
		mResource->mBuffer.fReset( newDerivedBuffer );
	}

	tGenericBuffer* tResourceLoader::fGetResourceBuffer( )
	{
		return mResource->mBuffer.fGetRawPtr( );
	}

	void tResourceLoader::fSetSelfOnResource( )
	{
		sigassert( !mResource->mLoader );
		mResource->mLoader = this;
		mResource->fAddToLoadList( );
	}

	Memory::tAllocStamp tResourceLoader::fMakeStamp( u32 size ) const
	{ 
		return Memory::tAllocStamp( __FILE__, __LINE__, "Resource", "", size, mResource ? mResource->fGetPath( ).fCStr( ) : NULL ); 
	}


}
