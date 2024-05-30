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
		s16							mStateIndex;
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
		virtual void fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tMeshEntity );

	class base_export tMeshEntity : public Gfx::tRenderableEntity
	{
		define_dynamic_cast( tMeshEntity, Gfx::tRenderableEntity );
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
			mutable tGrowableArray< tMeshEntityPtr > mTransitionPieces;
			u32 mIgnoreFlagsMask; //ignores any entities with any of these flags

			b16 mJustChangeState; //no transition stuff
			b16 mIgnoreLogics;
			b16 mCollectAllTransition; //otherwise just transition for 1 state change.
			b16 pad0;

			tChangeMeshState( s16 prevIdx, s16 newIdx, b32 justChangeState = true, b32 allowDefaultTransition = true, u32 ignoreFlagsMask = 0, b32 ignoreLogics = false );
			b32 fProcess( tEntity& entity ) const; //return true if you should continue processing deeper
			void fChangeState( tEntity& root ) const;
			b32 fNothingLeft( ) const { return mHighestState < mNewStateIndex; }
		private:
			void fChangeStateRecursive( tEntity& root, b32 firstChange ) const;
		};

	private:
		const tSubMesh*			mSubMesh; // FIXME raw pointer not so safe
		const tMeshEntityDef*	mEntityDef; // FIXME raw pointer not so safe
		tSkinMapPtr				mSkinMap;
		b32						mStateChangeEnabled;
	public:
		tMeshEntity( 
			const Gfx::tRenderBatchPtr& batchPtr,
			const tMeshEntityDef* entityDef,
			const tSubMesh* subMesh,
			const Math::tAabbf& objectSpaceBox,
			const Math::tMat3f& objectToWorld );

		const tSubMesh* fSubMesh( ) const { return mSubMesh; }

		// Exlcude this mesh from raycasts or intersections if this is true.
		inline b32 fIgnore( ) const { return fDisabledOrInvisible( ) || !mSubMesh; }

		virtual void fCollectTris( const Math::tObbf& obb, tGrowableArray<Math::tTrianglef>& trisOut ) const;
		virtual void fCollectTris( const Math::tAabbf& aabb, tGrowableArray<Math::tTrianglef>& trisOut ) const;
		virtual void fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;
		virtual b32 fIntersects( const Math::tFrustumf& v ) const;
		virtual b32	fIntersects( const Math::tAabbf& v ) const;
		virtual b32 fIntersects( const Math::tObbf& v ) const;
		virtual b32 fIntersects( const Math::tSpheref& v ) const;
		virtual void fPropagateSkeleton( tAnimatedSkeleton& skeleton );
		virtual const tEntityDef* fQueryEntityDef( ) const { return mEntityDef; }
		const tMeshEntityDef& fEntityDef( ) const { sigassert( mEntityDef ); return *mEntityDef; }
		u32 fStateIndex( ) const { sigassert( mEntityDef ); return mEntityDef->mStateIndex; }
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
			
			// This is kind of hack until the entity def stores the mask instead of an index. then we'll just check that the bit index is set in the mask
			b32 enable = index == -1 || u32(fEntityDef( ).mStateIndex) == -1 || index == fEntityDef( ).mStateIndex;
			fSetDisabled( !enable ); 
		}

		// these are similar to the equivalent member functions of tEntity,
		// but consider only mesh entities that are currently enabled
		static Math::tAabbf fCombinedObjectSpaceBox( tEntity& parent );
		static Math::tAabbf fCombinedWorldSpaceBox( tEntity& parent );

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

}

#endif//__tMeshEntity__
