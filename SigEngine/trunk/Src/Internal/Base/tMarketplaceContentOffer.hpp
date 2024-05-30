//------------------------------------------------------------------------------
// \file tMarketplaceContentOffer.hpp - 07 Aug 2013
// \author colins
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tMarketplaceContentOffer__
#define __tMarketplaceContentOffer__
#include "XtlUtil.hpp"

namespace Sig
{
	typedef u64 tMarketplaceOfferId;

	///
	/// \class tMarketplaceContentOffer
	/// \brief Info about a marketplace offering
#ifdef platform_xbox360
	struct tMarketplaceContentOffer : public XMARKETPLACE_CURRENCY_CONTENTOFFER_INFO
	{
		tMarketplaceOfferId fOfferId( ) const { return qwOfferID; }
		std::wstring fOfferName( ) const { return std::wstring( wszOfferName ); }
		std::wstring fOfferPriceDisplay( ) const { return std::wstring( wszCurrencyPrice ); }
	};
#else
	struct tMarketplaceContentOffer
	{
		tMarketplaceOfferId fOfferId( ) const { log_warning_unimplemented( ); return 0; }
		std::wstring fOfferName( ) const { return std::wstring( ); }
		std::wstring fOfferPriceDisplay( ) const { return std::wstring( ); }
	};
#endif

	typedef tGrowableArray<tMarketplaceContentOffer> tMarketplaceContentOfferList;

	///
	/// \class tMarketplaceContentOfferEnumerator
	/// \brief Provides access to the title's marketplace content offers
	class tMarketplaceContentOfferEnumerator
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

		tMarketplaceContentOfferEnumerator( ) : mState( cStateNull ) { }

		inline tState fState( ) const { return mState; }
		inline b32 fEnumerating( ) const { return mState == cStateEnumerating; }
		inline b32 fFinished( ) const { return mState == cStateSuccess || mState == cStateFail; }
		inline b32 fSucceeded( ) const { return mState == cStateSuccess; }
		inline b32 fFailed( ) const { return mState == cStateFail; }

		// Step 1
		b32 fCreate( u32 userIndex, const u64* offerIds, u32 numOfferIds, u32 numItems );
		b32 fCreate( u32 userIndex, u32 offerType, u32 contentCategories, u32 numItems );

		// Step 2
		b32 fEnumerate( );

		// Step 3
		b32 fAdvance( );

		// Step 4
		u32 fResultCount( );
		void fGetResults( tMarketplaceContentOfferList& contentOfferListOut );

		// Step 5
		void fDestroy( );

	private:

		tState mState;

#ifdef platform_xbox360
		XtlUtil::tEnumerateOp mEnumOp;
#endif
	};

}

#endif//__tMarketplaceContentOffer__
