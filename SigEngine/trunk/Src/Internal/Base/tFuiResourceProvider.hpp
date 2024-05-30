//------------------------------------------------------------------------------
// \file tFuiResourceProvider.hpp - 16 Sep 2011
// \author pwalker
//
// Copyright © Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tFuiResourceProvider__
#define __tFuiResourceProvider__

#include "Gfx/tTextureFile.hpp"
#include "tResourceLoadList2.hpp"

namespace Sig
{
	/// \brief provide an interface for tFuiSystem to look up resources
	class tFuiResourceProvider
	{

		struct tFuiRes
		{
			tResourcePtr	mResource;
			b32				mPermaLoaded:1;
			b32				mRefCount:31;
		};
		tGrowableArray< tFuiRes > mLoadedResources;

		struct tFuiResourceLoadList
		{
			u32										mID;			///< For FUI to uniquely identify a tRLL2
			tRefCounterPtr< tResourceLoadList2 >	mResources;
			tStringPtr								mDebugOrigin;

			tFuiResourceLoadList( );
			tFuiResourceLoadList( u32 id, const tStringPtr& debugName );
		};
		tGrowableArray< tFuiResourceLoadList > mAsyncLoadingResourceLists;
		u32 mNextAsyncLoadingResourceListID;

	public:
		tFuiResourceProvider( );
		~tFuiResourceProvider( );

		Gfx::tTextureFile::tPlatformHandle fTextureLookup( const char* textureName, u32 *outWidth, u32 *outHeight );
		void fReleaseTexture( const Gfx::tTextureFile::tPlatformHandle& file );
		tResourcePtr fAddTextureResource( const tResourceId& rid, const b32 permaLoaded );

		void		fAddToResourceLoadList( u32 rllid, const tResourceId& rid );
		tFuiVoid	fAddTextureFileToResourceLoadList( u32 rllid, const tFilePathPtr& newPath );
		u32			fCreateResourceLoadList( const tStringPtr& debugName );
		tFuiVoid	fDestroyResourceLoadList( u32 rllid );
		b32			fResourceLoadListDone( u32 rllid );

	private:
		static void fExportFuiInterface( ); // self registered during init due to dependency ordering issues
		tFuiRes* fFindResource( const tResourceId& rid );

	};
} // Sig

#endif
