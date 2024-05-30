#include "BasePch.hpp"
#include "tCollisionAgents.hpp"
#include "tHeightFieldMeshEntity.hpp"
#include "tContactPoint.hpp"
#include "tRigidBody.hpp"
#include "tPhysicsWorld.hpp"
#include "Math/tIntersectionObbObb.hpp"
#include "Math/tIntersectionSphereObb.hpp"
#include "Math/tIntersectionSphereSphere.hpp"
#include "Math/tIntersectionObbTriangle.hpp"
#include "Math/tIntersectionSphereTriangle.hpp"
#include "Math/tIntersectionGeneralSAT.hpp"
#include "tGJK.hpp"

using namespace Sig::Math;

namespace Sig { namespace Physics
{
	devvar( bool, Physics_Debug_RenderTris, false );
	devvar( f32, Physics_Collision_TriCacheSize, 1.f );

	namespace tCollisionAgents
	{
		class tBaseAgent : public tCollisionDispatch::tAgent
		{
		protected:
			tPersistentContactManifoldPtr mManifold;

			tCollisionShape& mShapeA;
			tCollisionShape& mShapeB;
			tRigidBody* mManifoldOwner;

		public:
			tBaseAgent( tCollisionShape& a, tCollisionShape& b, b32 unlimitedPts )
				: tAgent( a, b )
				, mShapeA( a )
				, mShapeB( b )
				, mManifoldOwner( NULL )
			{
				tPhysicsBody* bA = a.mOwner;
				tPhysicsBody* bB = b.mOwner;

				// flip these around as necessary
				tRigidBody* rA = NULL;
				tRigidBody* rB = NULL;
				tCollisionShape* sA = NULL;
				tCollisionShape* sB = NULL;
				b32 flipped = false;

				if( bA->fObjectType( ) == cPhysicsObjectTypeFixed )
				{
					if( bB->fObjectType( ) == cPhysicsObjectTypeRigid )
					{
						tRigidBody* body = bB->fStaticCast<tRigidBody>( );

						if( !body->fConstrainedTo( bA ) )
						{
							rA = body;
							sA = &b;
							sB = &a;
							flipped = true;
						}
					}
				}
				else if( bB->fObjectType( ) == cPhysicsObjectTypeFixed )
				{
					if( bA->fObjectType( ) == cPhysicsObjectTypeRigid )
					{
						tRigidBody* body = bA->fStaticCast<tRigidBody>( );

						if( !body->fConstrainedTo( bB ) )
						{
							rA = body;
							sA = &a;
							sB = &b;
						}
					}
				}
				else
				{
					// both rigid bodies
					tRigidBody* rbA = bA->fStaticCast<tRigidBody>( );
					tRigidBody* rbB = bB->fStaticCast<tRigidBody>( );

					if( !rbA->fConstrainedTo( rbB ) )
					{
						rA = rbA;
						rB = rbB;
						sA = &a;
						sB = &b;
						if( rB < rA )
						{
							fSwap( rA, rB );
							fSwap( sA, sB );
							flipped = true;
						}
					}
				}

				// rA is sorted to be the lowest address. We always store the manifold on that object.
				//  if rA is the only rigidbody, then rB will be NULL.
				if( rA )
				{
					u32 key = tCollisionShape::fMakeKey( a, b );
					sigassert( !rA->fFindManifold( key ) );

					mManifold.fReset( NEW tPersistentContactManifold( rA, rB, *sA, *sB, flipped, key ) );
					mManifold->fSetUnlimitedPts( unlimitedPts );
					mManifold->fSetPassive( a.fFlagSet( tCollisionShape::cPassive ) || b.fFlagSet( tCollisionShape::cPassive ) );

					mManifoldOwner = rA;
					mManifoldOwner->fAddPersistentManifold( mManifold.fGetRawPtr( ) );
				}
			}

			~tBaseAgent( )
			{
				if( mManifold )
				{
					if( mManifoldOwner )
						mManifoldOwner->fRemovePersistentManifold( *mManifold );
				}
			}

			void fUpdateSupportMapping( tSupportMapping& sm, tCollisionShape& shape )
			{
				switch( shape.fType( ) )
				{
					case cCollisionShapeOBB:
						{
							tCollisionShapeOBB& s = static_cast<tCollisionShapeOBB&>( shape );
							tObbSupportMapping& m = static_cast<tObbSupportMapping&>( sm );
							m.fUpdate( s.mShape );
						}						
						break;
					case cCollisionShapeSphere:
						{
							tCollisionShapeSphere& s = static_cast<tCollisionShapeSphere&>( shape );
							tPointSphereSupportMapping& m = static_cast<tPointSphereSupportMapping&>( sm );
							m.mCenter = s.mCenter;
							m.mExtraRadius = s.mExtraRadius;
						}
						break;
					case cCollisionShapeCapsule:
						{
							tCollisionShapeCapsule& s = static_cast<tCollisionShapeCapsule&>( shape );
							tPointCapsuleSupportMapping& m = static_cast<tPointCapsuleSupportMapping&>( sm );
							m.fUpdate( s.mShape );
						}
						break;
					case cCollisionShapeCylinder:
						{
							tCollisionShapeCylinder& s = static_cast<tCollisionShapeCylinder&>( shape );
							tCylinderSupportMapping& m = static_cast<tCylinderSupportMapping&>( sm );
							m.fUpdate( s.mShape );
						}
						break;
					case cCollisionShapeHeightField:
					case cCollisionShapeMesh:
						// nothing to do
						// these shouldn't be moving
						break;
					case cCollisionShapeConvexHull:
						{
							tCollisionShapeConvexHull& s = static_cast<tCollisionShapeConvexHull&>( shape );
							tHullSupportMapping& m = static_cast<tHullSupportMapping&>( sm );
							m.mLocalXform = s.mLocalXform;
						}
						break;
					case cCollisionShapeRay:
						// nothing to do

						break;
					default:
						sigassert( "Unsupported mapping!, fill this in." );
				}
			}
		};

		class tGJKAgent : public tBaseAgent
		{
			tGJK mGJK;
			tContactPoint mResult;

		public:
			tGJKAgent( tSupportMapping* a, tSupportMapping* b, tCollisionShape& shapeA, tCollisionShape& shapeB )
				: tBaseAgent( shapeA, shapeB, false )
				, mGJK( a, b )
			{
				mGJK.fReset( );
			}

			virtual void fStepMT( f32 dt )
			{
				if( !mManifold )
					return; //nothing to do :(

				mGJK.fA( )->fSetWorldXform( mShapeA.fWorldXform( ) );
				mGJK.fB( )->fSetWorldXform( mShapeB.fWorldXform( ) );

				mGJK.fResume( );
				mGJK.fCompute( );

				mResult = mGJK.fMakeContactPt( );
			}

			virtual void fStepST( f32 dt )
			{
				if( mManifold )
					mManifold->fAddContact( mResult );
			}

			virtual void fVolatileUpdate( ) 
			{ 
				fUpdateSupportMapping( *mGJK.fA( ), mShapeA );
				fUpdateSupportMapping( *mGJK.fB( ), mShapeB );
			}
		};

		class tGJKRaycastAgent : public tBaseAgent
		{
			tGJKRaycast mGJK;
			tRayf mLocalRay;

		public:
			tGJKRaycastAgent( tCollisionShapeRay& shapeA, tCollisionShape& shapeB, tSupportMapping* shape )
				: tBaseAgent( shapeA, shapeB, false )
				, mGJK( shape, shapeA.mShape )
				, mLocalRay( shapeA.mShape )
			{
				if_devmenu( ++tSupportMapping::gCounts[ cRaySupport ]; )
			}

			~tGJKRaycastAgent( )
			{
				if_devmenu( sigassert( tSupportMapping::gCounts[ cRaySupport ] > 0 ); --tSupportMapping::gCounts[ cRaySupport ]; )				
			}

			virtual void fStepMT( f32 dt )
			{
				if( !mManifold )
					return; //nothing to do :(

				mGJK.fRay( ) = mLocalRay.fTransform( mShapeA.fWorldXform( ) );
				mGJK.fShape( )->fSetWorldXform( mShapeB.fWorldXform( ) );

				mGJK.fCompute( );
			}

			virtual void fStepST( f32 dt )
			{
				if( mManifold && mGJK.mIntersects )
					mManifold->fAddContact( tContactPoint( mGJK.mPoint, mGJK.mNormal, mGJK.mT ) );
			}

			virtual void fVolatileUpdate( ) 
			{ 
				fUpdateSupportMapping( *mGJK.fShape( ), mShapeB );
			}
		};

		class tGJKTriangleBase : public tBaseAgent
		{
		protected:
			tTriangleSupportMapping* mTriMap;
			tSpatialEntity* mTriSource;
			tTrianglef mNullTri;

			// cache triangles for a larger aabb than needed. only once the reall aabb exceeds this do we need to recache
			tGrowableArray<Math::tTrianglef> mTris;
			Math::tAabbf mCachedSize;

			void fUpdateCache( const Math::tAabbf& currentBounds )
			{
				if( !mCachedSize.fContains( currentBounds ) )
				{
					// todo: maybe only inflate in velocity direction.
					mCachedSize = currentBounds.fInflate( Physics_Collision_TriCacheSize );
					mTris.fSetCount( 0 );
					mTriSource->fCollectTris( mCachedSize, mTris );
				}

				if( Physics_Debug_RenderTris )
				{
					for( u32 i = 0; i < mTris.fCount( ); ++i )
						tPhysicsWorld::fDebugGeometry( ).fRenderOnce( mTris[ i ], tVec4f( 1,0,0,1 ) );
				}
			}

		public:
			tGJKTriangleBase( tCollisionShape& shapeA, tCollisionShape& shapeB, tSpatialEntity* sb )
				: tBaseAgent( shapeA, shapeB, false )
				, mTriMap( NEW tTriangleSupportMapping( &mNullTri, shapeB.mExtraRadius ) )
				, mTriSource( sb )
				, mCachedSize( Math::tAabbf::cZeroSized )
			{
				sigassert( mTriSource );
			}
		};


		class tGJKTriangleAgent : public tGJKTriangleBase
		{
			tGJK mGJK;
			tGrowableArray<tContactPoint> mResults;

		public:
			tGJKTriangleAgent( tCollisionShape& shapeA, tCollisionShape& shapeB, tSupportMapping* sa, tSpatialEntity* sb )
				: tGJKTriangleBase( shapeA, shapeB, sb )
				, mGJK( sa, mTriMap )
			{ }

			virtual void fStepMT( f32 dt )
			{
				if( !mManifold )
					return; //nothing to do :(

				mGJK.fA( )->fSetWorldXform( mShapeA.fWorldXform( ) );

				fUpdateCache( mShapeA.fCachedWorldAABB( ) );

				mResults.fSetCount( 0 );
				
				for( u32 i = 0; i < mTris.fCount( ); ++i )
				{
					mTriMap->mTri = &mTris[ i ];
					mGJK.fReset( );
					mGJK.fCompute( );
					
					tVec3f normal = mTris[ i ].fComputeUnitNormal( );
					tContactPoint cp = mGJK.fMakeContactPt( &normal );
					mResults.fPushBack( cp );
				}
			}

			virtual void fStepST( f32 dt )
			{
				for( u32 i = 0; i < mResults.fCount( ); ++i )
					mManifold->fAddContact( mResults[ i ] );
			}

			virtual void fVolatileUpdate( ) 
			{ 
				fUpdateSupportMapping( *mGJK.fA( ), mShapeA );
			}
		};



		class tGJKTriangleRaycastAgent : public tGJKTriangleBase
		{
			tGJKRaycast mGJK;
			tRayf mLocalRay;

			tContactPoint mResult;
			b32 mHit;

		public:
			tGJKTriangleRaycastAgent( tCollisionShapeRay& shapeA, tCollisionShape& shapeB, tSpatialEntity* sb )
				: tGJKTriangleBase( shapeA, shapeB, sb )
				, mLocalRay( shapeA.mShape )
				, mGJK( mTriMap, mLocalRay )
			{ 
				mGJK.mDebugRender = false;
				if_devmenu( ++tSupportMapping::gCounts[ cRaySupport ]; )
			}

			~tGJKTriangleRaycastAgent( )
			{
				if_devmenu( sigassert( tSupportMapping::gCounts[ cRaySupport ] > 0 ); --tSupportMapping::gCounts[ cRaySupport ]; )
			}

			virtual void fStepMT( f32 dt )
			{
				if( !mManifold )
					return; //nothing to do :(

				mGJK.fRay( ) = mLocalRay.fTransform( mShapeA.fWorldXform( ) );

				fUpdateCache( mShapeA.fCachedWorldAABB( ) );

				f32 bestT = cInfinity;
				mHit = false;

				for( u32 i = 0; i < mTris.fCount( ); ++i )
				{
					mTriMap->mTri = &mTris[ i ];
					mGJK.fCompute( );

					if( mGJK.mIntersects && mGJK.mT < bestT )
					{
						bestT = mGJK.mT;
						mHit = true;
						mResult = tContactPoint( mGJK.mPoint, mGJK.mNormal, mGJK.mT );
					}
				}
			}

			virtual void fStepST( f32 dt )
			{
				if( mHit )
					mManifold->fAddContact( mResult );
			}
		};



		tSupportMapping* fOBBSupport( tCollisionShape& shape )
		{
			tCollisionShapeOBB& s = shape.fCast<tCollisionShapeOBB>( );
			return NEW tObbSupportMapping( s.mShape, s.mExtraRadius );
		}

		tSupportMapping* fSphereSupport( tCollisionShape& shape )
		{
			tCollisionShapeSphere& s = shape.fCast<tCollisionShapeSphere>( );
			return NEW tPointSphereSupportMapping( s.mCenter, s.mExtraRadius );
		}

		tSupportMapping* fConvexSupport( tCollisionShape& shape )
		{
			tCollisionShapeConvexHull& s = shape.fCast<tCollisionShapeConvexHull>( );
			return NEW tHullSupportMapping( s.mShape, s.mLocalXform, s.mExtraRadius );
		}

		tSupportMapping* fCylinderSupport( tCollisionShape& shape )
		{
			tCollisionShapeCylinder& s = shape.fCast<tCollisionShapeCylinder>( );
			return NEW tCylinderSupportMapping( s.mShape, s.mExtraRadius );
		}

		tSupportMapping* fCapsuleSupport( tCollisionShape& shape )
		{
			tCollisionShapeCapsule& s = shape.fCast<tCollisionShapeCapsule>( );
			return NEW tPointCapsuleSupportMapping( s.mShape );
		}


		tCollisionDispatch::tAgent* fCapsuleVsCapsule( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKAgent( fCapsuleSupport( shapeA ), fCapsuleSupport( shapeB ), shapeA, shapeB );
		}

		tCollisionDispatch::tAgent* fCapsuleVsCylinder( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKAgent( fCapsuleSupport( shapeA ), fCylinderSupport( shapeB ), shapeA, shapeB );
		}


		tCollisionDispatch::tAgent* fObbVsObb( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKAgent( fOBBSupport( shapeA ), fOBBSupport( shapeB ), shapeA, shapeB );
		}

		tCollisionDispatch::tAgent* fObbVsCylinder( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKAgent( fOBBSupport( shapeA ), fCylinderSupport( shapeB ), shapeA, shapeB );
		}

		tCollisionDispatch::tAgent* fObbVsCapsule( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKAgent( fOBBSupport( shapeA ), fCapsuleSupport( shapeB ), shapeA, shapeB );
		}


		tCollisionDispatch::tAgent* fSphereVsObb( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKAgent( fSphereSupport( shapeA ), fOBBSupport( shapeB ), shapeA, shapeB );
		}

		tCollisionDispatch::tAgent* fSphereVsSphere( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKAgent( fSphereSupport( shapeA ), fSphereSupport( shapeB ), shapeA, shapeB );
		}

		tCollisionDispatch::tAgent* fSphereVsCylinder( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKAgent( fSphereSupport( shapeA ), fCylinderSupport( shapeB ), shapeA, shapeB );
		}

		tCollisionDispatch::tAgent* fSphereVsCapsule( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKAgent( fSphereSupport( shapeA ), fCapsuleSupport( shapeB ), shapeA, shapeB );
		}



		tCollisionDispatch::tAgent* fConvexVsObb( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKAgent( fConvexSupport( shapeA ), fOBBSupport( shapeB ), shapeA, shapeB );
		}

		tCollisionDispatch::tAgent* fConvexVsSphere( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKAgent( fConvexSupport( shapeA ), fSphereSupport( shapeB ), shapeA, shapeB );
		}

		tCollisionDispatch::tAgent* fConvexVsConvex( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKAgent( fConvexSupport( shapeA ), fConvexSupport( shapeB ), shapeA, shapeB );
		}

		tCollisionDispatch::tAgent* fConvexVsCylinder( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKAgent( fConvexSupport( shapeA ), fCylinderSupport( shapeB ), shapeA, shapeB );
		}

		tCollisionDispatch::tAgent* fConvexVsCapsule( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKAgent( fConvexSupport( shapeA ), fCapsuleSupport( shapeB ), shapeA, shapeB );
		}



		tCollisionDispatch::tAgent* fObbVsHeightField( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKTriangleAgent( shapeA, shapeB, fOBBSupport( shapeA ), shapeB.fCast<tCollisionShapeHeightfield>( ).mHeightField->fStaticCast<tSpatialEntity>( ) );
		}

		tCollisionDispatch::tAgent* fSphereVsHeightField( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKTriangleAgent( shapeA, shapeB, fSphereSupport( shapeA ), shapeB.fCast<tCollisionShapeHeightfield>( ).mHeightField->fStaticCast<tSpatialEntity>( ) );
		}

		tCollisionDispatch::tAgent* fConvexVsHeightField( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKTriangleAgent( shapeA, shapeB, fConvexSupport( shapeA ), shapeB.fCast<tCollisionShapeHeightfield>( ).mHeightField->fStaticCast<tSpatialEntity>( ) );
		}

		tCollisionDispatch::tAgent* fCapsuleVsHeightField( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKTriangleAgent( shapeA, shapeB, fCapsuleSupport( shapeA ), shapeB.fCast<tCollisionShapeHeightfield>( ).mHeightField->fStaticCast<tSpatialEntity>( ) );
		}

		tCollisionDispatch::tAgent* fCylinderVsHeightField( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKTriangleAgent( shapeA, shapeB, fCylinderSupport( shapeA ), shapeB.fCast<tCollisionShapeHeightfield>( ).mHeightField->fStaticCast<tSpatialEntity>( ) );
		}



		tCollisionDispatch::tAgent* fObbVsMesh( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKTriangleAgent( shapeA, shapeB, fOBBSupport( shapeA ), shapeB.fCast<tCollisionShapeMesh>( ).mMesh->fStaticCast<tSpatialEntity>( ) );
		}

		tCollisionDispatch::tAgent* fSphereVsMesh( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKTriangleAgent( shapeA, shapeB, fSphereSupport( shapeA ), shapeB.fCast<tCollisionShapeMesh>( ).mMesh->fStaticCast<tSpatialEntity>( ) );
		}

		tCollisionDispatch::tAgent* fConvexVsMesh( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKTriangleAgent( shapeA, shapeB, fConvexSupport( shapeA ), shapeB.fCast<tCollisionShapeMesh>( ).mMesh->fStaticCast<tSpatialEntity>( ) );
		}

		tCollisionDispatch::tAgent* fCapsuleVsMesh( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKTriangleAgent( shapeA, shapeB, fCapsuleSupport( shapeA ), shapeB.fCast<tCollisionShapeMesh>( ).mMesh->fStaticCast<tSpatialEntity>( ) );
		}

		tCollisionDispatch::tAgent* fCylinderVsMesh( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKTriangleAgent( shapeA, shapeB, fCylinderSupport( shapeA ), shapeB.fCast<tCollisionShapeMesh>( ).mMesh->fStaticCast<tSpatialEntity>( ) );
		}



		tCollisionDispatch::tAgent* fRaycastVsRaycast( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			// nothing to do.
			return NEW tCollisionDispatch::tAgent( shapeA, shapeB );
		}

		tCollisionDispatch::tAgent* fRaycastVsObb( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKRaycastAgent( shapeA.fCast<tCollisionShapeRay>( ), shapeB, fOBBSupport( shapeB ) );
		}

		tCollisionDispatch::tAgent* fRaycastVsSphere( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKRaycastAgent( shapeA.fCast<tCollisionShapeRay>( ), shapeB, fSphereSupport( shapeB ) );
		}

		tCollisionDispatch::tAgent* fRaycastVsCylinder( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKRaycastAgent( shapeA.fCast<tCollisionShapeRay>( ), shapeB, fCylinderSupport( shapeB ) );
		}

		tCollisionDispatch::tAgent* fRaycastVsCapsule( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKRaycastAgent( shapeA.fCast<tCollisionShapeRay>( ), shapeB, fCapsuleSupport( shapeB ) );
		}

		tCollisionDispatch::tAgent* fRaycastVsConvex( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKRaycastAgent( shapeA.fCast<tCollisionShapeRay>( ), shapeB, fConvexSupport( shapeB ) );
		}

		tCollisionDispatch::tAgent* fRaycastVsMesh( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKTriangleRaycastAgent( shapeA.fCast<tCollisionShapeRay>( ), shapeB, shapeB.fCast<tCollisionShapeMesh>( ).mMesh->fStaticCast<tSpatialEntity>( ) );
		}

		tCollisionDispatch::tAgent* fRaycastVsHeightField( tCollisionShape& shapeA, tCollisionShape& shapeB )
		{
			return NEW tGJKTriangleRaycastAgent( shapeA.fCast<tCollisionShapeRay>( ), shapeB, shapeB.fCast<tCollisionShapeHeightfield>( ).mHeightField->fStaticCast<tSpatialEntity>( ) );
		}



		void fRegisterAllColliders( tCollisionDispatch& disp )
		{
			disp.fRegisterCollider( fCapsuleVsCapsule, cCollisionShapeCapsule, cCollisionShapeCapsule );
			disp.fRegisterCollider( fCapsuleVsCylinder, cCollisionShapeCapsule, cCollisionShapeCylinder );

			disp.fRegisterCollider( fObbVsObb, cCollisionShapeOBB, cCollisionShapeOBB );
			disp.fRegisterCollider( fObbVsCylinder, cCollisionShapeOBB, cCollisionShapeCylinder );
			disp.fRegisterCollider( fObbVsCapsule, cCollisionShapeOBB, cCollisionShapeCapsule );

			disp.fRegisterCollider( fSphereVsObb, cCollisionShapeSphere, cCollisionShapeOBB );
			disp.fRegisterCollider( fSphereVsSphere, cCollisionShapeSphere, cCollisionShapeSphere );
			disp.fRegisterCollider( fSphereVsCapsule, cCollisionShapeSphere, cCollisionShapeCapsule );
			disp.fRegisterCollider( fSphereVsCylinder, cCollisionShapeSphere, cCollisionShapeCylinder );

			disp.fRegisterCollider( fConvexVsObb, cCollisionShapeConvexHull, cCollisionShapeOBB );
			disp.fRegisterCollider( fConvexVsSphere, cCollisionShapeConvexHull, cCollisionShapeSphere );
			disp.fRegisterCollider( fConvexVsConvex, cCollisionShapeConvexHull, cCollisionShapeConvexHull );
			disp.fRegisterCollider( fConvexVsCapsule, cCollisionShapeConvexHull, cCollisionShapeCapsule );
			disp.fRegisterCollider( fConvexVsCylinder, cCollisionShapeConvexHull, cCollisionShapeCylinder );

			disp.fRegisterCollider( fObbVsHeightField, cCollisionShapeOBB, cCollisionShapeHeightField );
			disp.fRegisterCollider( fSphereVsHeightField, cCollisionShapeSphere, cCollisionShapeHeightField );
			disp.fRegisterCollider( fConvexVsHeightField, cCollisionShapeConvexHull, cCollisionShapeHeightField );
			disp.fRegisterCollider( fCapsuleVsHeightField, cCollisionShapeCapsule, cCollisionShapeHeightField );
			disp.fRegisterCollider( fCylinderVsHeightField, cCollisionShapeCylinder, cCollisionShapeHeightField );

			disp.fRegisterCollider( fObbVsMesh, cCollisionShapeOBB, cCollisionShapeMesh );
			disp.fRegisterCollider( fSphereVsMesh, cCollisionShapeSphere, cCollisionShapeMesh );
			disp.fRegisterCollider( fConvexVsMesh, cCollisionShapeConvexHull, cCollisionShapeMesh );
			disp.fRegisterCollider( fCapsuleVsMesh, cCollisionShapeCapsule, cCollisionShapeMesh );
			disp.fRegisterCollider( fCylinderVsMesh, cCollisionShapeCylinder, cCollisionShapeMesh );


			disp.fRegisterCollider( fRaycastVsRaycast, cCollisionShapeRay, cCollisionShapeRay );
			disp.fRegisterCollider( fRaycastVsObb, cCollisionShapeRay, cCollisionShapeOBB );
			disp.fRegisterCollider( fRaycastVsSphere, cCollisionShapeRay, cCollisionShapeSphere );
			disp.fRegisterCollider( fRaycastVsCylinder, cCollisionShapeRay, cCollisionShapeCylinder );
			disp.fRegisterCollider( fRaycastVsCapsule, cCollisionShapeRay, cCollisionShapeCapsule );
			disp.fRegisterCollider( fRaycastVsConvex, cCollisionShapeRay, cCollisionShapeConvexHull );
			disp.fRegisterCollider( fRaycastVsHeightField, cCollisionShapeRay, cCollisionShapeHeightField );
			disp.fRegisterCollider( fRaycastVsMesh, cCollisionShapeRay, cCollisionShapeMesh );
		}

	}
	

}}
