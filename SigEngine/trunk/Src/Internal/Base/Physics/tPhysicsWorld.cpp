#include "BasePch.hpp"
#include "tPhysicsWorld.hpp"
#include "tConstraint.hpp"
#include "tContactIsland.hpp"
#include "tFixedBody.hpp"
#include "tSequentialImpulseSolver.hpp"
#include "tSupportMapping.hpp" //for debug counts
#include "tApplication.hpp" //for debug geometry

using namespace Sig::Math;

namespace Sig { namespace Physics
{
	devvar( bool, Physics_Debug_ShowStats, false );
	devvar( bool, Physics_Debug_RenderSOT, false );
	devvar( bool, Physics_Debug_RenderBodies, false );
	devvar( bool, Physics_Debug_RenderStatic, false );
	devvar( bool, Physics_Debug_RenderConstraints, false );
	devvar( bool, Physics_Debug_RenderIslands, false );
	devvar( bool, Physics_Step, true );
	devvar( f32, Physics_Debug_Speed, 1.0f );
	devvar( bool, Physics_Collision_DoMT, true );
	devvar( f32, Physics_Collision_Radius, 0.05f );

	devvar( f32, Physics_Test_TunnelSpeed, 0.0f );


	tPhysicsWorld::tParams::tParams( )
		: mCollisionRadius( Physics_Collision_Radius )
		, mSequentialSolver( true )
	{
	}

	tPhysicsWorld::tParams tPhysicsWorld::sParams;

	tPhysicsWorld::tPhysicsWorld( )
		: mTimeAccumulator( 0.f )
	{ 
		mMainStaticBody.fReset( NEW tFixedBody( ) );
		fAddObject( mMainStaticBody.fGetRawPtr( ) );
	}

	tPhysicsWorld::~tPhysicsWorld( )
	{
		mMainStaticBody->fRemoveFromWorld( );
	}

	void tPhysicsWorld::fAddObject( tPhysicsObject* obj )
	{
		sigassert( obj );
		sigassert( !mObjectLists[ obj->fObjectType( ) ].fFind( obj ) && "Object already in world!" );
		mObjectLists[ obj->fObjectType( ) ].fPushBack( tPhysicsObjectPtr( obj ) );
		obj->fSetWorld( this );
	}

	void tPhysicsWorld::fRemoveObject( tPhysicsObject* obj )
	{
		sigassert( obj );
		sigassert( mObjectLists[ obj->fObjectType( ) ].fFind( obj ) && "Object not in world!" );
		obj->fSetWorld( NULL );
		mObjectLists[ obj->fObjectType( ) ].fFindAndErase( obj );
	}

	Gfx::tDebugGeometryContainer& tPhysicsWorld::fDebugGeometry( ) 
	{
		return tApplication::fInstance( ).fSceneGraph( )->fDebugGeometry( );
	}

	Math::tVec3f tPhysicsWorld::fRandomColor( )
	{
		tRandom& r = tRandom::fSubjectiveRand( );
		f32 min = 0.5f;
		f32 range = 1.f - min;
		return tVec3f( r.fFloatZeroToOne( ), r.fFloatZeroToOne( ), r.fFloatZeroToOne( )) * range + tVec3f( min );
	}

	namespace
	{
		template< typename T >
		void fDrawList( tGrowableArray<tPhysicsObjectPtr>& list, tPhysicsWorld& world )
		{
			for( u32 i = 0; i < list.fCount( ); ++i )
			{
				T* item = list[ i ]->fStaticCast<T>( );
				item->fDebugDraw( world );
			}
		}

		template< typename T >
		void fPreCollideList( tGrowableArray<tPhysicsObjectPtr>& list, f32 dt )
		{
			for( u32 i = 0; i < list.fCount( ); ++i )
			{
				T* item = list[ i ]->fStaticCast<T>( );
				item->fPreCollide( dt );
			}
		}

		template< typename T >
		void fPostCollideList( tGrowableArray<tPhysicsObjectPtr>& list, f32 dt )
		{
			for( u32 i = 0; i < list.fCount( ); ++i )
			{
				T* item = list[ i ]->fStaticCast<T>( );
				item->fPostCollide( dt );
			}
		}

		template< typename T >
		void fStepConstraints( tGrowableArray<tPhysicsObjectPtr>& list, f32 dt )
		{
			for( u32 i = 0; i < list.fCount( ); ++i )
			{
				T* item = list[ i ]->fStaticCast<T>( );
				item->fStepST( dt, 1.f );
			}
		}
	}

	void tPhysicsWorld::fStep( f32 dt )
	{
		profile_pix( "tPhysicsWorld::fStep" );
		profile( cProfilePerfPhysicsTotal );

		sParams.mCollisionRadius = Physics_Collision_Radius;

		//dt *= Physics_Debug_Speed;
		//mTimeAccumulator += dt;

		//u32 stepsTaken = 0;
		const f32 cTimeStep = 1.f / 30.f; //hz

		//do
		{
			//mTimeAccumulator -= cTimeStep;
			//++stepsTaken;
			fSubStep( cTimeStep );
		}
		//while (mTimeAccumulator >= cTimeStep );

		//log_line( 0, "Steps taken: " << stepsTaken );

		if( Physics_Debug_RenderSOT )
			mShapeTree.fRenderAABBs( );

		if( Physics_Debug_RenderBodies )
			fDrawList<tRigidBody>( mObjectLists[ cPhysicsObjectTypeRigid ], *this );

		if( Physics_Debug_RenderStatic )
			fDrawList<tPhysicsBody>( mObjectLists[ cPhysicsObjectTypeFixed ], *this );

		if( Physics_Debug_RenderConstraints )
			fDrawList<tConstraint>( mObjectLists[ cPhysicsObjectTypeConstraint ], *this );

		if( Physics_Debug_RenderIslands )
			fDrawList<tContactIsland>( mObjectLists[ cPhysicsObjectTypeIsland ], *this );
	}

	namespace // Collision stuff
	{
		class tPairMT
		{
		public:
			const tGrowableArray<tSortedOverlapTree::tPair*>& mPairs;
			f32 mDT;

			tPairMT( const tGrowableArray<tSortedOverlapTree::tPair*>& pairs, f32 dt )
				: mPairs( pairs )
				, mDT( dt )
			{ }

			b32 fDoIt( u32 index )
			{
				tSortedOverlapTree::tPair& pair = *mPairs[ index ];
				tCollisionDispatch::tAgent* agent = static_cast<tCollisionDispatch::tAgent*>( pair.mData.fGetRawPtr( ) );
				sigassert( agent );

				agent->fStepMT( mDT );

				return true;
			}

			Threads::tDistributedForLoopCallback fMakeCallback( )
			{
				return make_delegate_memfn(  Threads::tDistributedForLoopCallback, tPairMT, fDoIt );
			}
		};

		void fUpdateAllPairsMT( const tGrowableArray<tSortedOverlapTree::tPair*>& pairs, f32 dt )
		{
			tPairMT mt( pairs, dt );
			u32 pairCnt = pairs.fCount( );

			if( Physics_Collision_DoMT )
			{
				Threads::tDistributedForLoopCallback callback = mt.fMakeCallback( );
				tApplication::fInstance( ).fSceneGraph( )->fDistributeForLoop( callback, pairCnt );
			}
			else
			{
				Threads::tDistributedForLoopCallback callback = mt.fMakeCallback( );
				for( u32 i = 0; i < pairCnt; ++i )
					callback( i );
			}
		}

		void fUpdateAllPairsCreateAgentST( const tGrowableArray<tSortedOverlapTree::tPair*>& pairs, f32 dt )
		{
			for( u32 i = 0; i < pairs.fCount( ); ++i )
			{
				tSortedOverlapTree::tPair& pair = *pairs[ i ];

				if( !pair.mData )
				{
					tCollisionShape* sA = static_cast<tCollisionShape*>( pair.mA->mUserData );
					tCollisionShape* sB = static_cast<tCollisionShape*>( pair.mB->mUserData );
					pair.mData.fReset( tCollisionDispatch::fInstance( ).fCreateAgent( *sA, *sB ) );
				}
				else
				{
					tCollisionDispatch::tAgent* agent = static_cast<tCollisionDispatch::tAgent*>( pair.mData.fGetRawPtr( ) );
					sigassert( agent );
					if( agent->mVolatile )
						agent->fVolatileUpdate( );
				}
			}
		}

		void fUpdateAllPairsST( const tGrowableArray<tSortedOverlapTree::tPair*>& pairs, f32 dt )
		{
			for( u32 i = 0; i < pairs.fCount( ); ++i )
			{
				tSortedOverlapTree::tPair& pair = *pairs[ i ];
				tCollisionDispatch::tAgent* agent = static_cast<tCollisionDispatch::tAgent*>( pair.mData.fGetRawPtr( ) );
				sigassert( agent );
				agent->fStepST( dt );
			}
		}
	}

	void tPhysicsWorld::fSubStep( f32 dt )
	{
		if( Physics_Step )
		{
			{
				fPreCollideList<tContactIsland>( mObjectLists[ cPhysicsObjectTypeIsland ], dt );
				fPreCollideList<tRigidBody>( mObjectLists[ cPhysicsObjectTypeRigid ], dt );
			}
			{
				profile( cProfilePerfPhysicsCollisionPurge );
				mShapeTree.fPurge( );
			}
			{
				profile( cProfilePerfPhysicsCollisionTest );
				fUpdateAllPairsCreateAgentST( mShapeTree.fPairs( ), dt );
				fUpdateAllPairsMT( mShapeTree.fPairs( ), dt );
				fUpdateAllPairsST( mShapeTree.fPairs( ), dt );
			}
			{
				profile( cProfilePerfPhysicsCollisionResolve );
				mSolver.fSolve( mObjectLists[ cPhysicsObjectTypeIsland ], *this, dt );
			}
			{
				profile( cProfilePerfPhysicsContraintStep );
				fStepConstraints<tConstraint>( mObjectLists[ cPhysicsObjectTypeConstraint ], dt );
			}

			// puts things to sleep, does general end of frame maintainance
			fPostCollideList<tRigidBody>( mObjectLists[ cPhysicsObjectTypeRigid ], dt );
			fPostCollideList<tContactIsland>( mObjectLists[ cPhysicsObjectTypeIsland ], dt );
		}
	}

	void tPhysicsWorld::fLogStats( std::wstringstream& statsText )
	{
#ifdef sig_devmenu
		if( Physics_Debug_ShowStats )
		{
			statsText << "Physics: " << std::endl;
			statsText << " Bodies: " << fNumObjects( cPhysicsObjectTypeRigid ) + fNumObjects( cPhysicsObjectTypeFixed ) << " / " << tPhysicsObject::fObjectsInstantiated( cPhysicsObjectTypeRigid ) + tPhysicsObject::fObjectsInstantiated( Physics::cPhysicsObjectTypeFixed ) << std::endl;
			statsText << " SOT: " << fShapeTree( ).fItemsCount( ) << std::endl;
			statsText << " Constraints: " << fNumObjects( cPhysicsObjectTypeConstraint ) << " / " << tPhysicsObject::fObjectsInstantiated( cPhysicsObjectTypeConstraint ) << std::endl;
			statsText << " Islands: " << fNumObjects( cPhysicsObjectTypeIsland ) << " / " << tPhysicsObject::fObjectsInstantiated( cPhysicsObjectTypeIsland ) << std::endl;

			// AABB and real sphere ommited because they arent' used in the game.
			statsText << " Supports: S(" << tSupportMapping::gCounts[ cPointSphereSupport ] << ") C(" << tSupportMapping::gCounts[ cPointCapsuleSupport ] << ") CY(" << tSupportMapping::gCounts[ cCylinderSupport ] << ") B(" << tSupportMapping::gCounts[ cObbSupport ] << ") H(" << tSupportMapping::gCounts[ cHullSupport ] << ") T(" << tSupportMapping::gCounts[ cTriangleSupport ] << ") R(" << tSupportMapping::gCounts[ cRaySupport ] << ")" << std::endl;
		}
#endif
	}

}}

// just for these temporary global funcs
#include "tShapeEntity.hpp"
#include "tHeightFieldMeshEntity.hpp"
#include "tFixedBody.hpp"

#include "tRagDoll.hpp"
#include "tAttachmentEntity.hpp"
#include "Logic/tAnimatable.hpp"

#include "Math/tConvexHull.hpp"
#include "tMeshEntity.hpp"

namespace Sig { namespace Physics
{

	namespace
	{


		class tTestRagDollLogic : public tLogic
			, public Logic::tAnimatable
		{
			define_dynamic_cast( tTestRagDollLogic, tLogic );
		public:
			b32 mFull;
			tPhysicsWorld* mWorld;

			tTestRagDollLogic( )
				: mWorld( NULL )
				, mFull( false )
			{ }

			void fSetup( tPhysicsWorld* world, b32 full )
			{
				tAnimatable::fSetLogic( this );

				mWorld = world;
				mFull = full;
				mDoll.fReset( NEW tRagDoll( ) );
			}

			tRagDoll* fDoll( ) { return mDoll.fGetRawPtr( ); }

			virtual void fOnSpawn( )
			{
				tAnimatable::fOnSpawn( );
				fOnPause( false );

				fSetupDoll( );

				if( mFull )
					fSetupAnimation( );

				mWorld->fAddObject( mDoll.fGetRawPtr( ) );
				fSetupTestPins( ); //pin the ragdoll to the world if so desired					
			}

			virtual void fOnDelete( )
			{
				tAnimatable::fOnDelete( );
				mDoll->fRemoveFromWorld( );
			}

			void fSetupAnimation( )
			{
				tAnimatable::fExecuteMotionState( "Idle", Sqrat::Object( ) );
			}

			void fSetupDoll( )
			{
				if( mFull )
				{
					sigassert( fAnimatedSkeleton( ) );

					tRagDollBuilderFromRig builder;
					builder.fCollectBodies( *fOwnerEntity( ), ~0 );
					builder.fConfigure( fOwnerEntity( )->fObjectToWorld( ), *fAnimatedSkeleton( ) );
					mDoll->fSetup( builder );

				}
				else
				{
					tRagDollBuilderBasic builder;

					//testing, find some decent stuff to make a ragdoll
					tGrowableArray<tEntityPtr> joints;
					tGrowableArray<tRigidBody*> bodies;

					for( u32 i = 0; i < fOwnerEntity( )->fChildCount( ); ++i )
					{
						const tEntityPtr& e = fOwnerEntity( )->fChild( i );

						tAttachmentEntity* ae = e->fDynamicCast<tAttachmentEntity>( );
						if( ae )
							joints.fPushBack( e );
						else if( e->fLogic( ) )
						{
							tRigidBody* rb = e->fLogic( )->fQueryPhysicalDerived<tRigidBody>( );
							if( rb )
							{
								mWorld->fRemoveObject( rb );
								bodies.fPushBack( rb );
							}
						}
					}

					for( u32 i = 0; i < joints.fCount( ); ++i )
						fFindClosestBodies( builder, bodies, joints[ i ] );

					mDoll->fSetup( builder );
				}

				mDoll->fSetFriction( 0.4f );
			}

			void fSetupTestPins( )
			{
				for( u32 i = 0; i < fOwnerEntity( )->fChildCount( ); ++i )
				{
					const tEntityPtr& e = fOwnerEntity( )->fChild( i );

					tAttachmentEntity* ae = e->fDynamicCast<tAttachmentEntity>( );
					if( ae )
					{
						const Physics::tRagDollBody* bone = mDoll->fData( ).mBodies.fFind( ae->fName( ) );
						if( bone )
						{
							tMat3f aRelPt = bone->mBody->fTransform( ).fInverse( ) * ae->fObjectToWorld( );
							mDoll->fInsertWorldConstraint( Physics::tConstraintPtr( NEW tPinConstraint( bone->mBody.fGetRawPtr( ), NULL, aRelPt ) ) );
						}
					}
				}
			}

			void fFindClosestBodies( tRagDollBuilderBasic& builder, const tGrowableArray<tRigidBody*>& bodies, tEntityPtr& joint )
			{
				f32 closest1 = cInfinity;
				f32 closest2 = cInfinity;
				u32 closest1id = ~0;
				u32 closest2id = ~0;

				const tVec3f jointX = joint->fObjectToWorld( ).fGetTranslation( );

				for( u32 i = 0; i < bodies.fCount( ); ++i )
				{
					f32 dist = (bodies[ i ]->fTransform( ).fGetTranslation( ) - jointX).fLength( );
					if( dist <= closest1  )
					{
						closest1 = dist;
						closest1id = i;
					}
				}
				for( u32 i = 0; i < bodies.fCount( ); ++i )
				{
					f32 dist = (bodies[ i ]->fTransform( ).fGetTranslation( ) - jointX).fLength( );
					if( dist <= closest2 && i != closest1id  )
					{
						closest2 = dist;
						closest2id = i;
					}
				}

				if( closest1id != ~0 && closest2id != ~0 )
					builder.fAddJoint( joint->fObjectToWorld( ), bodies[ closest1id ], bodies[ closest2id ] );
			}

			virtual void fOnPause( b32 paused )
			{
				if( paused )
				{
					fRunListRemove( cRunListAnimateMT );
					fRunListRemove( cRunListMoveST );
				}
				else
				{
					fRunListInsert( cRunListAnimateMT );
					fRunListInsert( cRunListMoveST );
				}
			}

			tRagDollPtr mDoll;

		protected:
			virtual Logic::tAnimatable* fQueryAnimatable( ) { return this; }

			virtual void fMoveST( f32 dt )
			{
				Logic::tAnimatable::fMoveST( dt );

				fOwnerEntity( )->fMoveTo( mDoll->fExpectedEntityXform( ) );
			}

			void fAnimateMT( f32 dt )
			{
				Logic::tAnimatable::fAnimateMT( dt );
			}
		};

		class tTestLogic : public tLogic
		{
			define_dynamic_cast( tTestLogic, tLogic );
		public:
			virtual Logic::tPhysical* fQueryPhysical( ) 
			{ 
				return mBody.fGetRawPtr( ); 
			}

			virtual void fOnSpawn( )
			{
				fOnPause( false );
			}

			virtual void fOnDelete( )
			{
				mBody->fRemoveFromWorld( );
			}

			virtual void fOnPause( b32 paused )
			{
				if( paused )
					fRunListRemove( cRunListMoveST );
				else
					fRunListInsert( cRunListMoveST );
			}

			tPhysicsBodyPtr mBody;
			
		protected:
			virtual void fMoveST( f32 dt )
			{
				if( mBody->fObjectType( ) == Physics::cPhysicsObjectTypeRigid )
					fOwnerEntity( )->fMoveTo( mBody->fTransform( ) );
			}
		};

		void fAttachLogic( tPhysicsBody* body, tEntity* parent )
		{
			tTestLogic *dl = NEW tTestLogic( );
			dl->mBody.fReset( body );
			parent->fAcquireLogic( tLogicPtr( dl ) );
		}

		struct tShapeAdder
		{
			mutable tPhysicsBody* mBody;
			tEntity* mParent;

			tShapeAdder( tPhysicsBody* body, tEntity* parent ) 
				: mBody( body ), mParent( parent ) 
			{ }

			b32 operator( )( const tEntity& e ) const
			{
				tShapeEntity* se = e.fDynamicCast<tShapeEntity>( );
				if( se )
					mBody->fAddShape( tCollisionShape::fFromShapeEntity( se, mParent->fWorldToObject( ) ) );
				else
				{
					tHeightFieldMeshEntity* he = e.fDynamicCast<tHeightFieldMeshEntity>( );
					if( he )
						mBody->fAddShape( NEW tCollisionShapeHeightfield( *he ) );
					else
					{
						//tMeshEntity* me = e.fDynamicCast< tMeshEntity >( );
						//if( me && (me->fEntityDef( ).mStateIndex == 0 || me->fEntityDef( ).mStateIndex == -1) && me->fEntityDef( ).mStateType != tMeshEntityDef::cStateTypeTransition )
						//{
						//	//tAabbf b = tMeshEntity::fCombinedObjectSpaceBox( *me );
						//	//tMat3f xform = mParent->fWorldToObject( ) * me->fObjectToWorld( );
						//	//mBody->fAddShape( NEW tCollisionShapeOBB( tObbf( b, xform ) ) );

						//	tMat3f invXform = mParent->fWorldToObject( ) * me->fObjectToWorld( );

						//	tConvexHull hull;
						//	hull.fConstruct( me, invXform );
						//	mBody->fAddShape( NEW tCollisionShapeConvexHull( hull ) );
						//}
					}
				}

				return false;
			}
		};

		void fAddAllShapes( tPhysicsBody* body, tEntity* parent )
		{
			tShapeAdder adder( body, parent );
			adder( *parent );
			parent->fForEachDescendent( adder );
		}

		void fCreateDymamicFromShapes( tPhysicsWorld* world, tEntity* entity )
		{
			tRigidBody* body = NEW tRigidBody( );

			fAddAllShapes( body, entity );

			tMat3f xform = entity->fObjectToWorld( );
			xform.fNormalizeBasis( );

			body->fSetTransform( xform );
			body->fSetMassPropertiesFromShape( 1.f, 1.0f );

			world->fAddObject( body );

			// much better to let the system come to rest naturally.
			//body->fSleepNow( );

			fAttachLogic( body, entity );


			// tunneling test
			body->fSetVelocity( tVec3f( 0, -Physics_Test_TunnelSpeed, 0 ) );
		}

		void fCreateStaticFromObjectBounds( tPhysicsWorld* world, tEntity* entity )
		{
			tFixedBody* body = NEW tFixedBody( );
			//fAddAllShapes( body, entity );

			tAabbf aabb = entity->fCombinedObjectSpaceBox( );
			body->fAddShape( NEW tCollisionShapeOBB( tObbf( aabb, entity->fObjectToWorld( ) ) ) );

			world->fAddObject( body );

			fAttachLogic( body, entity );
		}

		void fCreateStaticFromShapes( tPhysicsWorld* world, tEntity* entity )
		{
			tFixedBody* body = NEW tFixedBody( );

			fAddAllShapes( body, entity );

			world->fAddObject( body );

			fAttachLogic( body, entity );
		}

		void fCreateRagDoll( tPhysicsWorld* world, tEntity* entity )
		{
			tTestRagDollLogic *dl = NEW tTestRagDollLogic( );
			dl->fSetup( world, false );
			entity->fAcquireLogic( tLogicPtr( dl ) );
		}
	}

	void tPhysicsWorld::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class<tPhysicsWorld, Sqrat::NoCopy<tPhysicsWorld>> classDesc( vm.fSq( ) );
			classDesc
				.GlobalFunc(_SC("CreateDymamicFromShapes"),			&fCreateDymamicFromShapes)
				.GlobalFunc(_SC("CreateStaticFromObjectBounds"),	&fCreateStaticFromObjectBounds)
				.GlobalFunc(_SC("CreateStaticFromShapes"),			&fCreateStaticFromShapes)
				.GlobalFunc(_SC("CreateRagDoll"),					&fCreateRagDoll)
				
				;
			vm.fNamespace(_SC("Physics")).Bind(_SC("World"), classDesc ); 
		}

		{
			Sqrat::DerivedClass< tTestRagDollLogic, tLogic, Sqrat::NoCopy<tTestRagDollLogic> > classDesc( vm.fSq( ) );
			classDesc
				.Func(_SC("Setup"), &tTestRagDollLogic::fSetup)
				.Prop(_SC("RagDoll"),	&tTestRagDollLogic::fDoll)
				;
			vm.fNamespace(_SC("Physics")).Bind(_SC("TestRagDollLogic"), classDesc);
		}
	}

} }
