//------------------------------------------------------------------------------
// \file tMeshEntity.hpp - 15 Aug 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tMeshEntity__
#define __tMeshEntity__
#include "tEntityDef.hpp"
#include "tSkinMap.hpp"
#include "Gfx/tRenderableEntity.hpp"

namespace Sig
{
	class tMesh;
	class tSubMesh;
	class tMeshEntity;
	class tScriptObject;
	namespace Physics { class tCollisionShape; }
	namespace Gfx { tVisibilitySetRef; }

	///
	/// \class tMeshEntityDef
	/// \brief 
	class base_export tMeshEntityDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tMeshEntityDef, 0xC1A483B7 );
	public:
		enum tStateType
		{
			cStateTypeState,
			cStateTypeTransition,
			cStateTypeCount
		};
	public:
		tMesh*						mMesh;
		u16							mStateMask;
		tEnum<tStateType,u8>		mStateType;
		u8							pad0;
		f32							mSortOffset;

	public:
		tMeshEntityDef( );
		tMeshEntityDef( tNoOpTag );
		~tMeshEntityDef( );

		virtual b32 fHasRenderableBounds( ) const { return true; }
		virtual void fOnFileLoaded( );
		virtual void fOnFileUnloading( );
		virtual void fCollectEntities( const tCollectEntitiesParams& params ) const;

		virtual void fSelectLOD( Gfx::tRenderableEntity* entity, f32 ratio, b32 shadows, b32 normals ) const;

		b32 fStateIndexSet( u32 stateIndex ) const { return (mStateMask & (1<<stateIndex)); }
	};

	define_smart_ptr( base_export, tRefCounterPtr, tMeshEntity );

	///
	/// \class tMeshEntity
	/// \brief 
	class base_export tMeshEntity : public Gfx::tRenderableEntity
	{
		debug_watch( tMeshEntity );
		define_dynamic_cast( tMeshEntity, Gfx::tRenderableEntity );
		define_class_pool_new_delete( tMeshEntity, 256 );
	public:

		///
		/// \brief Use this class as the 'forEach' object to tEntity::fForEachDescendent
		struct base_export tChangeMeshState
		{
			s16 mPrevStateIndex;
			s16 mNewStateIndex;
			mutable s16 mHighestState;
			mutable b8 mGatherDefaultTransitionPieces;
			b8 mAllowDefaultTransition;
			mutable tGrowableArray< tRefCounterPtr< tMeshEntity > > mTransitionPieces;
			u32 mIgnoreFlagsMask; //ignores any entities with any of these flags

			b16 mJustChangeState; //no transition stuff
			b16 mIgnoreLogics;
			b16 mCollectAllTransition; //otherwise just transition for 1 state change.
			mutable b16 mCollectTransitionMask; // set during fChangeState

			tChangeMeshState( s16 prevIdx, s16 newIdx, b32 justChangeState = true, b32 allowDefaultTransition = true, u32 ignoreFlagsMask = 0, b32 ignoreLogics = false );
			b32 fProcess( tEntity& entity ) const; //return true if you should continue processing deeper
			void fChangeState( tEntity& root ) const;
			b32 fNothingLeft( ) const { return mHighestState < mNewStateIndex; }
		private:
			void fChangeStateRecursive( tEntity& root, b32 firstChange ) const;
		};

	private:
		const tSubMesh*			mSubMesh;
		const tMeshEntityDef*	mEntityDef;
		tSkinMapPtr				mSkinMap;
		b32						mStateChangeEnabled;
		tResourcePtr			mFromResource;

	protected:
		tRefCounterPtr< Physics::tCollisionShape > mCollisionShape;

		virtual void fOnSpawn( );
		virtual void fOnDelete( );

	public:
		tMeshEntity( 
			const Gfx::tRenderBatchPtr& batchPtr,
			const tMeshEntityDef* entityDef,
			const tSubMesh* subMesh,
			const Math::tAabbf& objectSpaceBox,
			const Math::tMat3f& objectToWorld,
			Gfx::tVisibilitySetRef& visibility,
			tResource* resource );
		virtual ~tMeshEntity( );

		const tSubMesh* fSubMesh( ) const { return mSubMesh; }
		void fSetCollision( Physics::tCollisionShape* collision );

		virtual void fCollectTris( const Math::tObbf& obb, tGrowableArray<Math::tTrianglef>& trisOut ) const;
		virtual void fCollectTris( const Math::tAabbf& aabb, tGrowableArray<Math::tTrianglef>& trisOut ) const;
		
		virtual void fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;
		
		virtual void fUpdateLOD( const Math::tVec3f & eye );
		virtual void fPropagateSkeleton( Anim::tAnimatedSkeleton& skeleton );
		virtual const tEntityDef* fQueryEntityDef( ) const { return mEntityDef; }
		const tMeshEntityDef& fEntityDef( ) const { sigassert( mEntityDef ); return *mEntityDef; }
		u32 fStateMask( ) const { sigassert( mEntityDef ); return mEntityDef->mStateMask; }
		b32 fStateChangeEnabled( ) const { return mStateChangeEnabled; }
		void fSetStateChangeEnabled( b32 set ) { mStateChangeEnabled = set; }

		// from tRenderInstance
		virtual const Math::tMat3f* fRI_ObjectToLocal( ) const { return &mEntityDef->mObjectToLocal; }
		virtual const Math::tMat3f* fRI_LocalToObject( ) const { return &mEntityDef->mLocalToObject; }
		virtual const tSkinMap*		fRI_SkinMap( ) const { return mSkinMap.fGetRawPtr( ); }

		// states
		static void fStateCount( tEntity& parent, s32& statesOut, s32& transitionsOut );
		static void fEnableStateChanges( tEntity& parent, b32 enable );

		// From tStateableEntity
		virtual void fStateMaskEnable( u32 index ) 
		{ 
			//log_line( 0, "Path: " << fEntityDef( ).mMesh->mGeometryFile->fGetResourceIdRawPath( ) << "Index: " << index << " defIndex: " << u32(fEntityDef( ).mStateIndex) );
			fSetDisabled( !fStateEnabled( index ) ); 
		}

		// these are similar to the equivalent member functions of tEntity,
		// but consider only mesh entities that are currently enabled
		static Math::tAabbf fCombinedObjectSpaceBox( tEntity& parent );
		static Math::tAabbf fCombinedWorldSpaceBox( tEntity& parent );

		// use this for the application to override the lod quality
		static u32 gLODForceMesh;

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

}

#endif//__tMeshEntity__
