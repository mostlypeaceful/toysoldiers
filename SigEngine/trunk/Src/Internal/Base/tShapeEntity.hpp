//------------------------------------------------------------------------------
// \file tShapeEntity.hpp - 06 Sep 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tShapeEntity__
#define __tShapeEntity__
#include "tEntityDef.hpp"
#include "tSpatialEntity.hpp"

namespace Sig
{
	namespace Math { class tConvexHull; }
	namespace Physics { class tCollisionShape; }

	///
	/// \class tShapeEntityDef
	/// \brief 
	class base_export tShapeEntityDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tShapeEntityDef, 0x5714AB93 );
	public:
		enum tShapeType
		{
			cShapeTypeBox,
			cShapeTypeSphere,
			cShapeTypeConvexHull,
			cShapeTypeCylinder,
			cShapeTypeCapsule,
			// last
			cShapeTypeCount
		};

		// Data pertaining to the potential visiblity set.
		struct tPVSInformation
		{
			declare_reflector( );
			tLoadInPlaceStringPtr* mSetName;
			b32 mInvert;

			tPVSInformation( tLoadInPlaceStringPtr* setName = NULL, b32 invert = false )
				: mSetName( setName )
				, mInvert( invert )
			{ }

			tPVSInformation( tNoOpTag )
			{ }
		};

	public:
		tEnum<tShapeType,u8> mShapeType;
		u8 pad0;
		u16 mStateMask;
		tLoadInPlacePtrWrapper<Math::tConvexHull> mHull;
		tDynamicArray<tPVSInformation> mPotentialVisibility;

	public:
		tShapeEntityDef( );
		tShapeEntityDef( tNoOpTag );
		~tShapeEntityDef( );

		virtual void fCollectEntities( const tCollectEntitiesParams& params ) const;
	};

	///
	/// \class tShapeEntity
	/// \brief 
	class base_export tShapeEntity : public tSpatialEntity
	{
		debug_watch( tShapeEntity );
		define_dynamic_cast( tShapeEntity, tSpatialEntity );
	public:
		static const u32 cSpatialSetIndex;

		static void fSetCollisionEnabled( tEntity& root, b32 enabled );

	public:
		~tShapeEntity( );

		inline u32 fShapeType( ) const { return mShapeType; }
		inline b32 fEnabled( ) const { return mEnabled; }
		void fSetEnabled( b32 enabled );
		void fCreateCollision( );
		void fUpdateCollision( );

		//------------------------------------------------------------------------------
		// tEntity overrides
		//------------------------------------------------------------------------------
		virtual const tEntityDef* fQueryEntityDef( ) const { return mEntityDef; }
		virtual void fPropagateSkeleton( Anim::tAnimatedSkeleton& skeleton );
		virtual b32 fIsHelper( ) const { return true; }

		//------------------------------------------------------------------------------
		// tSpatialEntity overrides
		//------------------------------------------------------------------------------
		virtual u32	fSpatialSetIndex( ) const { return cSpatialSetIndex; }
		virtual void fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const = 0;
		virtual b32 fIntersects( const Math::tFrustumf& v ) const = 0;
		virtual b32 fIntersects( const Math::tAabbf& v ) const = 0;
		virtual b32 fIntersects( const Math::tObbf& v ) const = 0;
		virtual b32 fIntersects( const Math::tSpheref& v ) const = 0;

		//------------------------------------------------------------------------------
		// tStateableEntity overrides
		//------------------------------------------------------------------------------
		virtual void fStateMaskEnable( u32 index ) { fSetEnabled( fStateEnabled( index ) ); }

		//------------------------------------------------------------------------------
		// Abstract interface
		//------------------------------------------------------------------------------
		virtual b32 fContains( const Math::tVec3f& point ) const = 0;
		virtual b32 fContains2D( const Math::tVec3f& point ) const = 0;
		virtual b32 fIntersects( const tShapeEntity& otherShape ) const = 0;
		virtual Math::tVec3f fClosestPoint( const Math::tVec3f& point ) const = 0;
		virtual Math::tVec3f fSupport( const Math::tVec3f& dir ) const = 0;
		if_devmenu( virtual void fDebugRender( const Math::tVec4f& color = Math::tVec4f( 1, 1, 1, 0.5f ), b32 doBasis = true ) const = 0 );
		
		Math::tObbf fParentRelativeBox( ) const;
		Math::tSpheref fParentRelativeSphere( ) const;

		// Legacy support, will assert if this shape is not the right type
		const Math::tObbf& fBox( ) const;
		const Math::tSpheref& fSphere( ) const;

	protected:
		tShapeEntity( const tShapeEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld, b32 createCollision );
		tShapeEntity( const Math::tAabbf& objectSpaceBox, tShapeEntityDef::tShapeType shape, b32 createCollision );
		
		const tShapeEntityDef*		mEntityDef;
		u32							mShapeType;
		b8							mEnabled;
		b8							mCreateCollision;
		b16							pad0;

		// This is optional and may not be set
		tRefCounterPtr< Physics::tCollisionShape > mCollisionShape;

		virtual void fOnSpawn( );
		virtual void fOnDelete( );

	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tShapeEntity );

	///
	/// \class tSphereEntity
	/// \brief 
	class base_export tSphereEntity : public tShapeEntity
	{
		debug_watch( tSphereEntity );
		define_dynamic_cast( tSphereEntity, tShapeEntity );
		define_class_pool_new_delete( tSphereEntity, 128 );
	public:

		tSphereEntity( const tShapeEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld, b32 createCollision );
		tSphereEntity( const Math::tSpheref& sphere, b32 createCollision = false );

		inline const Math::tSpheref& fSphere( ) const { return mSphere; }
		inline const Math::tVec3f&	 fCenter( ) const { return mSphere.fCenter( ); }
		inline f32					 fRadius( ) const { return mSphere.fRadius( ); }

		//------------------------------------------------------------------------------
		// tEntity overrides
		//------------------------------------------------------------------------------
		virtual void fOnMoved( b32 recomputeParentRelative );

		//------------------------------------------------------------------------------
		// tSpatialEntity overrides
		//------------------------------------------------------------------------------
		virtual void fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;
		virtual b32 fIntersects( const Math::tFrustumf& v ) const;
		virtual b32 fIntersects( const Math::tAabbf& v ) const;
		virtual b32 fIntersects( const Math::tObbf& v ) const;
		virtual b32 fIntersects( const Math::tSpheref& v ) const;

		//------------------------------------------------------------------------------
		// tShapeEntity overrides
		//------------------------------------------------------------------------------
		virtual b32 fContains( const Math::tVec3f& point ) const;
		virtual b32 fContains2D( const Math::tVec3f& point ) const;
		virtual b32 fIntersects( const tShapeEntity& otherShape ) const;
		virtual Math::tVec3f fClosestPoint( const Math::tVec3f& point ) const;
		virtual Math::tVec3f fSupport( const Math::tVec3f& dir ) const;

		if_devmenu( virtual void fDebugRender( const Math::tVec4f& color, b32 doBasis ) const );

	private:

		Math::tSpheref fComputeSphere( ) const;

	private:

		Math::tSpheref mSphere;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tSphereEntity );

	///
	/// \class tBoxEntity
	/// \brief 
	class base_export tBoxEntity : public tShapeEntity
	{
		debug_watch( tBoxEntity );
		define_dynamic_cast( tBoxEntity, tShapeEntity );
		define_class_pool_new_delete( tBoxEntity, 128 );
	public:

		tBoxEntity( const tShapeEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld, b32 createCollision );
		tBoxEntity( const Math::tAabbf& box, b32 createCollision = false );

		inline const Math::tObbf& fBox( ) const { return mBox; }

		//------------------------------------------------------------------------------
		// tEntity overrides
		//------------------------------------------------------------------------------
		virtual void fOnMoved( b32 recomputeParentRelative );

		//------------------------------------------------------------------------------
		// tSpatialEntity overrides
		virtual void fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const;
		virtual b32 fIntersects( const Math::tFrustumf& v ) const;
		virtual b32 fIntersects( const Math::tAabbf& v ) const;
		virtual b32 fIntersects( const Math::tObbf& v ) const;
		virtual b32 fIntersects( const Math::tSpheref& v ) const;

		//------------------------------------------------------------------------------
		// tShapeEntity overrides
		//------------------------------------------------------------------------------
		virtual b32 fContains( const Math::tVec3f& point ) const;
		virtual b32 fContains2D( const Math::tVec3f& point ) const;
		virtual b32 fIntersects( const tShapeEntity& otherShape ) const;
		virtual Math::tVec3f fClosestPoint( const Math::tVec3f& point ) const;
		virtual Math::tVec3f fSupport( const Math::tVec3f& dir ) const;

		if_devmenu( virtual void fDebugRender( const Math::tVec4f& color, b32 doBasis ) const );
		//------------------------------------------------------------------------------

		
	private:

		Math::tObbf fComputeBox( ) const;

	private:

		Math::tObbf mBox;
	};

	define_smart_ptr( base_export, tRefCounterPtr, tBoxEntity );

	///
	/// \class tConvexHullEntity
	/// \brief 
	class base_export tConvexHullEntity : public tShapeEntity
	{
		debug_watch( tConvexHullEntity );
		define_dynamic_cast( tConvexHullEntity, tShapeEntity );
		define_class_pool_new_delete( tConvexHullEntity, 128 );
	public:

		tConvexHullEntity( const tShapeEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld, b32 createCollision );

		inline const Math::tConvexHull& fObjectSpaceHull( ) const { sigassert( mEntityDef ); return *mEntityDef->mHull; }

		//------------------------------------------------------------------------------
		// tSpatialEntity overrides
		// THESE THINGS ARE SERIOUSLY BLOATED.
		//------------------------------------------------------------------------------
		virtual void fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const { tSpatialEntity::fRayCast( ray, hit ); }
		virtual void fOnMoved( b32 recomputeParentRelative ) { tShapeEntity::fOnMoved( recomputeParentRelative ); fUpdateCollision( ); }
		virtual b32 fIntersects( const Math::tFrustumf& v ) const { return tSpatialEntity::fIntersects( v ); }
		virtual b32 fIntersects( const Math::tAabbf& v ) const { return tSpatialEntity::fIntersects( v ); }
		virtual b32 fIntersects( const Math::tObbf& v ) const { return tSpatialEntity::fIntersects( v ); }
		virtual b32 fIntersects( const Math::tSpheref& v ) const { return tSpatialEntity::fIntersects( v ); }

		//------------------------------------------------------------------------------
		// tShapeEntity overrides
		//------------------------------------------------------------------------------
		virtual b32 fContains( const Math::tVec3f& point ) const;
		virtual b32 fContains2D( const Math::tVec3f& point ) const;
		virtual b32 fIntersects( const tShapeEntity& otherShape ) const { return false; }
		virtual Math::tVec3f fClosestPoint( const Math::tVec3f& point ) const { return Math::tVec3f::cZeroVector; }
		virtual Math::tVec3f fSupport( const Math::tVec3f& dir ) const { return Math::tVec3f::cZeroVector; }
		if_devmenu( virtual void fDebugRender( const Math::tVec4f& color, b32 doBasis ) const );

	};

	define_smart_ptr( base_export, tRefCounterPtr, tConvexHullEntity );

}

#endif//__tShapeEntity__
