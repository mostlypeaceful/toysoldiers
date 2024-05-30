//------------------------------------------------------------------------------
// \file tUserPicServices.cpp - 15 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tUserPicServices.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	// tUserPicServices
	//------------------------------------------------------------------------------
	void tUserPicServices::fInitialize( )
	{

	}

	//------------------------------------------------------------------------------
	void tUserPicServices::fShutdown( )
	{
		fPlatformShutdown( );
	}

	//------------------------------------------------------------------------------
	void tUserPicServices::fProcessST( )
	{
		profile_pix( "tUserPicServices::fProcessST" );

		// Note: Dimension is guess at reasonable max
		tFixedGrowingArray<tPictureKey, 32> toRemove;

		tPictureMap::tIterator itr = mPictureMap.fBegin( );
		tPictureMap::tIterator end = mPictureMap.fEnd( );
		for( ; itr != end; ++itr )
		{
			if( itr->fNullOrRemoved( ) )
				continue;

			tPicture & pic = *itr->mValue;

			switch( pic.mState )
			{
			case cStateNull: 
				{
					// Check that we still have references before starting query
					if( !pic.mExternalRefCount )
					{
						if( toRemove.fCount( ) < toRemove.fCapacity( ) )
						{
							sigassert( toRemove.fIndexOf( itr->mKey ) == -1 );
							toRemove.fPushBack( itr->mKey );
						}
					}	
					else fQueryPic( itr->mKey, pic );
				} break;
			case cStateBusy: 
				{
					fProcessPic( pic );
				} break;
			case cStateReady:
				{
					// Mark for removal if it's no longer referenced
					if( !pic.mExternalRefCount && toRemove.fCount( ) < toRemove.fCapacity( ) )
					{
						sigassert( toRemove.fIndexOf( itr->mKey ) == -1 );
						toRemove.fPushBack( itr->mKey );
					}
				} break;
			}
		}

		// Remove all the dead pictures
		const u32 toRemoveCount = toRemove.fCount( );
		for( u32 p = 0; p < toRemoveCount; ++p )
		{
			if( tPicturePtr * pic = mPictureMap.fFind( toRemove[ p ] ) )
			{
				if( (*pic)->mState == cStateReady )
					fDestroyPic( **pic );

				mPictureMap.fRemove( pic );
			}
			else sigassert( 0 && "Picture to remove does not exist in picture map!" );
		}
	}

	//------------------------------------------------------------------------------
	void tUserPicServices::fDestroyPic( tPicture & picture )
	{
		sigassert( picture.mState == cStateReady && "Sanity!" );

		fDestroyTexture( picture.mTexture );
		picture.mState = cStateNull;
	}

	//------------------------------------------------------------------------------
	tUserPicServices::tPicture * tUserPicServices::fFindPic( const tPictureKey & key )
	{
		sigassert( key.fIsValid( ) );

		if( tPicturePtr * ptr = mPictureMap.fFind( key ) )
			return ptr->fGetRawPtr( );
		return NULL;
	}

	//------------------------------------------------------------------------------
	void tUserPicServices::fAcquirePic( const tPictureKey & key )
	{
		tPicture * pic = fFindPic( key );
		if( !pic )
		{
			pic = NEW_TYPED( tPicture );

			// Build a key with reliable memory
			tPictureKey reliableKey; 
			{
				// Set the size
				reliableKey.mSmall = key.mSmall;

				// Get the string
				const wchar_t * str; u32 count;
				key.mKey.fGetString( str, &count );

				pic->mKeyData.fInitialize( str, count );
		
				// Set the data
				reliableKey.mKey.fSetString( pic->mKeyData.fBegin( ), count );
			}

			mPictureMap.fInsert( reliableKey, tPicturePtr( pic ) );
		}

		pic->mExternalRefCount++;
	}

	//------------------------------------------------------------------------------
	void tUserPicServices::fReleasePic( const tPictureKey & key )
	{
		if( tPicture * pic = fFindPic( key ) )
		{
			sigassert( pic->mExternalRefCount );
			--pic->mExternalRefCount;
		}
	}

	//------------------------------------------------------------------------------
	b32 tUserPicServices::fGetTexture( const tPictureKey & key, Gfx::tTextureReference & texRef )
	{
		if( tPicture * pic = fFindPic( key ) )
		{
			if( pic->mState == cStateReady )
			{
				texRef = pic->mTexture;
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------
	b32 tUserPicServices::fTextureIsReady( const tPictureKey& key )
	{
		if( tPicture * pic = fFindPic( key ) )
		{
			if( pic->mState == cStateReady )
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------
	// tUserPicRef
	//------------------------------------------------------------------------------
	tUserPicRef::tUserPicRef( const tUserPicRef & other )
	{
		if( !other.fNull( ) )
			fReset( other.mKey.mKey, other.mKey.mSmall );
	}

	//------------------------------------------------------------------------------
	b32 tUserPicRef::fGetTexture( Gfx::tTextureReference & tex, Math::tVec2f * outDims ) const
	{
		if( fNull( ) ) 
			return false;

		if( tUserPicServices::fInstance( ).fGetTexture( mKey, tex ) )
		{
			if( outDims )
				outDims->x = outDims->y = ( mKey.mSmall ? 32.f : 64.f ); // Dims from XDK docs
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------
	b32 tUserPicRef::fIsReady( ) const
	{
		if( fNull( ) )
			return false;

		return tUserPicServices::fInstance( ).fTextureIsReady( mKey );
	}

	//------------------------------------------------------------------------------
	void tUserPicRef::fReset( const tUserData & key, b32 _small )
	{
		sigassert( key.fIsSet( ) );

		if( key != mKey.mKey || _small != mKey.mSmall )
		{
			fRelease( );

			mKey.mKey = key;
			mKey.mSmall = _small;

			tUserPicServices::fInstance( ).fAcquirePic( mKey );
		}
	}

	//------------------------------------------------------------------------------
	void tUserPicRef::fRelease( )
	{
		if( !fNull( ) )
		{
			tUserPicServices::fInstance( ).fReleasePic( mKey );
			mKey.mKey.fReset( );
		}
	}

	//------------------------------------------------------------------------------
	tUserPicRef & tUserPicRef::operator=( const tUserPicRef & other )
	{
		fRelease( );

		if( !other.fNull( ) )
			fReset( other.mKey.mKey, other.mKey.mSmall );

		return *this;
	}

	//------------------------------------------------------------------------------
	// tUserProfilePicRef
	//------------------------------------------------------------------------------
	tUserProfilePicRef::tUserProfilePicRef( const tUserProfilePicRef & other )
		: mSmall( other.mSmall )
		, mPic( other.mPic )
		, mProfile( other.mProfile )
	{

	}

	//------------------------------------------------------------------------------
	void tUserProfilePicRef::fUpdateST( )
	{
		if( !mProfile.fNull( ) && mPic.fNull( ) )
		{
			tUserData key;
			if( mProfile.fGetSetting( tUserProfileServices::cProfilePictureKey, key ) )
				mPic.fReset( key, mSmall );
		}
	}

	//------------------------------------------------------------------------------
	void tUserProfilePicRef::fReset( u32 requesterIdx, const tPlatformUserId & userId, b32 _small )
	{
		mSmall = _small;
		mProfile.fReset( requesterIdx, userId );
		mPic.fRelease( ); // Need profile before we can use the pic
	}

	//------------------------------------------------------------------------------
	void tUserProfilePicRef::fRelease( )
	{
		mPic.fRelease( );
		mProfile.fRelease( );
	}

	//------------------------------------------------------------------------------
	tUserProfilePicRef & tUserProfilePicRef::operator=( const tUserProfilePicRef & other )
	{
		fRelease( );
		mSmall = other.mSmall;
		mProfile = other.mProfile;
		mPic = other.mPic;
		
		return *this;
	}

} // ::Sig
