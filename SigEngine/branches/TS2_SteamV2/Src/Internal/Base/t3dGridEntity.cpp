#include "BasePch.hpp"
#include "tSceneGraph.hpp"
#include "t3dGridEntity.hpp"

#include "Math/tIntersectionRayObb.hpp"
#include "Math/tIntersectionRaySphere.hpp"
#include "Math/tIntersectionAabbObb.hpp"
#include "Math/tIntersectionAabbSphere.hpp"
#include "Math/tIntersectionSphereObb.hpp"

namespace Sig
{

	t3dGridEntityDef::t3dGridEntityDef( )
		: mCellCounts( 0, 0, 0 )
	{
	}

	t3dGridEntityDef::t3dGridEntityDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
		, mCellCounts( cNoOpTag )
	{
	}

	void t3dGridEntityDef::fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const
	{
		t3dGridEntity* entity = NEW t3dGridEntity( this, mBounds, mObjectToLocal );
		fApplyPropsAndSpawnWithScript( *entity, parent, creationFlags );
	}

	t3dGridEntity::t3dGridEntity( const t3dGridEntityDef* entityDef, const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld )
		: tSpatialEntity( objectSpaceBox )
		, mEntityDef( entityDef )
		, mCellCounts( entityDef->mCellCounts )
	{
		fMoveTo( objectToWorld );
	}

	t3dGridEntity::t3dGridEntity( const Math::tAabbf& objectSpaceBox, const Math::tVec3u& cellCounts )
		: tSpatialEntity( Math::tAabbf( -1.f, +1.f ) )
		, mEntityDef( NULL )
		, mCellCounts( cellCounts )
	{
		Math::tMat3f objectToWorld = Math::tMat3f::cIdentity;
		objectToWorld.fSetTranslation( objectSpaceBox.fComputeCenter( ) );
		objectToWorld.fScaleLocal( 0.5f * objectSpaceBox.fComputeDiagonal( ) );

		fMoveTo( objectToWorld );
	}

	void t3dGridEntity::fPropagateSkeleton( tAnimatedSkeleton& skeleton )
	{
		fPropagateSkeletonInternal( skeleton, mEntityDef );
	}

	b32 t3dGridEntity::fContains( const Math::tVec3f& point ) const
	{
		return mBox.fContains( point );
	}

	b32 t3dGridEntity::fIntersects( const t3dGridEntity& otherShape ) const
	{
		return otherShape.fIntersects( mBox );
	}

	void t3dGridEntity::fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
		Math::tIntersectionRayObb<f32> i( ray, mBox );
		if( i.fIntersects( ) )
		{
			hit.mT = i.fT( );
			hit.mN = ( ray.fEvaluate( hit.mT ) - mBox.fCenter( ) ).fNormalizeSafe( Math::tVec3f::cYAxis ); // sphere-ized normal, not accurate for a box
		}
	}

	b32 t3dGridEntity::fIntersects( const Math::tFrustumf& v ) const
	{
		return tSpatialEntity::fIntersects( v );
	}

	b32 t3dGridEntity::fIntersects( const Math::tAabbf& v ) const
	{
		return Math::tIntersectionAabbObb<f32>( mBox, v ).fIntersects( );
	}

	b32 t3dGridEntity::fIntersects( const Math::tObbf& v ) const
	{
		return tSpatialEntity::fIntersects( v );
	}

	b32 t3dGridEntity::fIntersects( const Math::tSpheref& v ) const
	{
		return Math::tIntersectionSphereObb<f32>( mBox, v ).fIntersects( );
	}

	void t3dGridEntity::fOnMoved( b32 recomputeParentRelative )
	{
		tSpatialEntity::fOnMoved( recomputeParentRelative );
		mBox = fComputeBox( );
	}

	Math::tObbf t3dGridEntity::fParentRelativeBox( ) const
	{
		return Math::tObbf( Math::tAabbf( -1.f, +1.f ), fParentRelative( ) );
	}

	Math::tVec3f t3dGridEntity::fClosestPoint( const Math::tVec3f& point ) const
	{
		return mBox.fClosestPoint( point );
	}

	Math::tObbf t3dGridEntity::fComputeBox( ) const
	{
		return Math::tObbf( Math::tAabbf( -1.f, +1.f ), fObjectToWorld( ) );
	}

}
