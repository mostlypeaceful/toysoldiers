//------------------------------------------------------------------------------
// \file tDLCEnumerator.hpp - 23 Mar 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tDLCEnumerator__
#define __tDLCEnumerator__

#include "tContentEnumerator.hpp"
#include "tUser.hpp"

namespace Sig
{
	///
	/// \class tDLCPackage
	/// \brief 
	struct tDLCPackage : public tRefCounter
	{
		static const u32 cInvalidId = ~0;

		enum tPackageType
		{
			cDLCPackage,
			cTitleUpdatePackage,
			cDLCCompatibilityPackage,
			cInvalidPackage
		};

		tDLCPackage( ) 
			: mId( cInvalidId )
			, mType( cInvalidPackage )
			, mState( tContent::cStateNull )
		{ }

		u32				mId;
		u32				mType;
		tStringPtr		mDriveName;
		tFilePathPtr	mResPath;
		tContentData	mContentData;

		b32				mState;

		const tFilePathPtr& fResPath( ) const { return mResPath; }
		tFilePathPtr fDriveFolder( ) const { return tFilePathPtr( (std::string( mDriveName.fCStr( ) ) + ":\\").c_str( ) ); }
	};

	typedef tRefCounterPtr<tDLCPackage> tDLCPackagePtr;

	///
	/// \class tDLCEnumerator
	/// \brief Enumerates the list of dlc packages available
	class tDLCEnumerator
	{
	public:

		enum tState
		{
			cStateNull,
			cStateEnumerating,
			cStateSuccess,
			cStateFail
		};

		static const u32 cMaxPackages = 5; // This must be larger than the total associated with the title

	public:

		tDLCEnumerator( );
		~tDLCEnumerator( );

		inline tState fState( ) const { return mState; }

		//Step 1
		b32 fEnumerate( );

		// Step 2
		b32 fAdvance( );
		void fWait( );
		void fWaitForFinish( );

		// Step 3
		u32 fPackageCount( ) const { return mFinalPackages.fCount( ); }
		const tDLCPackagePtr& fPackage( u32 idx ) const { return mFinalPackages[ idx ]; }

		// Step 4 before restarting at step 1
		void fClose( );

		u32 fCountByType( u32 type ) const;

		b32 fFoundCorrupt( ) const { return mFoundCorrupt; }
		b32 fNeedToLoginForSomeContent( ) const { return mNeedToLoginForSomeContent; }

	private:

		enum tEnumState
		{
			cEnumStateEnumerating,
			cEnumStateTestEnumerating,
			cEnumStateCreatingContent
		};

	private:

		void fCreateContent( u32 index );
		void fAssignPackageIds( );

	private:
		
		tState mState;
		tEnumState mEnumState;

		struct tUserContent
		{
			tContentEnumerator mContentEnum;
			tGrowableArray< tContent > mContent;
			tGrowableArray< tDLCPackagePtr > mPackages;
		};


		tFixedArray<tUserContent, tUser::cMaxLocalUsers + 1> mUserContent;
		tGrowableArray< tDLCPackagePtr > mFinalPackages;
		b32 mFoundCorrupt;
		b32 mNeedToLoginForSomeContent;
	};
}

#endif//__tDLCEnumerator__
