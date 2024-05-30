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

namespace Sig
{
	class tSceneRefEntity;
	class tMersenneGenerator;
}

namespace Sig { namespace Gfx 
{

	class tGroundCoverCloud;

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
			cVisibilityForever,

			cVisibilityCount
		};

	public:

		tGroundCoverCloudDef( tNoOpTag ) : mElements( cNoOpTag ), mMask( cNoOpTag ), mHeights( cNoOpTag ) { } 
		tGroundCoverCloudDef( ) 
			: mUnitSize( 0 )
			, mPaintUnits( 0 )
			, mMaskDimX( 0 )
			, mMaskDimZ( 0 )
			, mDimX( 0 )
			, mDimZ( 0 )
			, mMaxUnitSpawns( 0 )
			, mRotation( cRotationNone )
			, mTranslation( cTranslationNone )
			, mYRotationScale( 0 )
			, mXZRotationScale( 0 )
			, mXZTranslationScale( 0 )
			, mYTranslationScale( 0 )
			, mScaleRangeAdjustor( 0 )
			, mWorldLengthX( 0 )
			, mWorldLengthZ( 0 ) { }

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

		tVisibility fVisibility( ) const;

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
		tEnum<tVisibility, byte> mVisibility;

		f32 mYRotationScale;
		f32 mXZRotationScale;
		f32 mXZTranslationScale;
		f32 mYTranslationScale;
		f32 mScaleRangeAdjustor;

		f32 mWorldLengthX, mWorldLengthZ;

		tDynamicArray<tElement> mElements;
		tDynamicArray<byte> mMask;

		tDynamicArray<f32> mHeights;
	};

	typedef tDynamicArray<tGroundCoverCloudDef> tGroundCoverCloudDefList;

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

	public:

		tGroundCoverCloud( const tGroundCoverCloudDef * def );
		~tGroundCoverCloud( );

		virtual void fPrepareRenderables( const Gfx::tCamera & camera );
		virtual void fGatherRenderables( tGatherCb * cb, b32 forShadows );
		virtual void fCleanRenderables( );

		f32 fUnitSize( ) const { return mDef->fUnitSize( ); }

		f32 fMinX( ) const { return -0.5f * mDef->fWorldLengthX( ) + ( 0.5f * mDef->fUnitSize( ) ); }
		f32 fMinZ( ) const { return -0.5f * mDef->fWorldLengthZ( ) + ( 0.5f * mDef->fUnitSize( ) ); }
		f32 fMaxX( ) const { return 0.5f * mDef->fWorldLengthX( ) + ( 0.5f * mDef->fUnitSize( ) ); }
		f32 fMaxZ( ) const { return 0.5f * mDef->fWorldLengthZ( ) + ( 0.5f * mDef->fUnitSize( ) ); }

		f32 fXPos( u32 x ) const { return fMinX( ) + x * mDef->fUnitSize( ); }
		f32 fZPos( u32 z ) const { return fMinZ( ) + z * mDef->fUnitSize( ); }


		struct tSpawnInfo
		{
			b32 mDoSpawn;
			Math::tMat3f mLocalXform;
			const tGroundCoverCloudDef::tElement * mElement;
		};

	public:

#if target_tools

		static void fSpawnCell(
			const tGroundCoverCloudDef * def,
			u32 cX, u32 cZ, f32 pX, f32 pZ,
			tGrowableArray<tSpawnInfo> & spawns,
			b32 skipHeights = false);

#endif

	protected:

		typedef tHashTable<tFilePathPtr, tGrowableArray< tSceneRefEntityPtr > > tUnusedTable;

		virtual void fOnSpawn( );
		virtual void fOnMoved( b32 recomputeParentRelative );

		void fAllocateCells( u32 newCount );
		void fAllocateCells( u32 newCount, tUnusedTable & unused );
		void fGatherRenderables( u32 minX, u32 minZ, u32 maxX, u32 maxZ, tGatherCb * cb, b32 forShadows );
		void fInstantiateCells( u32 minX, u32 minZ, u32 maxX, u32 maxZ, b32 force );

		///
		/// \class tRenderList
		/// \brief 
		class base_export tRenderList
		{
			struct tParent;

		public:

			
		public:

			const Math::tMat3f & fObjectToWorld( ) 
			{ return mParents.fCount( ) ? mParents[ 0 ].mEntity->fObjectToWorld( ) : Math::tMat3f::cIdentity; }

			void fSpawn( 
				tUnusedTable & table,
				const tGroundCoverCloudDef * def, 
				const Math::tMat3f & ownerWorld, 
				u32 cX, u32 cZ, f32 pX, f32 pZ );
			
			static void fSpawn(
				const tGroundCoverCloudDef * def,
				u32 cX, u32 cZ, f32 pX, f32 pZ,
				tGrowableArray<tSpawnInfo> & spawns,
				b32 skipHeights = false);

			void fMoveTo( 
				const tGroundCoverCloudDef * def, 
				const Math::tMat3f & ownerWorld, 
				u32 cX, u32 cZ, f32 pX, f32 pZ );
			void fApplyProperties( const tGroundCoverCloudDef * def, const tGroundCoverCloudDef::tElement & element );

			void fClear( ) { mRenderables.fClear( ); mParents.fClear( ); }
			void fDestroy( ) { mRenderables.fDeleteArray( ); mParents.fDeleteArray( ); }
			void fDestroy( tUnusedTable & table );

			u32 fCount( ) const { return mRenderables.fCount( ); }
			tRenderableEntity * & operator[]( u32 i ) { return mRenderables[ i ]; }
			tRenderableEntity * const & operator[]( u32 i ) const { return mRenderables[ i ]; }

		private:

			void fApplyProperties( 
				const tGroundCoverCloudDef * def,
				tParent & parent, 
				const tGroundCoverCloudDef::tElement & element, 
				tRenderableEntity * renderables[] );


			static b32 fGetRotation( const tGroundCoverCloudDef * def, tMersenneGenerator & random, Math::tEulerAnglesf & angles );
			static b32 fGetTranslation( const tGroundCoverCloudDef * def, tMersenneGenerator & random, Math::tVec3f & offset );
			static b32 fGetScale( const tGroundCoverCloudDef * def, tMersenneGenerator & random, f32 & scale );

		private:

			struct tParent
			{
				tSceneRefEntityPtr mEntity;
				u32 mRenderableStart, mRenderableEnd;
			};

			tArraySleeve< tRenderableEntity * > mRenderables;
			tArraySleeve< tParent > mParents;
		};

		///
		/// \class tCell
		/// \brief 
		class base_export tCell
		{ 
		public: 

			tCell( ) : mInstantiated( false ) { }

			b32 fIsInstantiated( ) const { return mInstantiated; }
			b32 fIsDeadOrEmpty( ) const { return !mInstantiated || !mList.fCount( ); }

			tRenderList & fRenderList( ) { sigassert( mInstantiated ); return mList; }
			const tRenderList & fRenderList( ) const { sigassert( mInstantiated ); return mList; }

			void fInstantiate( tRenderList & oldOwner );
			void fRelease( tRenderList & newOwner );

			void fDestroy( ) { mList.fDestroy( ); mInstantiated = false; }
			void fDestroy( tUnusedTable & table ) { mList.fDestroy( table ); mInstantiated = false; }

		private:

			b32 mInstantiated;
			tRenderList mList;
		};

	private:
		
		void fInstantiateCell( tCell & cell, u32 cX, u32 cZ, f32 pX, f32 pZ );
		void fDestroyCell( tCell & cell );
		void fUpdateGatherRect( u32 minX, u32 minZ, u32 maxX, u32 maxZ );
		
	protected:


		const tGroundCoverCloudDef * mDef;
		tDynamicArray< tCell > mCells;
		tUnusedTable mUnused;
		Math::tRectu mGatherRect;
	};
}}

#endif//__tGroundCoverCloud__
