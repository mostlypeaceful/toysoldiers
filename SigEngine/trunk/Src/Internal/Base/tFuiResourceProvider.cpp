#include "BasePch.hpp"
//------------------------------------------------------------------------------
// \file tFuiResourceProvider.cpp - 16 Sep 2011
// \author pwalker
//
// Copyright © Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "tFuiResourceProvider.hpp"
#include "tApplication.hpp"
#include "tGameAppBase.hpp"
#include "tUserPicServices.hpp"
#include "tFuiUserProfilePicService.hpp"
#include "tFuiDataStore.hpp"

namespace Sig
{
	devvar( bool, Fui_ShowLoadUnload, false );

	namespace
	{
		const tStringPtr cGlobalInstanceName( "tFuiResourceProvider" );
	}

	tFuiResourceProvider::tFuiResourceLoadList::tFuiResourceLoadList( )
	{
	}

	tFuiResourceProvider::tFuiResourceLoadList::tFuiResourceLoadList( u32 id, const tStringPtr& debugName )
		: mID( id )
		, mResources( NEW_TYPED( tResourceLoadList2 )( ) )
		, mDebugOrigin( debugName )
	{
	}

	tFuiResourceProvider::tFuiResourceProvider( )
		: mNextAsyncLoadingResourceListID( 0 )
	{
		static bool exported = false;
		if( !exported )
		{
			fExportFuiInterface( );
			exported = true;
		}

		tFuiDataStore::fInstance( ).fRegisterGlobalInstance( cGlobalInstanceName, this );
	}

	tFuiResourceProvider::~tFuiResourceProvider( )
	{
		tFuiDataStore::fInstance( ).fClearGlobalInstance( cGlobalInstanceName );
	}

	Gfx::tTextureFile::tPlatformHandle tFuiResourceProvider::fTextureLookup( const char* textureName, u32 *outWidth, u32 *outHeight )
	{
		Gfx::tTextureFile::tPlatformHandle h = tGameAppBase::fInstance( ).fHandleFuiTextureLookup( textureName, outWidth, outHeight );
		if( h )
		{
			if( Fui_ShowLoadUnload )
				log_line( Log::cFlagFui, "[" << ( textureName ? textureName : "" ) << "] handled by GameApp." );
			return h;
		}

		const b32 permaLoad = false;
		if( !textureName || !textureName[0] )
		{
			log_warning( "fTextureLookup trying to load a texture with " << ( textureName ? "{0}" : "NULL" ) << " filename. Returning 1x1 black texture." );

			*outWidth = 1;
			*outHeight = 1;
			return tGameAppBase::fInstance( ).fScreen( )->fBlackTexture( );
		}

		tGrowableArray< std::string > outStrings;
		StringUtil::fSplit( outStrings, textureName, "_" );
		if( outStrings.fCount( ) == 2 )
		{
			if( outStrings[ 0 ] == "gamerpic" )
			{
				if( const tFuiUserProfilePic* profilePic = tFuiUserProfilePicService::fInstance( ).fFindUser( tStringPtr( textureName ) ) )
				{
					Gfx::tTextureReference texRef;
					Math::tVec2f outDims;
					if( profilePic->fGetTexture( texRef, &outDims ) )
					{
						if( Fui_ShowLoadUnload )
							log_line( Log::cFlagFui, "[" << ( textureName ? textureName : "" ) << "] handled by Profile Pic Service." );

						*outWidth = (u32)outDims.x;
						*outHeight = (u32)outDims.y;

						if( texRef.fGetRaw( ) )
							return texRef.fGetRaw( );
						else
							return texRef.fGetTextureFile( )->fGetPlatformHandle( );
					}

					log_warning( "fTextureLookup found user [" << textureName << "] but was unable to find the gamer pic, returning 1x1 black texture for now." );
				}
				else
					log_warning( "fTextureLookup could not find user [" << textureName << "]. returning 1x1 black texture." );

				*outWidth = 1;
				*outHeight = 1;
				return tGameAppBase::fInstance( ).fScreen( )->fBlackTexture( );
			}
		}

		//try loading resource
		const tResourceId rid = tResourceId::fMake< Gfx::tTextureFile >( textureName );
		tResourcePtr res = fAddTextureResource( rid, permaLoad );
		Gfx::tTextureFile* texFile = res ? res->fCast< Gfx::tTextureFile >( ) : NULL;
		if( !texFile )
		{
			log_warning( "tFuiResourceProvider::fTextureLookup can't find file [" << textureName << "] returning 1x1 black texture." );
			*outWidth = 1;
			*outHeight = 1;
			return tGameAppBase::fInstance( ).fScreen( )->fBlackTexture( );
		}

		*outWidth = texFile->mWidth;
		*outHeight = texFile->mHeight;
		return texFile->mPlatformHandle;
	}

	void tFuiResourceProvider::fReleaseTexture( const Gfx::tTextureFile::tPlatformHandle& handle )
	{
		for( u32 i = 0; i < mLoadedResources.fCount( ); ++i )
		{
			Gfx::tTextureFile* resFile = mLoadedResources[ i ].mResource->fCast<Gfx::tTextureFile>( );
			sigassert( resFile );
			Gfx::tTextureFile::tPlatformHandle resHandle = resFile->fGetPlatformHandle( );
			if( resHandle == handle )
			{
				if( mLoadedResources[ i ].mPermaLoaded ) //found but we don't care 'cause it is perma-loaded
				{
					if( Fui_ShowLoadUnload )
						log_line( Log::cFlagFui, "[" << mLoadedResources[ i ].mResource->fGetPath( ) << "] is perma-loaded. Ignoring texture release." );
					break;
				}

				--mLoadedResources[ i ].mRefCount;
				if( mLoadedResources[ i ].mRefCount == 0 )
				{
					if( Fui_ShowLoadUnload )
					{
						Gfx::tTextureFile* texFile = mLoadedResources[ i ].mResource->fCast<Gfx::tTextureFile>( );
						log_line( Log::cFlagFui, "Released [" << mLoadedResources[ i ].mResource->fGetPath( ) << "] " 
							<< std::fixed << std::setprecision( 2 ) << Memory::fToMB<f32>( texFile->fVramUsage( ) ) 
							<< "MB. (" << texFile->mWidth << ", " << texFile->mHeight << ")" );
					}

					mLoadedResources.fErase( i );
				}
				else
				{
					if( Fui_ShowLoadUnload )
						log_line( Log::cFlagFui, "[" << mLoadedResources[ i ].mResource->fGetPath( ) << "] Not released, yet. RefCount[" << mLoadedResources[ i ].mRefCount << "]" );
				}
				break;
			}
		}
	}

	tResourcePtr tFuiResourceProvider::fAddTextureResource( const tResourceId& rid, const b32 permaLoaded )
	{
		tFuiRes* fuiRes = fFindResource( rid );
		if( !fuiRes )
		{
			tResourcePtr resPtr = tApplication::fInstance( ).fResourceDepot( )->fQueryLoadBlock( rid, this );
			if( !resPtr || resPtr->fCast<Gfx::tTextureFile>( ) == NULL )
			{
				if( Fui_ShowLoadUnload )
					log_line( Log::cFlagFui, "FAILED to load [" << resPtr->fGetPath( ) << "]" );

				//res does not exist, don't add to loaded resources
				return resPtr;
			}

			if( Fui_ShowLoadUnload )
			{
				Gfx::tTextureFile* texFile = resPtr->fCast<Gfx::tTextureFile>( );
				log_line( Log::cFlagFui, "Loaded [" << resPtr->fGetPath( ) << "] " 
					<< std::fixed << std::setprecision( 2 ) << Memory::fToMB<f32>( texFile->fVramUsage( ) ) 
					<< "MB. (" << texFile->mWidth << ", " << texFile->mHeight << ")" );
			}

			tFuiRes& newFuiRes = mLoadedResources.fPushBack( );
			newFuiRes.mResource = resPtr;
			newFuiRes.mRefCount = 0;
			newFuiRes.mPermaLoaded = false;
			fuiRes = &newFuiRes;
		}
		if( permaLoaded ) //we should never un-permaload
		{
			if( Fui_ShowLoadUnload )
				log_line( Log::cFlagFui, "Flagging [" << fuiRes->mResource->fGetPath( ) << "] as perma-loaded." );

			fuiRes->mPermaLoaded = true;
		}
		
		if( !fuiRes->mPermaLoaded )
		{
			++fuiRes->mRefCount;
			if( fuiRes->mRefCount > 1 )
			{
				if( Fui_ShowLoadUnload )
					log_line( Log::cFlagFui, "[" << fuiRes->mResource->fGetPath( ) << "] RefCount increased to [" << fuiRes->mRefCount << "]" );

				//Not a bug in Iggy code, Iggy doesn't even plan on fixing this currently. The problem? A string is just a string to iggy. "a" != "A"
				// "a/a" != "a\\a"   but to us, both of those would result in the same texture. So the problem that occurs through all of this is
				// that we generate an extra GDraw texture handle for each different string representation of the same asset. This is a problem
				// because we have a limited # of texture handles and we allocate those at startup. We should verify that it is safe to not allocate
				// an extra GDraw texture handle for this case and do that instead of trying to fixup all the mismatched strings.
				//log_warning( "Iggy docs state that multiple calls to IggyFunctions.setTextureForBitmap[" << fuiRes->mResource->fGetPath( ) << "] should only incur a single call to TextureCreate. This is not the case. Most likely a bug is within Iggy code." );
			}

			sigassert( fuiRes->mRefCount < 1024 && "WTF IS GOING ON? WHY DO WE HAVE 1024 iggy references to the same texture??" );
		}		

		return fuiRes->mResource;
	}

	tFuiResourceProvider::tFuiRes* tFuiResourceProvider::fFindResource( const tResourceId& rid )
	{
		for( u32 i = 0; i < mLoadedResources.fCount( ); ++i )
		{
			if( mLoadedResources[ i ].mResource->fGetResourceId( ) == rid )
				return &mLoadedResources[ i ];
		}
		return NULL;
	}

	u32 tFuiResourceProvider::fCreateResourceLoadList( const tStringPtr& debugName )
	{
		const tFuiResourceLoadList frll( mNextAsyncLoadingResourceListID++, debugName );
		mAsyncLoadingResourceLists.fPushBack( frll );
		return frll.mID;
	}

	void tFuiResourceProvider::fAddToResourceLoadList( u32 rllid, const tResourceId& rid )
	{
		if( StringUtil::fStartsWith( rid.fGetPath( ).fCStr( ), "gamerpic_" ) )
		{
			// Gamerpics are currently already (pre)loaded elsewhere, and aren't true paths.
			// If we used "gamerpic_{XUID}" instead of "gamerpic_{SOMEINDEX}, we could move that here for more consistency?
			// See: tFuiUserProfilePicService.
			return;
		}

		for( u32 i = 0; i < mAsyncLoadingResourceLists.fCount( ); ++i )
		{
			const tFuiResourceLoadList& frll = mAsyncLoadingResourceLists[ i ];
			if( frll.mID == rllid )
			{
				frll.mResources->fAdd( rid );
				return;
			}
		}

		log_assert( 0, "fAddToResourceLoadList( " << rllid << " ) failed: no such RLL" );
	}

	tFuiVoid tFuiResourceProvider::fAddTextureFileToResourceLoadList( u32 rllid, const tFilePathPtr& newPath )
	{
		fAddToResourceLoadList( rllid, tResourceId::fMake< Gfx::tTextureFile >( newPath ) );
		return 0; // return unused
	}

	tFuiVoid tFuiResourceProvider::fDestroyResourceLoadList( u32 rllid )
	{
		for( u32 i = 0; i < mAsyncLoadingResourceLists.fCount( ); ++i )
		{
			const tFuiResourceLoadList& frll = mAsyncLoadingResourceLists[ i ];
			if( frll.mID == rllid )
			{
				mAsyncLoadingResourceLists.fErase( i );
				return 0; // return unused
			}
		}

		log_assert( 0, "fDestroyResourceLoadList( " << rllid << " ) failed: no such RLL" );
		return 0; // return unused
	}

	b32 tFuiResourceProvider::fResourceLoadListDone( u32 rllid )
	{
		for( u32 i = 0; i < mAsyncLoadingResourceLists.fCount( ); ++i )
		{
			const tFuiResourceLoadList& frll = mAsyncLoadingResourceLists[ i ];
			if( frll.mID == rllid )
			{
				return frll.mResources->fDone( );
			}
		}

		log_assert( 0, "fDoneResourceLoadListDone( " << rllid << " ) failed: no such RLL" );
		return false;
	}

	void tFuiResourceProvider::fExportFuiInterface( )
	{
		FUIRat::tDataProvider< tFuiResourceProvider > bind;
		bind
			.Func( "fCreateResourceLoadList",			&tFuiResourceProvider::fCreateResourceLoadList )
			.Func( "fAddTextureFileToResourceLoadList",	&tFuiResourceProvider::fAddTextureFileToResourceLoadList )
			.Func( "fDestroyResourceLoadList",			&tFuiResourceProvider::fDestroyResourceLoadList )
			.Func( "fResourceLoadListDone",				&tFuiResourceProvider::fResourceLoadListDone )
			;

		bind.fRegister( );
	}
}//Sig

