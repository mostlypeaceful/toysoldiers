#include "BasePch.hpp"
#include "tResourceLoadList2.hpp"
#include "tApplication.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tResourceLoadList2
	//------------------------------------------------------------------------------
	void tResourceLoadList2::fAdd( const tResourceId& rid )
	{
		sigcheckfail( rid.fGetPath( ).fLength( ) && "Attempting to load an invalid file!", return );

		tResourcePtr res = tApplication::fInstance( ).fResourceDepot( )->fQuery( rid );
		log_sigcheckfail( res, "Resource could not be found: " << rid, return );

		res->fLoadDefault( this );
		mResources.fFindOrAdd( res );
	}

	//------------------------------------------------------------------------------
	b32 tResourceLoadList2::fDone( ) const
	{
		u32 total, complete;
		fCount( total, complete );
		return total == complete;
	}

	//------------------------------------------------------------------------------
	f32 tResourceLoadList2::fPercentComplete( ) const
	{
		u32 total, complete;
		fCount( total, complete );

		if( total == 0 )
			return 1.0f; // avoid potential div by 0

		return complete / (f32)total;
	}

	//------------------------------------------------------------------------------
	void tResourceLoadList2::fCount( u32& total, u32& complete ) const
	{
		total = mResources.fCount( );
		complete = 0;
		for( u32 i = 0; i < total; ++i )
		{
			if( mResources[ i ]->fLoaded( ) )
				++complete;
		}
	}
}
