//------------------------------------------------------------------------------
// \file tEditableGroundCoverCloud.hpp - 16 Sep 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tEditableGroundCoverCloud__
#define __tEditableGroundCoverCloud__
#include "Gfx/tGroundCoverCloud.hpp"
#include "tTerrainGeometry.hpp"
#include "Sigml.hpp"

namespace Sig
{
	class tools_export tEditableGroundCoverCloud : public Gfx::tGroundCoverCloud
	{
		define_dynamic_cast(tEditableGroundCoverCloud, Gfx::tGroundCoverCloud);
	public:

		typedef tTerrainGeometry::tEditableGroundCoverMask tEditableGroundCoverMask;

		enum tDirtyFlags
		{
			cDirtyNone = 0,
			cDirtyXforms = ( 1 << 0 ),
			cDirtyCells = ( 1 << 1 ),
			cDirtyElements = ( 1 << 2 ),
		};

		tEditableGroundCoverCloud( tEditableObjectContainer & container, f32 xLen, f32 zLen );
		virtual ~tEditableGroundCoverCloud( );

		const Gfx::tGroundCoverCloudDef & fDef( ) const { return mDefInstance; }

		void fSetDimensions( f32 xLen, f32 zLen );

		// Returns if anything has changed
		void fUpdateDef( 
			const Sigml::tGroundCoverLayer & layer,
			b32 updateCover = true );

		void fUpdateDef( 
			const tTerrainGeometry::tGroundCover & gc,
			b32 updateCover = true );

		void fUpdateMask( u32 minX, u32 minZ, u32 maxX, u32 maxZ, const f32 mask[] );
		void fUpdateHeights( const u32 count, const f32 * heights, b32 skipRefresh = false );
		void fUpdateHeights( u32 minX, u32 minZ, u32 maxX, u32 maxZ, const f32 * heights, b32 skipRefresh = false );

		void fUpdateShadows( const Sigml::tGroundCoverLayer & layer );
		void fUpdateFrequency( const Sigml::tGroundCoverLayer & layer );
		void fUpdateSpawnCount( const Sigml::tGroundCoverLayer & layer );

		u32 fDirty( ) const { return mDirtyFlags; }	

		b32 fNeedsHeights( ) const { return mNeedsHeights; }
		void fSetNeedsHeights( b32 val ) { mNeedsHeights = val; }

		b32 fVisible( ) const { return mVisible; }
		void fSetVisible( b32 visible ) { mVisible = visible; }

		virtual void fPrepareRenderables( const Gfx::tCamera & camera );
		virtual void fGatherRenderables( tGatherCb * cb, b32 forShadows );

		void fUpdateCover( );

	private:

		
		
		void fOnResourceLoaded( tResource & theResource, b32 success );
		void fRefreshXforms( u32 minX, u32 minZ, u32 maxX, u32 maxZ );

	private:

		tEditableObjectContainer & mContainer;

		b32 mNeedsHeights;
		b32 mVisible;
		u32 mDirtyFlags;

		tResource::tOnLoadComplete::tObserver mOnLoadComplete;

		Gfx::tGroundCoverCloudDef mDefInstance;
		tGrowableArray<tLoadInPlaceResourcePtr *> mResources;
		tDynamicArray<f32> mFullMask;
	};
}

#endif//__tEditableGroundCoverCloud__
