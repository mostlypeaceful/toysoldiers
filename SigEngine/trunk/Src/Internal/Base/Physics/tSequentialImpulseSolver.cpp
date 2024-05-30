#include "BasePch.hpp"
#include "tSequentialImpulseSolver.hpp"
#include "tPhysicsBody.hpp"
#include "tRigidBody.hpp"
#include "tContactIsland.hpp"
#include "tPhysicsWorld.hpp"
#include "tPersistentContactManifold.hpp"
#include "tApplication.hpp" //for multithreading

using namespace Sig::Math;

namespace Sig { namespace Physics
{

	devvar( u32, Physics_Sequential_Iterations, 4 );
	devvar( bool, Physics_Sequential_DoMT, false ); // constraints are currently not thread safe. collision shapes store entity ptrs and destruction is not safe-ified.
	devvar( bool, Physics_Sequential_Speculate, true );
	//devvar( bool, Physics_Sequential_Do, true );

	struct tTwoVec3f
	{
		tVec3f mA;
		tVec3f mB;
	};

	namespace
	{
		// layed out to be cache friendly
		struct tSupplementalCPData
		{
			tRigidBody*	mA;
			tVec3f		mAPt;
			tVec3f		mWorldOffsetA;
			tVec3f		mWJVecANormal; //how W changes with an impulse in the normal direction
			tTwoVec3f	mWJVecATangent; //how W changes with an impulse in the tangential direction
			f32			mImpulseDenominatorInv;

			tRigidBody* mB;
			tVec3f		mBPt;
			tVec3f		mWorldOffsetB;
			tVec3f		mWJVecBNormal;
			tTwoVec3f	mWJVecBTangent;
			tVec3f		mRelativeV; //this changes during each iteration, unlike the rest of the data
			tTwoVec3f	mTangentDir;
			tVec2f		mTangentImpulseDenominatorInv;

			void fUpdateRelativeV( )
			{
				mRelativeV = mA->fPointVelocityWorldOffset( mWorldOffsetA );
				if( mB )
					mRelativeV -= mB->fPointVelocityWorldOffset( mWorldOffsetB );
			}
		};

		class tSupplementalData
		{
		public:
			tSupplementalData( )
			{
				fRestart( );
			}

			void fRestart( )
			{
				mExpanding = true;
				mCurrentIndex = 0;
				mData.fSetCount( 0 );
			}

			void fStartIteration( )
			{
				mExpanding = false;
				mCurrentIndex = 0;
			}

			tSupplementalCPData& fData( )
			{
				if( mExpanding )
				{
					mData.fPushBack( tSupplementalCPData( ) );
					return mData.fBack( );
				}
				else
				{
					return mData[ mCurrentIndex++ ];
				}
			}

		private:
			b32 mExpanding;
			u32 mCurrentIndex;
			tGrowableArray< tSupplementalCPData > mData;
		};

		void fCollectManifolds( tContactIsland* island, tGrowableArray<tPersistentContactManifold*>& manifolds )
		{
			for( u32 b = 0; b < island->fMembers( ).fCount( ); ++b )
			{
				tPhysicsObject* o = island->fMembers( )[ b ].mPtr;
				if( o->fObjectType( ) == cPhysicsObjectTypeRigid )
				{
					tPhysicsBody* body = static_cast<tPhysicsBody*>( o );
					manifolds.fGrowCapacity( body->fManifolds( ).fCount( ) );
					for( u32 m = 0; m < body->fManifolds( ).fCount( ); ++m )
					{
						tPersistentContactManifold* manifold = body->fManifolds( )[ m ].fGetRawPtr( );
						if( !manifold->fPassive( ) )
							manifolds.fPushBack( manifold );
					}
				}
				else if( o->fObjectType( ) == cPhysicsObjectTypeIsland )
				{
					tContactIsland* island = static_cast<tContactIsland*>( o );
					fCollectManifolds( island, manifolds );
				}
			}
		}

		void fInitializeSupplementalData( tSupplementalCPData& data, tPersistentContactManifold& manifold, tPersistentContactPt& pt )
		{
			pt.fRecompute( *manifold.fA( ), manifold.fB( ) );

			data.mA = manifold.fA( );
			data.mAPt = pt.fAWorldPt( *data.mA );
			data.mWorldOffsetA = data.mAPt - data.mA->fTransform( ).fGetTranslation( );
			data.mWJVecANormal = data.mA->fWorldInertiaInv( ).fXformVector( data.mWorldOffsetA.fCross( pt.mWorldNormal ) );
			data.mImpulseDenominatorInv = data.mA->fComputeImpulseDenominator( data.mWorldOffsetA, pt.mWorldNormal );
			data.mRelativeV = data.mA->fPointVelocityWorldOffset( data.mWorldOffsetA );

			data.mB = manifold.fB( );
			if( data.mB )
			{		
				data.mBPt = pt.fBWorldPt( data.mB );
				data.mWorldOffsetB = data.mBPt - data.mB->fTransform( ).fGetTranslation( );
				data.mWJVecBNormal = data.mB->fWorldInertiaInv( ).fXformVector( data.mWorldOffsetB.fCross( pt.mWorldNormal ) );
				data.mImpulseDenominatorInv += data.mB->fComputeImpulseDenominator( data.mWorldOffsetB, pt.mWorldNormal );
				data.mRelativeV -= data.mB->fPointVelocityWorldOffset( data.mWorldOffsetB );
			}

			// Choose tangent directions. First in direction of relative, or in a random direction.
			tVec3f tanDir = data.mRelativeV - pt.mWorldNormal * pt.mWorldNormal.fDot( data.mRelativeV );
			f32 tanDirLen = tanDir.fLengthSquared( );
			if( !fEqual( tanDirLen, 0.f, (cVectorEqualLengthEpsilon*cVectorEqualLengthEpsilon) ) )
			{
				//tandir has length
				tanDir /= fSqrt( tanDirLen );
			}
			else
			{
				//no tangent dir from relativeV, make one up.
				tVec3f otherVector( pt.mWorldNormal.y, pt.mWorldNormal.z, pt.mWorldNormal.x );
				tanDir = pt.mWorldNormal.fCross( otherVector );
				tanDir.fNormalizeSafe( tVec3f::cZeroVector );
			}

			// Tandir guaranteed to be normalized and perpindicular to normal
			data.mTangentDir.mA = tanDir;
			data.mTangentDir.mB = pt.mWorldNormal.fCross( tanDir );

			data.mWJVecATangent.mA = data.mA->fWorldInertiaInv( ).fXformVector( data.mWorldOffsetA.fCross( data.mTangentDir.mA ) );
			data.mWJVecATangent.mB = data.mA->fWorldInertiaInv( ).fXformVector( data.mWorldOffsetA.fCross( data.mTangentDir.mB ) );

			data.mImpulseDenominatorInv = 1.f / data.mImpulseDenominatorInv;

			data.mTangentImpulseDenominatorInv.x = data.mA->fComputeImpulseDenominator( data.mWorldOffsetA, data.mTangentDir.mA );
			data.mTangentImpulseDenominatorInv.y = data.mA->fComputeImpulseDenominator( data.mWorldOffsetA, data.mTangentDir.mB );

			if( data.mB )
			{
				data.mTangentImpulseDenominatorInv.x += data.mB->fComputeImpulseDenominator( data.mWorldOffsetB, data.mTangentDir.mA );
				data.mTangentImpulseDenominatorInv.y += data.mB->fComputeImpulseDenominator( data.mWorldOffsetB, data.mTangentDir.mB );
				data.mWJVecBTangent.mA = data.mB->fWorldInertiaInv( ).fXformVector( data.mWorldOffsetB.fCross( data.mTangentDir.mA ) );
				data.mWJVecBTangent.mB = data.mB->fWorldInertiaInv( ).fXformVector( data.mWorldOffsetB.fCross( data.mTangentDir.mB ) );
			}

			data.mTangentImpulseDenominatorInv = 1.f / data.mTangentImpulseDenominatorInv;
		}

		void fApplyNormalImpulse( f32 imp, const tVec3f& normal, tSupplementalCPData& data )
		{
			data.mA->mV += imp * data.mA->fMassInv( ) * normal;
			data.mA->mW += imp * data.mWJVecANormal;

			if( data.mB )
			{
				data.mB->mV -= imp * data.mB->fMassInv( ) * normal;
				data.mB->mW -= imp * data.mWJVecBNormal;
			}
		}

		void fApplyTangentImpulse( const tVec2f& imp, tSupplementalCPData& data )
		{
			tVec2f impDMass = imp * data.mA->fMassInv( );

			data.mA->mV += impDMass.x * data.mTangentDir.mA + impDMass.y * data.mTangentDir.mB;
			data.mA->mW += imp.x * data.mWJVecATangent.mA + imp.y * data.mWJVecATangent.mB;

			if( data.mB )
			{
				impDMass = imp * data.mB->fMassInv( );
				data.mB->mV -= impDMass.x * data.mTangentDir.mA + impDMass.y * data.mTangentDir.mB;
				data.mB->mW -= imp.x * data.mWJVecBTangent.mA + imp.y * data.mWJVecBTangent.mB;
			}
		}

		void fWarmStart( tPersistentContactManifold& manifold, tSupplementalData& allData, tPersistentContactPt& pt )
		{
			tSupplementalCPData& data = allData.fData( );
			fInitializeSupplementalData( data, manifold, pt );

			// friction doesnt warm start very well :(
			pt.mLastImpulseTangent = tVec2f( 0.f );
			pt.mLastImpulseTangentWorld = tVec3f( 0.f );

			if( pt.mDepth < 0.f )
			{
				pt.mLastImpulse = 0.f;
				pt.mLastImpulseTangent = tVec2f( 0.f );
				pt.mLastImpulseTangentWorld = tVec3f( 0.f );
			}
			else
			{
				// project old friction into new ref frame
				//pt.mLastImpulseTangent = tVec2f( pt.mLastImpulseTangentWorld.fDot( data.mTangentDir.mA ), pt.mLastImpulseTangentWorld.fDot( data.mTangentDir.mB ) );
				
				fApplyNormalImpulse( pt.mLastImpulse, pt.mWorldNormal, data );
				//fApplyTangentImpulse( pt.mLastImpulseTangent, data );
			}
		}

		void fFinish( tSupplementalData& allData, tPersistentContactPt& pt )
		{
			tSupplementalCPData& data = allData.fData( );

			// done with iterations bake out the data.
			pt.mLastImpulseTangentWorld = data.mTangentDir.mA * pt.mLastImpulseTangent.x + data.mTangentDir.mB + pt.mLastImpulseTangent.y;
		}

		f32 fComputeContactImpulse( tSupplementalCPData& data, tPersistentContactPt& pt, f32 dtInv )
		{
			f32 correction = 0.f;

			if( pt.mDepth >= 0 )
			{
				// Penetrating

				data.fUpdateRelativeV( );
				f32 relDotNormal = data.mRelativeV.fDot( pt.mWorldNormal );

				//so-called "Baumgarte" stabilization -erin catto: http://www.wildbunny.co.uk/blog/2011/03/25/speculative-contacts-an-continuous-collision-engine-approach-part-1/
				correction = -relDotNormal + 0.4f * (pt.mDepth - 0.02f) * dtInv;
			}
			else if( Physics_Sequential_Speculate )
			{
				// Speculative

				data.fUpdateRelativeV( );
				f32 approachVel = data.mRelativeV.fDot( pt.mWorldNormal );

				f32 maxV = pt.mDepth * dtInv;
				f32 delta = maxV - approachVel;
				if( delta > 0 )
					correction = delta;
			}

			f32 impulse = correction * data.mImpulseDenominatorInv;
			return impulse;
		}

		tVec2f fComputeFrictionImpulse( tSupplementalCPData& data, tPersistentContactPt& pt, f32 dtInv )
		{
			tVec2f impulse = 0.f;

			if( pt.mLastImpulse > 0.f )
			{
				data.fUpdateRelativeV( );
				tVec2f tangentVMag( data.mRelativeV.fDot( data.mTangentDir.mA ), data.mRelativeV.fDot( data.mTangentDir.mB ) );
				impulse = tangentVMag * -data.mTangentImpulseDenominatorInv;
			}

			return impulse;
		}

		void fSolveContact( tSupplementalData& allData, tPersistentContactPt& pt, f32 dtInv )
		{
			tSupplementalCPData& data = allData.fData( );
			f32 newImpulse = fComputeContactImpulse( data, pt, dtInv );
			f32 oldImpulse = pt.mLastImpulse;

			// ensure that the total impulse does not put us below zero.
			pt.mLastImpulse = fMax( 0.f, pt.mLastImpulse + newImpulse );

			f32 impulseDelta = pt.mLastImpulse - oldImpulse;
			fApplyNormalImpulse( impulseDelta, pt.mWorldNormal, data );
		}

		void fSolveFriction( tSupplementalData& allData, tPersistentContactPt& pt, f32 dtInv )
		{
			tSupplementalCPData& data = allData.fData( );
			tVec2f newImpulse = fComputeFrictionImpulse( data, pt, dtInv );
			tVec2f oldImpulse = pt.mLastImpulseTangent;

			// clamp it
			f32 impulseMax = pt.mLastImpulse * data.mA->fFriction( ); //todo: * data.mB->fFriction( );
			pt.mLastImpulseTangent = fClamp( pt.mLastImpulseTangent + newImpulse, tVec2f( -impulseMax ), tVec2f( impulseMax ) );

			tVec2f impulseDelta = pt.mLastImpulseTangent - oldImpulse;
			fApplyTangentImpulse( impulseDelta, data );
		}

		void fWarmStartManifold( tPersistentContactManifold& manifold, tSupplementalData& allData )
		{
			for( u32 i = 0; i < manifold.fContacts( ).fCount( ); ++i )
				fWarmStart( manifold, allData, manifold.fContacts( )[ i ] );
		}

		void fFinishManifold( tPersistentContactManifold& manifold, tSupplementalData& allData )
		{
			for( u32 i = 0; i < manifold.fContacts( ).fCount( ); ++i )
				fFinish( allData, manifold.fContacts( )[ i ] );
		}

		void fCollideManifold( tSupplementalData& allData, tPersistentContactManifold& manifold, f32 dtInv )
		{
			for( u32 i = 0; i < manifold.fContacts( ).fCount( ); ++i )
				fSolveContact( allData, manifold.fContacts( )[ i ], dtInv );
		}

		void fFricManifold( tSupplementalData& allData, tPersistentContactManifold& manifold, f32 dtInv )
		{
			for( u32 i = 0; i < manifold.fContacts( ).fCount( ); ++i )
				fSolveFriction( allData, manifold.fContacts( )[ i ], dtInv );
		}
	}

	void tSequentialImpulseSolver::fSolve( tGrowableArray<tPhysicsObjectPtr>& islands, tPhysicsWorld& world, f32 dt )
	{
		mIslandsToSolve = &islands;
		mDtInv = 1.f / dt;

		u32 cnt = islands.fCount( );

		if( Physics_Sequential_DoMT )
		{
			tApplication::fInstance( ).fSceneGraph( )->fDistributeForLoop( fMakeCallback( ), cnt );
		}
		else
		{
			for( u32 i = 0; i < cnt; ++i )
				fProcessIsland( i );
		}
	}

	b32 tSequentialImpulseSolver::fProcessIsland( u32 index )
	{
		tSupplementalData allData;

		tContactIsland* island = static_cast<tContactIsland*>( (*mIslandsToSolve)[ index ].fGetRawPtr( ) );

		if( island->fSleeping( ) )
			return true;

		tGrowableArray< tPersistentContactManifold* > manifolds;
		fCollectManifolds( island, manifolds );

		//if( Physics_Sequential_Do )
		{
			for( u32 m = 0; m < manifolds.fCount( ); ++m )
				fWarmStartManifold( *manifolds[ m ], allData );

			for( u32 it = 0; it < Physics_Sequential_Iterations; ++it )
			{
				allData.fStartIteration( );
				for( u32 m = 0; m < manifolds.fCount( ); ++m )
					fCollideManifold( allData, *manifolds[ m ], mDtInv );

				allData.fStartIteration( );
				for( u32 m = 0; m < manifolds.fCount( ); ++m )
					fFricManifold( allData, *manifolds[ m ], mDtInv );
			}

			allData.fStartIteration( );
			for( u32 m = 0; m < manifolds.fCount( ); ++m )
				fFinishManifold( *manifolds[ m ], allData );
		}

		return true;
	}
	
}}
