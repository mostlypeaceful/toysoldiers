//------------------------------------------------------------------------------
// \file tAnimPackData.hpp - 26 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tAnimPackData__
#define __tAnimPackData__
#include "Anipk.hpp"
#include "GameArchiveString.hpp"

namespace Sig
{
	// animation packs
	struct toolsgui_export tAnimPackData
	{
		tResourcePtr mResource;
		Anipk::tFile mAnipkFile;
		tFilePathPtr mAnipkPath;
		std::string mLongLabel;
		std::string mLabelWithSize;
		b32 mDirty;
		tAnimPackData( ) : mDirty( false ) { }
		explicit tAnimPackData( const tResourcePtr& resource );
		inline b32 operator==( const tAnimPackData& other ) const { return mResource == other.mResource; }
		inline b32 operator==( const tResourcePtr& other ) const { return mResource == other; }
	};
	typedef tGrowableArray< tAnimPackData > tAnimPackList;


	/*
		
		The agent will scan the res directory looking for animpacks.
		It will build a table relating a skeleton to all applicable animpacks.

		It will cache this work in an engine temporary file. So periodic refreshing will be necessary.

	*/
	class toolsgui_export tAnimPackDataAgent
	{
		declare_singleton_define_own_ctor_dtor( tAnimPackDataAgent );
	public:		
		tAnimPackDataAgent( );
		~tAnimPackDataAgent( ) { }

		void fRefresh( );
		void fLoad( );

		void fGetAnimPacksForSkeleton( const tFilePathPtr& skelPath, tGrowableArray<tResourcePtr>& animPacks );

		struct tAnimPackList
		{
			tFilePathPtr mSkeleton;
			tGrowableArray< tFilePathPtr > mAnimPacks;
			tGrowableArray< tFilePathPtr > mSkeletonMaps;

			template<class tArchive>
			void fSaveLoad( tArchive& archive )
			{
				archive.fSaveLoad( mSkeleton );
				archive.fSaveLoad( mAnimPacks );
				archive.fSaveLoad( mSkeletonMaps );
			}

			tAnimPackList( const tFilePathPtr& skeleton = tFilePathPtr::cNullPtr )
				: mSkeleton( skeleton )
			{ }

			b32 operator == ( const tFilePathPtr& skeleton ) const { return mSkeleton == skeleton; }
		};

		static const u32 cVersion = 1;

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mAnimPacksBySkeleton );
		}

	private:
		tGrowableArray< tAnimPackList > mAnimPacksBySkeleton;

		void fSave( );
	};

}

#endif//__tAnimPackData__
