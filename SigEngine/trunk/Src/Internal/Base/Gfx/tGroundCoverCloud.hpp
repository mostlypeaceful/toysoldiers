//------------------------------------------------------------------------------
// \file tGroundCoverCloud.hpp - 13 Sep 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tGroundCoverCloud__
#define __tGroundCoverCloud__
#include "tEntityCloud.hpp"
#include "tRenderableEntity.hpp"
#include "tSceneRefEntity.hpp"
#include "tGeometryBufferVRam.hpp"

namespace Sig
{
	class tSubMesh;
	class tSceneRefEntity;
	class tMersenneGenerator;
	namespace Gui { class tColoredQuad; }
}

namespace Sig { namespace Gfx 
{
	class tGroundCoverCloud;
	struct tInstanceMaster;
	typedef tDynamicArray< Math::tMat3f > tMat3fArray;

	///
	/// \class tGroundCoverCloudDef
	/// \brief 
	class base_export tGroundCoverCloudDef
	{
		declare_reflector( );
	public:

		class tElement
		{
			declare_reflector( );
		public:

			const tResourcePtr & fSgResourcePtr( ) const { return mSgFile->fGetResourcePtr( ); }
			b32 fCastsShadow( ) const { return mCastsShadow; }
			f32 fFrequency( ) const { return mFrequency; }
			u32 fSpawnCount( ) const { return mSpawnCount; }

		public:
			tLoadInPlaceResourcePtr * mSgFile;
			b32 mCastsShadow;
			f32 mFrequency;
			u32 mSpawnCount;
		};

		enum tRotation 
		{
			cRotationNone = 0,
			cRotationRandomY,
			cRotationRandomXYZ
		};

		enum tTranslation
		{
			cTranslationNone = 0,
			cTranslationXZ,
			cTranslationXYZ
		};
		
		enum tVisibility
		{
			cVisibilityNear = 0,
			cVisibilityMedium,
			cVisibilityFar,
			cVisibilityNoFade,

			cVisibilityCount
		};

	public:

		tGroundCoverCloudDef( tNoOpTag ) 
			: mElements( cNoOpTag )
			, mMask( cNoOpTag )
			, mHeights( cNoOpTag )
			, mCells( cNoOpTag )
		{ }

		tGroundCoverCloudDef( ) 
			: mUnitSize( 0.f )
			, mPaintUnits( 0 )
			, mDimX( 0 )
			, mDimZ( 0 )
			, mMaskDimX( 0 )
			, mMaskDimZ( 0 )
			, mMaxUnitSpawns( 0 )
			, mRotation( cRotationNone )
			, mTranslation( cTranslationNone )
			, mFarVisibility( cVisibilityNear )
			, mNearVisibility( cVisibilityNoFade )
			, mYRotationScale( 0.f )
			, mXZRotationScale( 0.f )
			, mXZTranslationScale( 0.f )
			, mYTranslationScale( 0.f )
			, mScaleRangeAdjustor( 0.f )
			, mWorldLengthX( 0.f )
			, mWorldLengthZ( 0.f )
			, mInstancedUnitSize( 0.f )
			, mInstancedCellDiff( ~0 )
		{ }

		b32 fAnyElementsCastShadow( ) const;

		u32 fElementCount( ) const { return mElements.fCount( ); }
		const tElement * fElements( ) const { return mElements.fBegin( ); }

		f32 fUnitSize( ) const { return mUnitSize; }
		u32 fPaintUnits( ) const { return mPaintUnits; }

		u32 fDimX( ) const { return mDimX; }
		u32 fDimZ( ) const { return mDimZ; }

		u32 fMaskDimX( ) const { return mMaskDimX; }
		u32 fMaskDimZ( ) const { return mMaskDimZ; }

		tRotation fRotation( ) const { return mRotation; }
		tTranslation fTranslation( ) const { return mTranslation; }

		f32 fMaxYRotation( ) const { return Math::c2Pi * mYRotationScale; }
		f32 fMaxXZRotation( ) const { return Math::c2Pi * mXZRotationScale; }

		f32 fMaxXZTranslation( ) const { return 0.5f * mUnitSize * mXZTranslationScale; }
		f32 fMaxYTranslation( ) const { return 0.5f * mUnitSize * mYTranslationScale; }

		f32 fScaleRange( ) const { return 0.6f * mScaleRangeAdjustor; }
		b32 fHasScaleRange( ) const { return mScaleRangeAdjustor != 0; }

		f32 fWorldLengthX( ) const { return mWorldLengthX; }
		f32 fWorldLengthZ( ) const { return mWorldLengthZ; }

		b32 fMask( u32 x, u32 z ) const { return mMask[( z / mPaintUnits ) * mMaskDimX + ( x / mPaintUnits )]; }
		f32 fHeight( u32 x, u32 z, u32 s ) const { return mHeights[ ( z * mDimX + x ) * mMaxUnitSpawns + s ]; }

		tVisibility fFarVisibility( ) const;
		tVisibility fNearVisibility( ) const;
		
		f32 fMinX( ) const { return -0.5f * fWorldLengthX( ) + ( 0.5f * fUnitSize( ) ); }
		f32 fMinZ( ) const { return -0.5f * fWorldLengthZ( ) + ( 0.5f * fUnitSize( ) ); }
		f32 fMaxX( ) const { return 0.5f * fWorldLengthX( ) + ( 0.5f * fUnitSize( ) ); }
		f32 fMaxZ( ) const { return 0.5f * fWorldLengthZ( ) + ( 0.5f * fUnitSize( ) ); }

		f32 fXPos( u32 x ) const { return fMinX( ) + x * fUnitSize( ); }
		f32 fZPos( u32 z ) const { return fMinZ( ) + z * fUnitSize( ); }

		f32 fInstancedUnitSize( ) const { return mInstancedUnitSize; }
		u32 fInstancedCellDiff( ) const { return mInstancedCellDiff; }

		// Debug
		u32 fCalculateSize( ) const;

	public:

		static void fConvertMask( 
			u32 dimx, u32 dimz, const f32 inMask[], 
			u32 paintUnits, 
			u32 & maskDimX, u32 & maskDimZ, tDynamicArray<byte> & outMask );

	public:

		f32 mUnitSize; // size of one box
		u32 mPaintUnits;
		u32 mDimX, mDimZ;
		u32 mMaskDimX, mMaskDimZ;
		u32 mMaxUnitSpawns;
		
		tEnum<tRotation, byte> mRotation;
		tEnum<tTranslation, byte> mTranslation;
		tEnum<tVisibility, byte> mFarVisibility;
		tEnum<tVisibility, byte> mNearVisibility;

		f32 mYRotationScale;
		f32 mXZRotationScale;
		f32 mXZTranslationScale;
		f32 mYTranslationScale;
		f32 mScaleRangeAdjustor;

		f32 mWorldLengthX, mWorldLengthZ;

		tDynamicArray<tElement> mElements;
		tDynamicArray<byte> mMask;
		tDynamicArray<f32> mHeights;

		f32 mInstancedUnitSize;
		u32 mInstancedCellDiff;
		tDynamicArray< tDynamicArray< tMat3fArray > > mCells; //[element][cell]
	};

	typedef tDynamicArray<tGroundCoverCloudDef> tGroundCoverCloudDefList;

#ifdef sig_logging
#define gc_debug_cells
#endif//sig_logging

	// Every ground cover cloud can have multiple "elements" to be rendered.
	// Each "element" is basically a sigml with some extra data (see tGroundCoverCloudDef::tElement).
	// ( Most of this element code will go away once we get all matrices baked at assetgen time. )
	//
	// Instanced rendering can only work on a per-renderable basis. So every "element" is searched for
	// all renderables and then the real work can begin. Note: Ascend is currently the only game that
	// actually has multiple renderables within the same sigml.
	struct tInstanceRenderable
	{
		tInstanceRenderable( );
		void fCreateRenderable( const tGroundCoverCloudDef* def, const tInstanceMaster& master );
		void fCopyMatsToGpu( const Math::tMat3f* mats, const u32 count );
		u32												mState;
		tRefCounterPtr< tRenderableEntity >				mEntity; //what gets drawn
		tGeometryBufferVRam								mGpuMats;
	};
	struct tInstanceMaster //this structure can be though of as containing all the common render-batch data for multiple instanced draw calls
	{
		tInstanceMaster( );
		void fInit( const tGroundCoverCloud* cloud, const tRenderableEntity* src, u32 maxInstances );
		tRefCounterPtr<	const tRenderableEntity >		mEntity; //renderable we are copying for instancing
		tVertexFormatVRam								mFormat;
#ifdef platform_xbox360
		tIndexBufferVRam								mIndicies;
#endif//platform_xbox360
	};
	struct tInstancedCell
	{
		tDynamicArray< tInstanceRenderable > mRenderables;
		void fInit( const tGroundCoverCloudDef* def, const tDynamicArray< tInstanceMaster >& masters );
		void fInvalidate( const Math::tMat3f& otw, const tMat3fArray& mats, b32 asyncRealloc );
	};
	struct tInstanceGrid
	{
		static u32 base_export fDimX( const tGroundCoverCloudDef * def );
		static u32 base_export fDimZ( const tGroundCoverCloudDef * def );
		static u32 base_export fConvert( const tGroundCoverCloudDef * def, u32 x, u32 z );
		static void base_export fConvert( const tGroundCoverCloudDef * def, Math::tRectu& r );
		
		tInstanceGrid( );
		void fInit( const tGroundCoverCloud* cloud, const tDynamicArray< tInstanceMaster >& masters );
		void fGetIntersectingCells( u32 minX, u32 maxX, u32 minZ, u32 maxZ, const tCamera& camera, tGrowableArray<u32>& cells, const tDynamicArray< tMat3fArray >& cellMats ) const;
		void fAppendRenderables( tGrowableArray< tRenderableEntity* >& out, const tGrowableArray<u32>& cells );
		void fInvalidate( const tGrowableArray<u32>& cells, const tDynamicArray< tMat3fArray >& mats );
		void fInvalidateAll( const tDynamicArray< tMat3fArray >& mats );
		void fDeleteEverything( );
		const tGroundCoverCloud* mCloud;
		tDynamicArray< tInstancedCell > mCells;
		tDynamicArray< tPair< f32, f32 > > mHeights; //min/max

		//Debug Stuff
#ifdef gc_debug_cells
		void fSetupDebugDraw( );
		void fUpdateDebugDraw( const tGrowableArray<u32>& cells, const Gfx::tCamera& camera );

		b32 mSetup;
		tDynamicArray< tRefCounterPtr< Gui::tColoredQuad > > mDebugQuads;
		tRefCounterPtr< Gui::tColoredQuad > mDebugFrust;
#endif//gc_debug_cells
	};
	struct tBakedElement
	{
		tGroundCoverCloud*										mCloud;
		tRefCounterPtr< const tSceneRefEntity >					mSigml;
		const tDynamicArray< tMat3fArray >*						mMats;
		tDynamicArray< tInstanceMaster >						mRenderableMasters;
		tInstanceGrid											mGrid;
		tBakedElement( );
		void fInit( tGroundCoverCloud* cloud, tSceneRefEntity* sre, const tDynamicArray< tMat3fArray >* mats, const u32 maxInstances );
		void fAppendRenderables( tGrowableArray< tRenderableEntity* >& out, const tCamera& camera );
	};
	class tInstancedGroundCoverCloud
	{
	public:
		static u32 (*gGetAllowedRenderableState)( f32 x, f32 z );
	private:
		tGroundCoverCloud* mCloud;
		tDynamicArray< tBakedElement > mBakedElements;
	public:
		tInstancedGroundCoverCloud( );
		~tInstancedGroundCoverCloud( );
		void fInit( tGroundCoverCloud* cloud );
		void fUpdate( const tCamera& camera );
		void fInvalidate( f32 x, f32 z, f32 minRadius, f32 maxRadius );
		void fDeleteEverything( );
	};

	///
	/// \class tGroundCoverCloud
	/// \brief 
	class base_export tGroundCoverCloud : public tEntityCloud 
	{
		define_dynamic_cast(tGroundCoverCloud, tEntityCloud);

	public:

		static void fSetGlobalDisable( b32 disable );
		static b32 fGetGlobalDisable( );

		static void fSetGatherRadius( tGroundCoverCloudDef::tVisibility visibility, f32 radius );
		static f32 fGetGatherRadius( tGroundCoverCloudDef::tVisibility visibility );

		// Use tGroundCoverCloudDef::cVisibilityCount to disable globally forced visibility level
		static void fSetForcedVisibility( tGroundCoverCloudDef::tVisibility visibility  );
		static b32 fGetForcedVisibility( );

		typedef tDelegate< b32 ( tRenderableEntity * )> tGroundCoverFilter;
		static void fSetGroundCoverFilter( const tGroundCoverFilter & filter );

		struct tSpawnInfo
		{
			b32 mDoSpawn;
			Math::tMat3f mLocalXform;
			const tGroundCoverCloudDef::tElement * mElement;
		};
		static b32 fGetRotation( const tGroundCoverCloudDef * def, tMersenneGenerator & random, Math::tEulerAnglesf & angles );
		static b32 fGetTranslation( const tGroundCoverCloudDef * def, tMersenneGenerator & random, Math::tVec3f & offset );
		static b32 fGetScale( const tGroundCoverCloudDef * def, tMersenneGenerator & random, f32 & scale );
		static void fGenSpawnInfo(
			const tGroundCoverCloudDef * def,
			u32 cX, u32 cZ, f32 pX, f32 pZ,
			tGrowableArray<tSpawnInfo> & spawns,
			b32 skipHeights = false );

	public:

		explicit tGroundCoverCloud( const tGroundCoverCloudDef * def );
		~tGroundCoverCloud( );

		virtual b32 fShouldRender( ) const;
		virtual void fPrepareRenderables( const Gfx::tCamera & camera );
		virtual void fGatherRenderables( tGatherCb * cb, b32 forShadows );
		virtual void fCleanRenderables( );
		virtual void fInitGCInstancing( );

		void fInvalidateRenderables( f32 x, f32 z, f32 minRadius, f32 maxRadius );

		const tGroundCoverCloudDef* fDef( ) const { return mDef; }
		f32 fUnitSize( ) const { return mDef->fUnitSize( ); }
		f32 fMinX( ) const { return mDef->fMinX( ); }
		f32 fMinZ( ) const { return mDef->fMinZ( ); }
		f32 fMaxX( ) const { return mDef->fMaxX( ); }
		f32 fMaxZ( ) const { return mDef->fMaxZ( ); }

		f32 fXPos( u32 x ) const { return mDef->fXPos( x ); }
		f32 fZPos( u32 z ) const { return mDef->fZPos( z ); }

		void fUpdateAllCellBounds( );

	protected:

		typedef tGrowableArray< tSceneRefEntityPtr > tUnusedArray;
		typedef tHashTable<tFilePathPtr, tUnusedArray > tUnusedTable;

		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnMoved( b32 recomputeParentRelative );

		void fAllocateCells( u32 newCount );
		void fGatherRenderables( u32 minX, u32 minZ, u32 maxX, u32 maxZ, tGatherCb * cb, b32 forShadows );
		void fInstantiateCells( u32 minX, u32 minZ, u32 maxX, u32 maxZ, b32 force );

	public:
		Math::tRectu fGenInvalidateRect( f32 worldX, f32 worldZ, f32 radius ) const;

		tInstancedGroundCoverCloud mInstancedGC;

		///
		/// \class tRenderList
		/// \brief 
		class base_export tRenderList
		{
			struct tParent;
		public:

			void fSpawn( 
				tUnusedTable & table,
				const tGroundCoverCloudDef * def,
				tLinearFrustumCulling & lfc,
				const Math::tMat3f & ownerWorld,
				u32 cX, u32 cZ, f32 pX, f32 pZ );
			void fMoveTo( 
				const tGroundCoverCloudDef* def,
				tLinearFrustumCulling & lfc,
				const Math::tMat3f & ownerWorld,
				u32 cX, u32 cZ, f32 pX, f32 pZ );
			void fApplyProperties( 
				const tGroundCoverCloudDef * def,
				tLinearFrustumCulling & lfc, 
				const tGroundCoverCloudDef::tElement & element );
			void fUpdateCulling( 
				tLinearFrustumCulling & lfc );
			void fDestroy( 
				tUnusedTable & table, 
				tLinearFrustumCulling & lfc );

			u32 fCount( ) const { return mRenderables.fCount( ); }
			tRenderableEntity * & operator[]( u32 i ) { return mRenderables[ i ]; }
			tRenderableEntity * const & operator[]( u32 i ) const { return mRenderables[ i ]; }

		private:

			void fApplyProperties( 
				const tGroundCoverCloudDef * def,
				tLinearFrustumCulling & lfc,
				tParent & parent, 
				const tGroundCoverCloudDef::tElement & element, 
				tRenderableEntity * renderables[] );

		private:

			struct tParent
			{
				tSceneRefEntityPtr mEntity;
				u32 mRenderableStart, mRenderableEnd;
			};

			tDynamicArray< tRenderableEntity* > mRenderables;
			tDynamicArray< tParent > mParents;
		};

		///
		/// \class tCell
		/// \brief 
		class base_export tCell
		{ 
		public: 

			tCell( ) 
				: mInstantiated( false )
				, mBounds( -1.f )
			{ }

			b32 fIsInstantiated( ) const { return mInstantiated; }
			b32 fIsDeadOrEmpty( ) const { return !mInstantiated || !mList.fCount( ); }

			tRenderList & fRenderList( ) { sigassert( mInstantiated ); return mList; }
			const tRenderList & fRenderList( ) const { sigassert( mInstantiated ); return mList; }

			void fUpdateCulling( tLinearFrustumCulling& lfc ) { mList.fUpdateCulling( lfc ); }

			void fInstantiate( );

			void fDestroy( tUnusedTable & table, tLinearFrustumCulling & lfc ) { mList.fDestroy( table, lfc ); mInstantiated = false; }

		public:

			b32 mInstantiated;
			tRenderList mList;
			Math::tSpheref mBounds;
		};

	protected:

		void fUpdateCellBounds( u32 x, u32 z );
		void fDestroyCell( tCell & cell );
		void fUpdateGatherRect( u32 minX, u32 minZ, u32 maxX, u32 maxZ );
		
	protected:


		const tGroundCoverCloudDef * mDef;
		tDynamicArray< tCell > mCells;
		tUnusedTable mUnused;
		Math::tRectu mGatherRect;
		Math::tFrustumf mFrustum;
		b32 mFrustumDirty;
	};
}}

#endif//__tGroundCoverCloud__
