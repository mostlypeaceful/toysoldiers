//------------------------------------------------------------------------------
// \file Http.cpp - 16 Jul 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "Http.hpp"

namespace Sig { namespace Http 
{
	//------------------------------------------------------------------------------
	// tEndpoint
	//------------------------------------------------------------------------------
	tEndpoint::tEndpoint( )
		: mHostUrl( NULL )
	{
		fPlatformCtor( );
	}

	tEndpoint::~tEndpoint( )
	{
		fDestroy( );
	}

	tRequestPtr tEndpoint::fMakeSyncRequest( const char* verb, const char* path, const char* contentHeader, const char* data, u32 dataSize, u32 responseBufferSize )
	{
		tRequestPtr request = fOpenRequest( verb, path, contentHeader, data, dataSize, responseBufferSize );

		// Tick until the request completes
		while( request && !request->fIsComplete( ) )
			fTick( );

		return request;
	}

	//------------------------------------------------------------------------------
	// tRequest
	//------------------------------------------------------------------------------
	tRequest::tRequest( u32 responseBufferSize )
		: mState( cStateIdle )
		, mVerb( NULL )
		, mHttpStatusCode( 0 )
		, mContentLength( 0 )
		, mContentLengthRemaining( 0 )
		, mBytesRead( 0 )
		, mBytesToRead( 0 )
	{
		mReadBuffer.fResize( responseBufferSize );
		mETag.fResize( cMaxHeaderLength );
		mHeader.fResize( cMaxHeaderLength );

		fPlatformCtor( );
	}
	tRequest::~tRequest( )
	{
		fClose( );
	}

	b32 tRequest::fIsComplete( ) const
	{
		return ( mState == cStateCompleted || mState == cStateError );
	}

	b32 tRequest::fSuccess( ) const
	{
		return mState == cStateCompleted;
	}

	b32 tRequest::fFailed( ) const
	{
		return mState == cStateError;
	}

	void tRequest::fSetState(tState state)
	{
		//log_line( Log::cFlagNetwork, "[Http] state change to: " << state ); 
		mState = state;
	}

	const byte* tRequest::fReadBuffer( ) const
	{
		return mReadBuffer.fBegin( );
	}

	u32 tRequest::fReadBufferSize( ) const
	{
		return mReadBuffer.fTotalSizeOf( );
	}

	u32 tRequest::fHTTPStatusCode( ) const
	{
		return mHttpStatusCode;
	}

	u32 tRequest::fContentLength( ) const
	{
		return mContentLength;
	}

	u32 tRequest::fContentLengthRemaining( ) const
	{
		return mContentLengthRemaining;
	}

	const char* tRequest::GetETagHeader( ) const
	{
		if( strlen( mETag.fBegin( ) ) == 0 )
		{
			return NULL;
		}

		return mETag.fBegin( );
	}

	//------------------------------------------------------------------------------
	// tSystem
	//------------------------------------------------------------------------------
	tSystem::tSystem( )
		: mIsInitialized( false )
		, mStartupFlags( 0 )
	{
		mEndpointsByUserIdx.fResize( tUser::cMaxLocalUsers );
		fPlatformCtor( );
	}

	b32 tSystem::fInitialize( u32 startupFlags, const tStringPtr& userAgent )
	{
		sigassert( userAgent.fExists( ) && "userAgent is invalid" );

		// Store startup params so we can re-initialize if necessary
		mStartupFlags = startupFlags;
		mUserAgent = userAgent;

		mIsInitialized = fPlatformInitialize( );
		return mIsInitialized;
	}

	void tSystem::fShutdown( )
	{
		// Remove all endpoints
		for( u32 u = 0; u < tUser::cMaxLocalUsers; ++u )
			fRemoveEndpointsForUserIdx( u );

		fPlatformShutdown( );

		mIsInitialized = false;
	}

	void tSystem::fTick( )
	{
		profile_pix( "Http::tSystem::fTick" );
		for( u32 userIdx = 0; userIdx < mEndpointsByUserIdx.fCount( ); ++userIdx )
		{
			tEndpointList& endpoints = mEndpointsByUserIdx[ userIdx ];
			for( u32 i = 0; i < endpoints.fCount( ); ++i )
				endpoints[ i ]->fTick( );
		}
	}

	void tSystem::fOnUserSigninChange( const tUserSigninInfo oldStates[], const tUserSigninInfo newStates[] )
	{
		for( u32 u = 0; u < tUser::cMaxLocalUsers; ++u )
		{
			tUser::tSignInState oldState = oldStates[ u ].mState;
			tUser::tSignInState newState = newStates[ u ].mState;

			// We've gone from online to offline
			if( oldState == tUser::cSignInStateSignedInOnline &&
				newState != tUser::cSignInStateSignedInOnline )
			{
				fRemoveEndpointsForUserIdx( u );
			}
		}
	}

	void tSystem::fRemoveEndpointsForUserIdx( u32 userIdx )
	{
		// Explicitly call fDestroy( ) to invalidate the endpoints.
		// This ensures that the endpoint cannot be used via any existing tEndpointPtr variables
		tEndpointList& endpoints = mEndpointsByUserIdx[ userIdx ];
		for( u32 i = 0; i < endpoints.fCount( ); ++i )
			endpoints[ i ]->fDestroy( );

		endpoints.fDeleteArray( );
	}

	void tSystem::fRemoveEndpoint( tEndpoint* endpoint )
	{
		for( u32 userIdx = 0; userIdx < mEndpointsByUserIdx.fCount( ); ++userIdx )
		{
			tEndpointList& endpoints = mEndpointsByUserIdx[ userIdx ];
			for( u32 i = 0; i < endpoints.fCount( ); ++i )
			{
				if( endpoints[ i ] == endpoint )
				{
					// Explicitly call fDestroy( ) to invalidate the endpoint.
					// This ensures that the endpoint cannot be used via any existing tEndpointPtr variables
					endpoint->fDestroy( );
					endpoints.fErase( i );

					break;
				}
			}
		}
	}

} } // ::Sig::Http
