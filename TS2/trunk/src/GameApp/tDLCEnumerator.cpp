//------------------------------------------------------------------------------
// \file tDLCEnumerator.cpp - 23 Mar 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "GameAppPch.hpp"
#include "tDLCEnumerator.hpp"
#include "tUser.hpp"
#include "FileSystem.hpp"
#include "tGameApp.hpp"

namespace Sig
{

	//------------------------------------------------------------------------------
	tDLCEnumerator::tDLCEnumerator( )
		: mState( cStateNull )
		, mFoundCorrupt( false )
		, mNeedToLoginForSomeContent( false )
	{
	}

	//------------------------------------------------------------------------------
	tDLCEnumerator::~tDLCEnumerator( )
	{
		fClose( );
	}

	//------------------------------------------------------------------------------
	b32 tDLCEnumerator::fEnumerate( )
	{
		sigassert( mState == cStateNull && "Can only enumerate DLC form Null state" );

		b32 created = mUserContent[ 0 ].mContentEnum.fCreate( 
			tUser::cUserIndexNone,					// just test for what packages are out there with index none.
			tContentData::cContentDeviceIdAny,		// Don't care what device the dlc is on
			tContentData::cContentTypeMarketPlace,	// DLC is only available on the marketplace
			false,									// Doesn't have to be user specific, see above
			cMaxPackages							// Max number of packages to find
		);

		if( !created || !mUserContent[ 0 ].mContentEnum.fEnumerate( ) )
			return false;

		mState = cStateEnumerating;
		mEnumState = cEnumStateTestEnumerating;
		u32 numCreated = 0;

		fWaitForFinish( );

		//see if we found any content packages:
		if( mUserContent[ 0 ].mContentEnum.fResultCount( ) )
		{
			// we did
			// we need to wait long enough so we can open them
			fSleep( 2500 );

			mUserContent[ 0 ].mContentEnum.fDestroy( );

			for( u32 i = 0; i < mUserContent.fCount( ); ++i )
			{
				u32 userIndex = (i == tUser::cMaxLocalUsers) ? tUser::cUserIndexNone : i;

				created = mUserContent[ i ].mContentEnum.fCreate( 
					userIndex,								// specific user
					tContentData::cContentDeviceIdAny,		// Don't care what device the dlc is on
					tContentData::cContentTypeMarketPlace,	// DLC is only available on the marketplace
					false,									// Doesn't have to be user specific, see above
					cMaxPackages							// Max number of packages to find
					);

				if( created )
				{
					++numCreated;
					mUserContent[ i ].mContentEnum.fEnumerate( );
				}
			}
		}

		if( numCreated )
		{
			mState = cStateEnumerating;
			mEnumState = cEnumStateEnumerating;
		}
		else
		{
			mState = cStateSuccess;
			fAssignPackageIds( );
		}

		//mFoundCorrupt = false; //dont set this false again, once there has been corrupt found, always corrupt found.
		return true;
	}
	
	//------------------------------------------------------------------------------
	b32 tDLCEnumerator::fAdvance( )
	{
		if( mState == cStateEnumerating )
		{
			// Enumerating marketplace content
			if( mEnumState == cEnumStateEnumerating || mEnumState == cEnumStateTestEnumerating )
			{
				for( u32 i = 0; i < mUserContent.fCount( ); ++i )
				{
					if( mUserContent[ i ].mContentEnum.fState( ) != tContentEnumerator::cStateNull && !mUserContent[ i ].mContentEnum.fAdvance( ) )
						return false;
				}

				for( u32 i = 0; i < mUserContent.fCount( ); ++i )
				{
					if( mUserContent[ i ].mContentEnum.fState( ) != tContentEnumerator::cStateNull )
					{
						if( mUserContent[ i ].mContentEnum.fState( ) == tContentEnumerator::cStateSuccess )
						{
							if( mEnumState == cEnumStateTestEnumerating )
							{
								mState = cStateNull;
								return true;
							}
							else
							{
								fCreateContent( i );
								mEnumState = cEnumStateCreatingContent;
							}
						}
						else
						{
							mState = cStateFail;
							return true;
						}
					}
				}
			}

			// Mapping the content to drives
			else if( mEnumState == cEnumStateCreatingContent )
			{
				b32 finished = true;

				for( u32 i = 0; i < mUserContent.fCount( ); ++i )
				{
					u32 finishedCnt = 0;
					const u32 contentCount = mUserContent[ i ].mContent.fCount( );
					for( u32 c = 0; c < contentCount; ++c )
					{
						tContent & content = mUserContent[ i ].mContent[ c ];
						if( !content.fAdvance( ) )
							continue;

						switch( content.fState( ) )
						{

						// Keep waiting
						case tContent::cStateCreating:
							continue; 

						// Success!
						case tContent::cStateCreated:
							mUserContent[ i ].mPackages[ c ]->mState = tContent::cStateCreated;
							break;
						case tContent::cStateAccessDenied:
							mUserContent[ i ].mPackages[ c ]->mState = tContent::cStateAccessDenied;
							mNeedToLoginForSomeContent = true;
							break;
						case tContent::cStateFailed: // Failed to create
							// Falls through.
						// Corrupted
						case tContent::cStateCorrupt:
							mUserContent[ i ].mPackages[ c ]->mState = tContent::cStateCorrupt;
							break;
						case tContent::cStateNull:	 // Failed to start create
							break;

						//// Failed to create
						//
						//	break;

						//// Failed to start creation
						//
						//	break;
						}

						++finishedCnt;
					}

					// All done
					if( finishedCnt == contentCount )
					{
						mUserContent[ i ].mContent.fSetCount( 0 );
						mUserContent[ i ].mContentEnum.fDestroy( );
					}
					else
						finished = false;
				}

				if( finished )
				{
					mState = cStateSuccess;
					fAssignPackageIds( );
				}
			}
		}

		return mState != cStateEnumerating;
	}

	//------------------------------------------------------------------------------
	void tDLCEnumerator::fWait( )
	{
		if( mState == cStateEnumerating )
		{
			for( u32 i = 0; i < mUserContent.fCount( ); ++i )
				mUserContent[ i ].mContentEnum.fWait( );
			fAdvance( );
		}
	}

	//------------------------------------------------------------------------------
	void tDLCEnumerator::fWaitForFinish( )
	{
		while( mState == cStateEnumerating )
		{
			fSleep( 10 );
			fAdvance( );
		}
	}

	//------------------------------------------------------------------------------
	void tDLCEnumerator::fClose( )
	{
		for( u32 i = 0; i < mUserContent.fCount( ); ++i )
			mUserContent[ i ].mContentEnum.fDestroy( );
		mState = cStateNull;
	}

	//------------------------------------------------------------------------------
	u32 tDLCEnumerator::fCountByType( u32 type ) const
	{
		u32 count = 0;
		for( u32 i = 0; i < mFinalPackages.fCount( ); ++i )
			if( mFinalPackages[ i ]->mType == type )
				++count;

		return count;
	}

	//------------------------------------------------------------------------------
	void tDLCEnumerator::fCreateContent( u32 index )
	{
		const u32 resultCount = mUserContent[ index ].mContentEnum.fResultCount( );
		mUserContent[ index ].mContent.fSetCount( resultCount );

#if defined( platform_xbox360 )
		for( u32 r = 0; r < resultCount; ++r )
		{
			tDLCPackagePtr package( NEW tDLCPackage( ) );

			// Get the content data
			XCONTENT_CROSS_TITLE_DATA data;
			mUserContent[ index ].mContentEnum.fResult( r, package->mContentData, data );

			// Assign a drive name
			std::stringstream ss;
			ss << "content" << r;
			package->mDriveName = tStringPtr( ss.str( ) );

			// Start creating the content
			mUserContent[ index ].mContent[ r ].fCreate( 
				index,
				package->mDriveName, 
				data, 
				tContent::cContentFlagOpenExisting );

			mUserContent[ index ].mPackages.fPushBack( package );
		}
#endif

		log_line( 0, "Packages: Create: " << resultCount );
	}

	//------------------------------------------------------------------------------
	void tDLCEnumerator::fAssignPackageIds( )
	{
		char dlcPathBuf[256]= {0};

		tGrowableArray<std::wstring> uniquePackages;
		tGrowableArray<std::wstring> corruptPackages;
		tGrowableArray<std::wstring> deniedPackages;
		mFinalPackages.fSetCount( 0 );

		for( u32 u = 0; u < mUserContent.fCount( ); ++u )
		{
			for( s32 p = mUserContent[ u ].mPackages.fCount( ) - 1; p >= 0; --p )
			{
				// search for dlc res's
				tDLCPackagePtr ptr = mUserContent[ u ].mPackages[ p ];

				const std::wstring packageDisplayName( ptr->mContentData.mDisplayName.c_str( ) );
				if( uniquePackages.fFind( packageDisplayName ) )
					continue; //already found a good one for this package.

				for( u32 d = 0; d < cMaxPackages; ++d )
				{
					// Original game was looking for dlc%i. we dont want anyone to have access to that with out the title update
					// _snprintf( dlcPathBuf, sizeof( dlcPathBuf ) - 1, "%s:\\dlc%i.sacb", ptr->mDriveName.fCStr( ), d );


					// look for dlc
					{
						// new name convention tu_dlc%i
						_snprintf( dlcPathBuf, sizeof( dlcPathBuf ) - 1, "%s:\\tu_dlc%i.sacb", ptr->mDriveName.fCStr( ), d );
						const tFilePathPtr dlcPath = tFilePathPtr( dlcPathBuf );
						
						if( FileSystem::fFileExists( dlcPath ) )
						{
							ptr->mType = tDLCPackage::cDLCPackage;
							ptr->mId = d;
							ptr->mResPath = dlcPath;

							log_line( 0, "Packages: DLC: " << dlcPath );
							break;
						}
					}

					// look for compatibility
					{
						// new name convention tu_dlc%i
						_snprintf( dlcPathBuf, sizeof( dlcPathBuf ) - 1, "%s:\\tu_dlc_compat%i.sacb", ptr->mDriveName.fCStr( ), d );
						const tFilePathPtr dlcPath = tFilePathPtr( dlcPathBuf );

						if( FileSystem::fFileExists( dlcPath ) )
						{
							ptr->mType = tDLCPackage::cDLCCompatibilityPackage;
							ptr->mId = d;
							ptr->mResPath = dlcPath;

							log_line( 0, "Packages: DLC Compat: " << dlcPath );
							break;
						}
					}
				}

				if( ptr->mState != tContent::cStateCreated || ptr->mId == tDLCPackage::cInvalidId )
				{
					if( ptr->mState == tContent::cStateAccessDenied )
						deniedPackages.fFindOrAdd( packageDisplayName );
					else
						corruptPackages.fFindOrAdd( packageDisplayName );
				}
				else
				{
					//this is a good package
					mFinalPackages.fPushBack( ptr );
					uniquePackages.fPushBack( packageDisplayName );
					corruptPackages.fFindAndErase(  packageDisplayName );
					deniedPackages.fFindAndErase(  packageDisplayName );
				}
			}
		}

		u32 tuPackageIndex = 0;

		//look for title update packages:
		for( u32 d = 0; d < cMaxPackages; ++d )
		{
			_snprintf( dlcPathBuf, sizeof( dlcPathBuf ) - 1, "update:\\tu%i.sacb", d );
			const tFilePathPtr dlcPath = tFilePathPtr( dlcPathBuf );

			if( FileSystem::fFileExists( dlcPath ) )
			{
				tDLCPackage* pkg = new tDLCPackage( );

				pkg->mType = tDLCPackage::cTitleUpdatePackage;
				pkg->mId = d;
				pkg->mResPath = dlcPath;

				// stack these in front of dlc packages.
				mFinalPackages.fInsert( tuPackageIndex++, tDLCPackagePtr( pkg ) );

				log_line( 0, "Packages: TU: " << dlcPath );
				break;
			}
		}

		tFilePathPtrList list;
		FileSystem::fGetFileNamesInFolder( list, tFilePathPtr( "UPDATE:\\" ), true, true );

		for( u32 i = 0; i < list.fCount( ); ++i )
			log_line( 0, "File found: " << list[ i ] );

		for( u32 i = 0; i < corruptPackages.fCount( ); ++i )
			log_line( 0, "Corrupt Package: " << StringUtil::fWStringToString( corruptPackages[ i ].c_str( ) ) );

		for( u32 i = 0; i < deniedPackages.fCount( ); ++i )
			log_line( 0, "Denied Package: " << StringUtil::fWStringToString( deniedPackages[ i ].c_str( ) ) );
		
		mFoundCorrupt = (corruptPackages.fCount( ) > 0);
		mUserContent.fFill( tUserContent( ) );
	}
}
