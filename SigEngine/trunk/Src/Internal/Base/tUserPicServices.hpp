//------------------------------------------------------------------------------
// \file tUserPicServices.hpp - 15 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tUserPicServices__
#define __tUserPicServices__

#include "tUser.hpp"
#include "tUserProfileServices.hpp"
#include "Gfx/tTextureReference.hpp"

namespace Sig
{
	///
	/// \class tUserPicServices
	/// \brief 
	class tUserPicServices
	{
		friend class tUserPicRef;
		declare_singleton( tUserPicServices );
	public:

		void fInitialize( );
		void fShutdown( );

		// Advance our picture queries
		void fProcessST( );

	private:
		void fPlatformShutdown( );

		enum tState
		{
			cStateNull,
			cStateBusy,
			cStateReady
		};

		struct tPictureKey
		{
			b32 mSmall;
			tUserData mKey;

			tPictureKey( ) { }
			tPictureKey( b32 _small, const tUserData & key )
				: mSmall( _small ), mKey( key ) { }
			b32 operator==( const tPictureKey & key ) const
			{
				return mSmall == key.mSmall && mKey == key.mKey;
			}

			b32 fIsValid( ) const
			{
				return mKey.fType( ) == tUserData::cTypeUnicode;
			}
		};

		struct tPictureKeyHash
		{
			inline static u32 fHash( const tPictureKey & key, const u32 maxSize )
			{
				u32 hash = Hash::fGenericHash( 
					(const byte*)key.mKey.fDataPtr( ), key.mKey.fDataSize( ), maxSize );
				hash ^= ( key.mSmall ? 0 : 1 );
				return hash % maxSize;
			}
		};

		struct tPicture : public tRefCounter
		{
			tPicture( ) : mState( cStateNull ), mExternalRefCount( 0 ) { }
			~tPicture( ) { sigassert( mState != cStateBusy ); sigassert( !mTexture.fGetRaw( ) ); }

			u32 mState;
			u32 mExternalRefCount;

			Gfx::tTextureReference mTexture;
			tDynamicArray<wchar_t> mKeyData;

#ifdef platform_xbox360
			XUSER_DATA mPlatformKey;
			XOVERLAPPED mOverlapped;
#endif //platform_xbox360
		};

		typedef tRefCounterPtr<tPicture> tPicturePtr;
		typedef tHashTable<tPictureKey, tPicturePtr, tHashTableExpandAndShrinkResizePolicy, tPictureKeyHash > tPictureMap;

	private:

		void fQueryPic( const tPictureKey & key, tPicture & picture );
		void fProcessPic( tPicture & picture );
		void fDestroyPic( tPicture & picture );
		void fCreateTexture( Gfx::tTextureReference & out, b32 _small );
		void fDestroyTexture( Gfx::tTextureReference & in );

		tPicture * fFindPic( const tPictureKey & key );

		// Access for tUserPicRef
		void fAcquirePic( const tPictureKey & key );
		void fReleasePic( const tPictureKey & key );
		b32 fGetTexture( const tPictureKey & key, Gfx::tTextureReference & texRef );
		b32 fTextureIsReady( const tPictureKey& key );

	private:

		tPictureMap mPictureMap;
	};

	///
	/// \class tUserPicRef
	/// \brief References a picture by key
	class tUserPicRef
	{
	public:

		tUserPicRef( ) { }
		tUserPicRef( const tUserPicRef & other );
		~tUserPicRef( ) { fRelease( ); }

		b32 fNull( ) const { return !mKey.mKey.fIsSet( ); }
		b32 fGetTexture( Gfx::tTextureReference & tex, Math::tVec2f * outDims = NULL ) const;
		b32 fIsReady( ) const;

		void fReset( const tUserData & key, b32 _small );
		void fRelease( );

		tUserPicRef & operator=( const tUserPicRef & other );

	private:

		tUserPicServices::tPictureKey mKey;
	};

	///
	/// \class tUserProfilePicRef
	/// \brief Handy for wrapping a profile ref with a gamer pic ref
	class tUserProfilePicRef
	{
	public:

		tUserProfilePicRef( ) { }
		tUserProfilePicRef( const tUserProfilePicRef & other );

		b32 fNull ( ) const { return mProfile.fNull( ); }
		b32 fError( ) const { return mProfile.fError( ); }
		b32 fIsReady( ) const { return mPic.fIsReady( ); }

		const tUserProfileRef & fProfile( ) const { return mProfile; }
		const tUserPicRef & fPic( ) const { return mPic; }

		void fUpdateST( );

		void fReset( u32 requesterIdx, const tPlatformUserId & userId, b32 _small ); 
		void fRelease( );

		tUserProfilePicRef & operator=( const tUserProfilePicRef & other );

	private:

		b32 mSmall;
		tUserPicRef mPic;
		tUserProfileRef mProfile;
	};

} // ::Sig

#endif//__tUserPicServices__
