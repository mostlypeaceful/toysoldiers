#include "BasePch.hpp"
#include "tCollisionShapes.hpp"
#include "tCollisionAgents.hpp"
#include "tPhysicsBody.hpp"
#include "tPhysicsWorld.hpp"
#include "tShapeEntity.hpp"
#include "tHeightFieldMeshEntity.hpp"
#include "tMeshEntity.hpp"
#include "Math/tConvexHull.hpp"

using namespace Sig::Math;

namespace Sig { namespace Physics
{
	const tVec4f tCollisionShape::cDebugColor = tVec4f( 1,0,0, 0.5f );
	const f32 tCollisionShape::cMinimumAxisHalfSize = 0.04f; //below extra radius

	namespace
	{
		const f32 cZshift = 0.0001f;
	}

	devvar( bool, Physics_Debug_DrawMovingStaticShapes, false );

	tCollisionShape* tCollisionShape::fFromShapeEntity( tShapeEntity* se, const Math::tMat3f& parentInvXform )
	{
		sigassert( se );

		switch( se->fShapeType( ) )
		{
		case tShapeEntityDef::cShapeTypeBox:
			{
				tBoxEntity* box = se->fStaticCast<tBoxEntity>( );
				return NEW tCollisionShapeOBB( tObbf( box->fBox( ) ).fTransform( parentInvXform ) );
				break;
			}
		case tShapeEntityDef::cShapeTypeSphere:
			{
				tSphereEntity* s = se->fStaticCast<tSphereEntity>( );
				return NEW tCollisionShapeSphere( tSpheref( s->fSphere( ) ).fTransform( parentInvXform ) );
				break;
			}
		case tShapeEntityDef::cShapeTypeConvexHull:
			{
				tConvexHullEntity* s = se->fStaticCast<tConvexHullEntity>( );
				return NEW tCollisionShapeConvexHull( &s->fObjectSpaceHull( ), parentInvXform * s->fObjectToWorld( ) );
				break;
			}
		case tShapeEntityDef::cShapeTypeCylinder:
			{
				tBoxEntity* box = se->fStaticCast<tBoxEntity>( );
				return NEW tCollisionShapeCylinder( tCylinder( se->fObjectToWorld( ) ).fTransform( parentInvXform ) );
				break;
			}
		case tShapeEntityDef::cShapeTypeCapsule:
			{
				tBoxEntity* box = se->fStaticCast<tBoxEntity>( );
				return NEW tCollisionShapeCapsule( tCapsule( se->fObjectToWorld( ) ).fTransform( parentInvXform ) );
				break;
			}
		}

		sigassert( !"Unsupported shape entity type" );
		return NULL;
	}

	void tCollisionShape::fUpdateTransform( tCollisionShape* cs, tShapeEntity* se, const Math::tMat3f& parentInvXform )
	{
		sigassert( se );
		sigassert( cs );

		switch( se->fShapeType( ) )
		{
		case tShapeEntityDef::cShapeTypeBox:
			{
				tBoxEntity* box = se->fStaticCast<tBoxEntity>( );
				tCollisionShapeOBB& shape = cs->fCast<tCollisionShapeOBB>( );

				shape.mShape = tObbf( box->fBox( ) ).fTransform( parentInvXform );
				break;
			}
		case tShapeEntityDef::cShapeTypeSphere:
			{
				tSphereEntity* s = se->fStaticCast<tSphereEntity>( );
				tCollisionShapeSphere& shape = cs->fCast<tCollisionShapeSphere>( );
				shape.mCenter = tSpheref( s->fSphere( ) ).fTransform( parentInvXform ).mCenter;
				break;
			}
		case tShapeEntityDef::cShapeTypeConvexHull:
			{
				tConvexHullEntity* s = se->fStaticCast<tConvexHullEntity>( );
				tCollisionShapeConvexHull& shape = cs->fCast<tCollisionShapeConvexHull>( );
				shape.mLocalXform = parentInvXform * s->fObjectToWorld( );
				break;
			}
		case tShapeEntityDef::cShapeTypeCylinder:
			{
				tBoxEntity* box = se->fStaticCast<tBoxEntity>( );
				tCollisionShapeCylinder& shape = cs->fCast<tCollisionShapeCylinder>( );
				shape.mShape = tCylinder( se->fObjectToWorld( ) ).fTransform( parentInvXform );
				break;
			}
		case tShapeEntityDef::cShapeTypeCapsule:
			{
				tBoxEntity* box = se->fStaticCast<tBoxEntity>( );
				tCollisionShapeCapsule& shape = cs->fCast<tCollisionShapeCapsule>( );
				shape.mShape = tCapsule( se->fObjectToWorld( ) ).fTransform( parentInvXform );
				break;
			}
		}

		cs->fShapeChanged( );

		if( Physics_Debug_DrawMovingStaticShapes && cs->mOwner )
			cs->fDebugDraw( );
	}

	tCollisionShape::tCollisionShape( u8 type, f32 extraRadius )
		: mType( type )
		, mFlags( 0 )
		, mOwner( NULL )
		, mExtraRadius( extraRadius )
		, mExtraBroadphase( 0.f )
		, mIdentityMask( ~0 )
		, mCollisionMask( ~0 )
	{ }

	u32 tCollisionShape::fMakeKey( const tCollisionShape& a, const tCollisionShape& b )
	{
		sigassert( a.mItem );
		sigassert( b.mItem );

		return tSortedOverlapTree::fMakeKey( *a.mItem, *b.mItem );
	}

	u32 tCollisionShape::fComputeFlags( ) const
	{
		u32 flags = 0;

		switch( mOwner->fObjectType( ) )
		{
		case cPhysicsObjectTypeFixed:
			flags |= tSortedOverlapTree::cItemFlagStatic;
			break;
		}

		return flags;
	}

	void tCollisionShape::fAddToTree( tSortedOverlapTree& tree )
	{
		sigassert( !mItem && "Can only be added to one tree!" );
		sigassert( mOwner && "I need an owner!" );
		mItem.fReset( NEW tSortedOverlapTree::tItem( (void*)this, (void*)mOwner ) );
		tree.fAddItem( *mItem, fComputeFlags( ), fWorldAABB( ) );
	}

	void tCollisionShape::fAddToBatch( tSortedOverlapTree::tBatch& batch )
	{
		sigassert( !mItem && "Can only be added to one tree!" );
		sigassert( mOwner && "I need an owner!" );
		mItem.fReset( NEW tSortedOverlapTree::tItem( (void*)this, (void*)mOwner ) );
		batch.fAddItem( *mItem, fComputeFlags( ), fWorldAABB( ) );
	}

	void tCollisionShape::fRemoveFromOwner( )
	{
		if( mOwner )
			mOwner->fRemoveShape( this );
	}

	const Math::tMat3f& tCollisionShape::fWorldXform( ) const
	{
		sigassert( mOwner && "Need an owner!" );
		return mOwner->fTransform( );
	}

	Math::tAabbf tCollisionShape::fLocalAABB( ) 
	{ 
		sigassert( !"Need to implement this!" );
		// should include the core shape extended by the collision radius
		return Math::tAabbf::cZeroSized; 
	}

	Math::tAabbf tCollisionShape::fWorldAABB( ) 
	{ 
		sigassert( !"Need to implement this!" );
		// should include the core shape extended by the collision radius
		return Math::tAabbf::cZeroSized; 
	}





	tCollisionShapeOBB::tCollisionShapeOBB( const Math::tObbf& obb )
		: tCollisionShape( cShapeType, tPhysicsWorld::fParams( ).mCollisionRadius )
		, mShape( obb )
	{
		mShape.mExtents = fMax( tVec3f( cMinimumAxisHalfSize ), mShape.mExtents - tVec3f( mExtraRadius ) );
	}
	
	Math::tAabbf tCollisionShapeOBB::fLocalAABB( ) 
	{ 
		Math::tAabbf aabb( mShape );
		return aabb.fInflate( mExtraRadius );
	}

	Math::tAabbf tCollisionShapeOBB::fWorldAABB( ) 
	{ 
		return fLocalAABB( ).fTransform( fWorldXform( ) );
	}

	void tCollisionShapeOBB::fDebugDraw( )
	{
		tObbf o = mShape.fTransform( fWorldXform( ) );
		o.mExtents += mExtraRadius + cZshift;
		tPhysicsWorld::fDebugGeometry( ).fRenderOnce( o, cDebugColor );
	}




	Math::tAabbf tCollisionShapeSphere::fLocalAABB( ) 
	{ 
		Math::tVec3f& center = mCenter;
		Math::tVec3f radius( mExtraRadius + mExtraBroadphase );
		return Math::tAabbf( center - radius, center + radius );
	}

	Math::tAabbf tCollisionShapeSphere::fWorldAABB( ) 
	{ 
		Math::tVec3f center = fWorldXform( ).fXformPoint( mCenter );
		Math::tVec3f radius( mExtraRadius + mExtraBroadphase );
		return Math::tAabbf( center - radius, center + radius );
	}

	void tCollisionShapeSphere::fDebugDraw( )
	{
		tPhysicsWorld::fDebugGeometry( ).fRenderOnce( tSpheref( fWorldXform( ).fXformPoint( mCenter ), mExtraRadius + cZshift ), cDebugColor );
	}




	tCollisionShapeCapsule::tCollisionShapeCapsule( const Math::tCapsule& capsule )
		: tCollisionShape( cShapeType, capsule.mRadius )
		, mShape( capsule )
	{
	}

	Math::tAabbf tCollisionShapeCapsule::fLocalAABB( ) 
	{ 
		return mShape.fToAABB( ); //shape is not reduced no need to inflate.
	}

	Math::tAabbf tCollisionShapeCapsule::fWorldAABB( ) 
	{ 
		return fLocalAABB( ).fTransform( fWorldXform( ) );
	}

	void tCollisionShapeCapsule::fDebugDraw( )
	{
		tCapsule o = mShape.fTransform( fWorldXform( ) );
		o.mRadius += cZshift;
		tPhysicsWorld::fDebugGeometry( ).fRenderOnce( o, cDebugColor );
	}



	tCollisionShapeCylinder::tCollisionShapeCylinder( const Math::tCylinder& cylinder )
		: tCollisionShape( cShapeType, tPhysicsWorld::fParams( ).mCollisionRadius )
		, mShape( cylinder )
	{
		mShape.mRadius = fMax( cMinimumAxisHalfSize, mShape.mRadius - mExtraRadius );
		mShape.mHalfHeight = fMax( cMinimumAxisHalfSize, mShape.mHalfHeight - mExtraRadius );
		fShapeChanged( );
	}

	void tCollisionShapeCylinder::fShapeChanged( )
	{
		mLocalAABB = mShape.fToAABB( ).fInflate( mExtraRadius );
	}

	void tCollisionShapeCylinder::fDebugDraw( )
	{
		tCylinder o = mShape.fTransform( fWorldXform( ) );
		o.mRadius += mExtraRadius + cZshift;
		o.mHalfHeight += mExtraRadius + cZshift;
		tPhysicsWorld::fDebugGeometry( ).fRenderOnce( o, cDebugColor );
	}




	tCollisionShapeHeightfield::tCollisionShapeHeightfield( const tEntity& heightField )
		: tCollisionShape( cCollisionShapeHeightField, tPhysicsWorld::fParams( ).mCollisionRadius )
		, mHeightField( heightField.fDynamicCast<tHeightFieldMeshEntity>( ) )
	{
		sigassert( mHeightField && "Must pass in a tHeighFieldMeshEntity" );
		fShapeChanged( );
	}

	void tCollisionShapeHeightfield::fShapeChanged( )
	{
		mLocalAABB = mHeightField->fCombinedWorldSpaceBox( );
		mLocalAABB = mLocalAABB.fInflate( mExtraRadius );
	}

	void tCollisionShapeHeightfield::fDebugDraw( )
	{
		// not needed right now :/
	}





	tCollisionShapeMesh::tCollisionShapeMesh( const tEntity& meshEntity )
		: tCollisionShape( cCollisionShapeMesh, tPhysicsWorld::fParams( ).mCollisionRadius )
		, mMesh( meshEntity.fDynamicCast<tMeshEntity>( ) )
	{
		sigassert( mMesh && "Must pass in a tMeshEntity" );
		fShapeChanged( );
	}

	void tCollisionShapeMesh::fShapeChanged( )
	{
		mLocalAABB = mMesh->fCombinedWorldSpaceBox( );
		mLocalAABB = mLocalAABB.fInflate( mExtraRadius );
	}

	void tCollisionShapeMesh::fDebugDraw( )
	{
		// not needed right now :/
	}

	tCollisionShapeConvexHull::tCollisionShapeConvexHull( const Math::tConvexHull* hull, const Math::tMat3f& localXform )
		: tCollisionShape( cShapeType, tPhysicsWorld::fParams( ).mCollisionRadius )
		, mShape( hull )
		, mLocalXform( localXform )
	{
		sigassert( mShape );
		fShapeChanged( );
	}

	void tCollisionShapeConvexHull::fShapeChanged( )
	{
		mLocalAABB = mShape->fToAABB( );
		mLocalAABB = mLocalAABB.fInflate( mExtraRadius );
		mLocalAABB = mLocalAABB.fTransform( mLocalXform );
	}

	void tCollisionShapeConvexHull::fDebugDraw( )
	{
		tPhysicsWorld::fDebugGeometry( ).fRenderOnce( *mShape, fWorldXform( )* mLocalXform, cDebugColor );
	}



	tCollisionShapeRay::tCollisionShapeRay( const Math::tRayf& ray, f32 broadphaseBoundary )
		: tCollisionShape( cShapeType, 0.f )
		, mShape( ray )
	{
		mLocalAABB.fInvalidate( );
		mLocalAABB |= ray.mOrigin;
		mLocalAABB |= ray.mOrigin + ray.mExtent;
		mLocalAABB = mLocalAABB.fInflate( broadphaseBoundary );

		//rays are passive by default
		fSetFlag( cPassive, true );
	}

	void tCollisionShapeRay::fDebugDraw( )
	{
		tRayf o = mShape.fTransform( fWorldXform( ) );
		tPhysicsWorld::fDebugGeometry( ).fRenderOnce( o, cDebugColor );
	}



	tCollisionDispatch::tAgent::tAgent( tCollisionShape& a, tCollisionShape& b )
		: mVolatile( a.fFlagSet( tCollisionShape::cVolatile ) | b.fFlagSet( tCollisionShape::cVolatile ) )
	{
	}



	void tCollisionDispatch::fRegisterCollider( tCreateAgentFuncPtr func, u32 typeA, u32 typeB )
	{
		mColliders[ typeA ][ typeB ] = tCreateAgentFunc( func, false );
		mColliders[ typeB ][ typeA ] = tCreateAgentFunc( func, true );
	}

	void tCollisionDispatch::fRegisterAllColliders( )
	{
		tCollisionAgents::fRegisterAllColliders( *this );
	}

	tCollisionDispatch::tAgent* tCollisionDispatch::fCreateAgent( tCollisionShape& a, tCollisionShape& b )
	{
		tCreateAgentFunc& collider = mColliders[ a.fType( ) ][ b.fType( ) ];			
		log_assert( collider.mFunc, "No collision function for this pair! A: " << (u32)a.fType( ) << " B: " << (u32)b.fType( ) );

		if( a.fCollisionMaskAgainst( b ) )
		{
			// Passed bit masking check.
			if( collider.mFlip )
				return collider.mFunc( b, a );
			else
				return collider.mFunc( a, b );
		}
		else
		{
			// Didnt pass bit checks, create null agent.
			return NEW tAgent( a, b );
		}
	}


}}
