#include "BasePch.hpp"
#include "tSpatialEntity.hpp"
#include "tSceneGraph.hpp"

#include "Math/tIntersectionAabbFrustum.hpp"
#include "Math/tIntersectionAabbAabb.hpp"
#include "Math/tIntersectionAabbObb.hpp"
#include "Math/tIntersectionAabbSphere.hpp"

using namespace Sig::Math;

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
		, mWantsToBeInSpatialTree( true )
	{
		mSpatial.fReset( NEW tEntityBVH::tObject( this ) );

		mWorldSpaceBox = mObjectSpaceBox;
		fToSpatialSetObject( )->mWorldSpaceBox = mObjectSpaceBox;
		fResetLazyUpdateListIndex( );
	}

	tSpatialEntity::tSpatialEntity( const Math::tAabbf& objectSpaceBox, u16 stateMask )
		: tStateableEntity( stateMask )
		, mObjectSpaceBox( objectSpaceBox )
		, mWantsToBeInSpatialTree( true )
	{
		sync_event_v_c( mObjectSpaceBox, tSync::cSCSpatial );
		mSpatial.fReset( NEW tEntityBVH::tObject( this ) );

		mWorldSpaceBox = mObjectSpaceBox;
		fToSpatialSetObject( )->mWorldSpaceBox = mObjectSpaceBox;
		fResetLazyUpdateListIndex( );
	}

	tSpatialEntity::~tSpatialEntity( )
	{
	}

	void tSpatialEntity::fOnSpawn( )
	{
		sigassert( fSceneGraph( ) );

		if( mWantsToBeInSpatialTree )
			fAddToSpatialTree( );

		tEntity::fOnSpawn( );
	}

	void tSpatialEntity::fOnDelete( )
	{
		sigassert( fSceneGraph( ) );

		if( !fToSpatialSetObject( )->mCellKey.fNull( ) )
			fSceneGraph( )->fSpatialRemove( *this );

		tEntity::fOnDelete( );
	}

	void tSpatialEntity::fRemoveFromSpatialTree( )
	{
		mWantsToBeInSpatialTree = false;

		if( !fToSpatialSetObject( )->mCellKey.fNull( ) && fSceneGraph( ) )
			fSceneGraph( )->fSpatialRemove( *this );
	}

	void tSpatialEntity::fAddToSpatialTree( )
	{
		mWantsToBeInSpatialTree = true;
		if( fToSpatialSetObject( )->mCellKey.fNull( ) && fSceneGraph( ) )
		{
			fToSpatialSetObject( )->mWorldSpaceBox = mWorldSpaceBox;
			fSceneGraph( )->fSpatialInsert( *this );
		}
	}

	void tSpatialEntity::fToObjectSpace( Math::tFrustumf& fInObject, const Math::tFrustumf& fInWorld ) const
	{
		const Math::tMat4f worldToObject = Math::tMat4f( fObjectToWorld( ) ).fTranspose( );
		fInWorld.fTransform( fInObject, worldToObject );
		fInObject.fNormalize( );
	}

	void tSpatialEntity::fSetObjectSpaceBox( const Math::tAabbf& objectSpaceBox )
	{
		sigassert( objectSpaceBox.fIsValid( ) );
		mObjectSpaceBox = objectSpaceBox;
		fRecomputeWorldBounds( );
	}

	void tSpatialEntity::fSetObjectSpaceBox( const Math::tAabbf& objectSpaceBox, const Math::tMat3f& objectToWorld )
	{
		sigassert( objectSpaceBox.fIsValid( ) );
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
		return Math::tIntersectionAabbFrustum<f32>( fWorldSpaceBox( ), v ).fIntersects( );
	}


	b32 tSpatialEntity::fIntersects( const Math::tAabbf& v ) const
	{
		return Math::tIntersectionAabbAabb<f32>( fWorldSpaceBox( ), v ).fIntersects( );
	}

	b32 tSpatialEntity::fIntersects( const Math::tObbf& v ) const
	{
		return Math::tIntersectionAabbObb<f32>( fWorldSpaceBox( ), v ).fIntersects( );
	}

	b32 tSpatialEntity::fIntersects( const Math::tSpheref& v ) const
	{
		return Math::tIntersectionAabbSphere<f32>( fWorldSpaceBox( ), v ).fIntersects( );
	}

	void tSpatialEntity::fOnMoved( b32 recomputeParentRelative )
	{
		tEntity::fOnMoved( recomputeParentRelative );
		fRecomputeWorldBounds( );
	}

	void tSpatialEntity::fRecomputeWorldBounds( )
	{
		mWorldSpaceBox = mObjectSpaceBox.fTransform( fObjectToWorld( ) );
		sync_event_v_c( mWorldSpaceBox, tSync::cSCSpatial );

		if( !fToSpatialSetObject( )->mCellKey.fNull( ) )
		{
			sigassert( fSceneGraph( ) );
			fToSpatialSetObject( )->mWorldSpaceBox = mWorldSpaceBox;
			fSceneGraph( )->fSpatialMove( *this );
		}
	}

}
