#include "BasePch.hpp"
#include "tSpatialEntity.hpp"
#include "tSceneGraph.hpp"

#include "Math/tIntersectionAabbFrustum.hpp"
#include "Math/tIntersectionAabbAabb.hpp"
#include "Math/tIntersectionAabbObb.hpp"
#include "Math/tIntersectionAabbSphere.hpp"

namespace Sig
{

	void tSpatialEntity::fSetObjectSpaceBoxOverride( tEntity& root, const Math::tAabbf& newObjSpaceBox )
	{
		tSpatialEntity* spatial = root.fDynamicCast< tSpatialEntity >( );
		if( spatial )
			spatial->fSetObjectSpaceBox( newObjSpaceBox );
		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fSetObjectSpaceBoxOverride( *root.fChild( i ), newObjSpaceBox );
	}

	tSpatialEntity::tSpatialEntity( )
		: tStateableEntity( 0xFFFF )
		, mObjectSpaceBox( Math::tVec3f( -0.0001f ), Math::tVec3f( +0.0001f ) )
	{
		mWorldSpaceBox = mObjectSpaceBox;
		fResetLazyUpdateListIndex( );
	}

	tSpatialEntity::tSpatialEntity( const Math::tAabbf& objectSpaceBox, u16 stateMask )
		: tStateableEntity( stateMask )
		, mObjectSpaceBox( objectSpaceBox )
	{
		sync_event_v_c( mObjectSpaceBox, tSync::cSCSpatial );
		mWorldSpaceBox = mObjectSpaceBox;
		fResetLazyUpdateListIndex( );
	}

	void tSpatialEntity::fOnSpawn( )
	{
		sigassert( fSceneGraph( ) );
		fSceneGraph( )->fSpatialInsert( *this );
		tEntity::fOnSpawn( );
	}

	void tSpatialEntity::fOnDelete( )
	{
		sigassert( fSceneGraph( ) );
		fSceneGraph( )->fSpatialRemove( *this );
		tEntity::fOnDelete( );
	}

	void tSpatialEntity::fToObjectSpace( Math::tFrustumf& fInObject, const Math::tFrustumf& fInWorld ) const
	{
		const Math::tMat4f worldToObject = Math::tMat4f( fObjectToWorld( ) ).fTranspose( );
		fInWorld.fTransform( fInObject, worldToObject );
		fInObject.fNormalize( );
	}

	void tSpatialEntity::fSetObjectSpaceBox( const Math::tAabbf& objectSpaceBox )
	{
		mObjectSpaceBox = objectSpaceBox;
		fRecomputeWorldBounds( );
	}

	void tSpatialEntity::fSetObjectSpaceBox( const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld )
	{
		mObjectSpaceBox = objectSpaceBox;
		mObjectToWorld = objectToWorld;
		fOnMoved( true ); // calls fRecomputeWorldBounds internally
	}

	void tSpatialEntity::fRayCastDefault( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
		const Math::tRayf localSpaceRay = ray.fTransform( fWorldToObject( ) );

		f32 t = Math::cInfinity;
		if( fObjectSpaceBox( ).fIntersectsWalls( localSpaceRay, t ) )
			hit = Math::tRayCastHit( t, Math::tVec3f::cYAxis );
	}

	void tSpatialEntity::fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
	}

	b32	tSpatialEntity::fIntersects( const Math::tFrustumf& v ) const
	{
		if( fQuickRejectByFlags( ) )
			return false;
		return Math::tIntersectionAabbFrustum<f32>( mWorldSpaceBox, v ).fIntersects( );
	}

	b32 tSpatialEntity::fIntersects( const Math::tAabbf& v ) const
	{
		if( fQuickRejectByFlags( ) )
			return false;
		return Math::tIntersectionAabbAabb<f32>( mWorldSpaceBox, v ).fIntersects( );
	}

	b32 tSpatialEntity::fIntersects( const Math::tObbf& v ) const
	{
		if( fQuickRejectByFlags( ) )
			return false;
		return Math::tIntersectionAabbObb<f32>( mWorldSpaceBox, v ).fIntersects( );
	}

	b32 tSpatialEntity::fIntersects( const Math::tSpheref& v ) const
	{
		if( fQuickRejectByFlags( ) )
			return false;
		return Math::tIntersectionAabbSphere<f32>( mWorldSpaceBox, v ).fIntersects( );
	}

	void tSpatialEntity::fOnMoved( b32 recomputeParentRelative )
	{
		tEntity::fOnMoved( recomputeParentRelative );
		fRecomputeWorldBounds( );
	}

	void tSpatialEntity::fRecomputeWorldBounds( )
	{
		//if( fQuickRejectByFlags( ) )
		//	return;
		mWorldSpaceBox = fObjectSpaceBox( ).fTransform( fObjectToWorld( ) );
		sync_event_v_c( mWorldSpaceBox, tSync::cSCSpatial );
		if( fSceneGraph( ) )
			fSceneGraph( )->fSpatialMove( *this );
	}

}
