//------------------------------------------------------------------------------
// \file tAnimPackData.hpp - 26 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tAnimPackData__
#define __tAnimPackData__
#include "Anipk.hpp"

namespace Sig
{
	// animation packs
	struct tAnimPackData
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
}

#endif//__tAnimPackData__
