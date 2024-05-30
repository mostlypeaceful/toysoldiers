//------------------------------------------------------------------------------
// \file tMarketplaceAsset.hpp - 04 Apr 2012
// \author colins
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tMarketplaceAsset__
#define __tMarketplaceAsset__
#include "XtlUtil.hpp"

namespace Sig
{
	///
	/// \class tMarketplaceAsset
	/// \brief A consumable asset that a user obtains from the marketplace
#ifdef platform_xbox360
	struct tMarketplaceAsset : public XMARKETPLACE_ASSET
	{
		u32 fCount( ) const { return dwQuantity; }
		u32 fId( ) const { return dwAssetID; }
	};
#else
	struct tMarketplaceAsset
	{
		u32 fCount( ) const { return 0; }
		u32 fId( ) const { return 0; }
	};
#endif

	typedef tGrowableArray<tMarketplaceAsset> tMarketplaceAssetList;

	///
	/// \class tMarketplaceAssetEnumerator
	/// \brief Provides access to a user's marketplace assets
	class tMarketplaceAssetEnumerator
	{
	public:

		enum tState
		{
			cStateNull = 0,
			cStateCreated,
			cStateEnumerating,
			cStateSuccess,
			cStateFail
		};

	public:

		tMarketplaceAssetEnumerator( ) : mState( cStateNull ) { }

		inline tState fState( ) const { return mState; }
		inline b32 fEnumerating( ) const { return mState == cStateEnumerating; }
		inline b32 fFinished( ) const { return mState == cStateSuccess || mState == cStateFail; }
		inline b32 fSucceeded( ) const { return mState == cStateSuccess; }
		inline b32 fFailed( ) const { return mState == cStateFail; }

		// Step 1
		b32 fCreate( 
			u32 userIndex,	// [ 0 , tUser::cMaxLocalUsers )
			u32 numItems );	// Maximum number of items to return

		// Step 2
		b32 fEnumerate( );

		// Step 3
		b32 fAdvance( );

		// Step 4
		u32 fResultCount( );
		void fGetResults( tMarketplaceAssetList& assetListOut );

		// Step 5
		void fDestroy( );

	private:

		tState mState;

#ifdef platform_xbox360
		XtlUtil::tEnumerateOp mEnumOp;
#endif
	};

}

#endif//__tMarketplaceAsset__
