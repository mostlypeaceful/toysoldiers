#include "BasePch.hpp"
#include "tCharacterPhysics.hpp"
#include "tGroundRayCastCallback.hpp"
#include "tSceneGraphCollectTris.hpp"
#include "tShapeEntity.hpp"

#include "Math/tIntersectionSphereObb.hpp"
#include "Math/tIntersectionSphereSphere.hpp"
#include "Math/tIntersectionRayPlane.hpp"

#include "tPhysicsWorld.hpp"
#include "tContactIsland.hpp"

using namespace Sig::Math;

namespace Sig { namespace Physics
{

	devvar( bool, Physics_Character_UseCachedQuery, true );
	devvar( bool, Physics_Character_RenderCollision, false );
	devvar( bool, Physics_Character_RenderRay, false );
	devvar( f32, Physics_Character_CacheBuffer, 0.25f );
	devvar( f32, Physics_Character_CacheRadius, 10.0f );
	devvar( u32, Physics_Character_SkipFrames, 0 );
	devvar( f32, Physics_Character_CriticalRayLen, 40.f );
	devvar( f32, Physics_Character_CriticalStepHeight, 0.5f );
	devvar( f32, Physics_Character_CriticalStepHeightFalling, 1.75f );
	devvar( f32, Physics_Character_ExtrRayPercentage, 0.25f );
	devvar( f32, Physics__Old_Character_Gravity, 10.5f );

	devvar( bool,		Physics_Character_ProjectAndClamp_Resolve,				true );						/// Enable this whole projection passes + clamp pass setup.
	devvar( u32,		Physics_Character_ProjectAndClamp_ProjectPasses,		3 );						/// If we're hitting multiple planes a frame, how many times we'll try to fix the final position via projecting the movement vector on the blocking plane before giving up.
	devvar( u32,		Physics_Character_ProjectAndClamp_ClampPasses,			1 );						/// If we're still intersecting with collision planes after projecting, how many times we'll try to fix the final position via clamping the movement vector to the first blocking plane we hit.  I don't think there's ever a reason for this to be >1
	devvar( bool,		Physics_Character_ProjectAndClamp_Spam,					false );					/// Enable debug spam.
	devvar_clamp( f32,	Physics_Character_ProjectAndClamp_ClampPushValue,		-0.01f, -1.0f, +1.0f, 2 );	/// When we clamp positions, collide at this fudge factor away from the t=0 clamp point so we're not immediately intersecting in the next frame as well.
	devvar_clamp( f32,	Physics_Character_ProjectAndClamp_ProjectPushValue,		 0.00f, -1.0f, +1.0f, 2 );	/// How much to push the projection vector out from the clamping plane when projecting to help avoid false positives when clamping.
	devvar_clamp( f32,	Physics_Character_ProjectAndClamp_TooDeepThreshold,		-5.00f,-100.0f,+100.0f, 1 );	/// At what point have we dug too greedily, too deep.

	devvar( bool,		Physics_Character_ProjectAndClamp_RenderHitPlanes,		false );
	devvar_clamp( f32,	Physics_Character_ProjectAndClamp_RenderHitPlanesProjectDist, 10.0f, 0.0f, 100.0f, 1 );
	devvar_clamp( f32,	Physics_Character_ProjectAndClamp_RenderHitPlanesClampDist, 9.0f, 0.0f, 100.0f, 1 );

	devvar( bool, Perf_Character_TwoTimesAhead, false );

	namespace
	{
		const tStringPtr cCharacterPhysics( "CharacterPhysics" );
		const u32 cMaxProxQueriesPerFrame = 1;


		struct tHighPriRayCastCallback
		{
		public:
			struct tHit
			{
				f32			mT;
				tVec3f		mN;
				tEntity*	mFirstEntity;

				tHit( f32 t = 0.f, const tVec3f& n = tVec3f::cYAxis, tEntity* e = NULL )
					: mT( t )
					, mN( n )
					, mFirstEntity( e )
				{ }

				b32 operator < ( const tHit& right ) const { return mT < right.mT; }
			};

			mutable tGrowableArray<tHit>	mHits;
			tEntity*					mIgnoreEntity;
			const tEntityTagMask		mGroundMask;
			f32							mStepHeightT;
			f32							mStepDownHeightT;
			tRayf						mRay;
			u32							mHitIndex;

			// extra length is a bit more probe down, to sort of glue the player to the ground over littl ebumps
			explicit tHighPriRayCastCallback( tEntity& owner, tVec3f& currentPos, f32 stepHeight, tEntityTagMask groundMask, f32 extraProbe ) 
				: mIgnoreEntity( &owner )
				, mGroundMask( groundMask )
				, mHitIndex( ~0 )
			{
				mRay.mExtent = tVec3f::cYAxis * -Physics_Character_CriticalRayLen;
				mRay.mOrigin = currentPos - mRay.mExtent + tVec3f::cYAxis * extraProbe;
				mStepHeightT = (Physics_Character_CriticalRayLen + extraProbe - stepHeight) / Physics_Character_CriticalRayLen;
				mStepDownHeightT = 1.f;

				// go twice as far! if we know something's below us we wont bother popping up to high stuff
				mRay.mExtent *= 2.f;
				mStepHeightT *= 0.5f;
				mStepDownHeightT *= 0.5f;
			}

			const tRayf& fRay( ) const
			{ 
				return mRay; 
			}

			inline void operator()( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const
			{
				//if( i->fQuickRejectByFlags( ) )
				//	return;
				tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i->fOwner( ) );
				if( !spatial->fHasGameTagsAny( mGroundMask ) )
					return;
				if( spatial == mIgnoreEntity || spatial->fIsAncestorOfMine( *mIgnoreEntity ) )
					return;
				if( i->fQuickRejectByBox( ray ) )
					return;

				Math::tRayCastHit hit;
				spatial->fRayCast( ray, hit );
				if( hit.fHit( ) )
				{
					mHits.fPushBack( tHit( hit.mT, hit.mN, spatial ) );
				}
			}

			void fFinalize( )
			{
				std::sort( mHits.fBegin( ), mHits.fEnd( ) );

				if( mHits.fCount( ) )
				{
					for( u32 i = 0; i < mHits.fCount( ); ++i )
					{
						if( mHits[ i ].mT > mStepDownHeightT )
						{
							// there's a hit below our step down height, we're falling.
							return;
						}

						if( mHits[ i ].mT > mStepHeightT )
						{
							// this hit is the highest one we can step on to
							mHitIndex = i;
							return;
						}
					}

					//no index chosen, no hits below us. use lowest hit (highest T)
					mHitIndex = mHits.fCount( ) - 1;
				}
			}

			u32 fHasHit( ) const { return mHitIndex != ~0; }
			const tHit& fHit( ) const { return mHits[ mHitIndex ]; }

		};
	}

	tCharacterPhysics::tCharacterPhysics( )
		: mJumpTimer( -1.0f )
		, mCollisionMask( 0 )
		, mQueryMask( 0 )
		, mSurfaceNormal( tVec3f::cZeroVector )
		, mCachedQuery( cCharacterPhysics, cMaxProxQueriesPerFrame )
		, mDeprioritizedColTimer( 0.f )
		, mGravity( -Physics__Old_Character_Gravity )
		, mPositionAtStartOfLastFall( tVec3f::cZeroVector )
		, mFallingTimer( 0.f )
		, mSkipFrames( Physics_Character_SkipFrames )
	{
	}

	void tCharacterPhysics::fBasicSetup( u32 groundMask, u32 collisionMask, f32 height )
	{
		fSetGroundMask( groundMask );
		fSetCollisionMask( collisionMask );
		fSetCharacterHeight( height );
	}

	void tCharacterPhysics::fSetCharacterHeight( f32 height )
	{
		const f32 extraFactor = Physics_Character_ExtrRayPercentage; //extra distance to add to probe length
		mExtraLength = -height * extraFactor;

		mStepHeight = 0.75f * height;
		mExtentLength = -mStepHeight;

		fConfigureCachedQuery( );
	}

	void tCharacterPhysics::fOnDelete( )
	{
		mCachedQuery.fOnDelete( );
		mStandingOn.fRelease( );
	}

	void tCharacterPhysics::fPhysicsMT( tLogic* logic, f32 dt, b32 highPri )
	{
		if( !fTestBits( mInternalFlags, cFlagMoveToDirty ) )
			return; // character hasn't been moved

		if( fDisabled( ) )
			return;

		mInternalFlags = fClearBits( mInternalFlags, cFlagMoveToDirty | cFlagJustLanded | cFlagStartedFalling );

		tVec3f hitPos;
		b32 mHitFound = false;
		tVec3f currentPos = mTransform.fGetTranslation( );
		tEntity *standingOn = NULL;

		if( Physics_Character_UseCachedQuery )
		{
			mCachedQuery.fProximity( ).fSetRespectGroupMax( mDeprioritizedColTimer > 0.f );
			sigassert( !mV.fIsNan( ) );
			mCachedQuery.fUpdateCritical( logic, mV );

			mDeprioritizedColTimer -= dt;
		}

		if( fTestBits( mInternalFlags, cFlagWantsJump ) )
		{
			mInternalFlags = fClearBits( mInternalFlags, cFlagWantsJump );
			mV = mJumpVel;
			mJumpVel = tVec3f::cZeroVector;
		}
		else if ( mJumpTimer <= 0.0f )
		{
			//not jumping do the ray cast

			// ray cast from my stomach to just below my feet, looking for ground
			if( !mSkipFrames || fFalling( ) )
			{
				mSkipFrames = Physics_Character_SkipFrames;

				// real result
				f32 length = mExtentLength;
				if( !fTestBits( mInternalFlags, cFlagFalling ) )
					length += mExtraLength;

				const tVec3f stepVector = mStepHeight * tVec3f::cYAxis;
				const tVec3f probeVector = length * tVec3f::cYAxis;

				// ray goes from step height down to probe point
				tRayf ray = tRayf( currentPos + stepVector, probeVector );

				if( Physics_Character_RenderRay )
					logic->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( ray, tVec4f::cOnesVector );

				if( highPri )
				{
					tHighPriRayCastCallback rayCastCallback( *logic->fOwnerEntity( ), currentPos, fFalling( ) ? Physics_Character_CriticalStepHeightFalling: Physics_Character_CriticalStepHeight, mGroundMask, mExtraLength );
					tRayf highPriRay = rayCastCallback.fRay( );

					if( Physics_Character_UseCachedQuery ) 
						mCachedQuery.fProximity( ).fRayCast( highPriRay, rayCastCallback );
					else
						logic->fSceneGraph( )->fRayCastAgainstRenderable( ray, rayCastCallback );

					rayCastCallback.fFinalize( );

					mHitFound = rayCastCallback.fHasHit( );
					if( mHitFound )
					{
						const tHighPriRayCastCallback::tHit& hit = rayCastCallback.fHit( );
						hitPos = highPriRay.fPointAtTime( hit.mT );
						standingOn = hit.mFirstEntity;
						mSurfaceNormal = hit.mN;
					}
				}
				else
				{
					tGroundRayCastCallback rayCastCallback( *logic->fOwnerEntity( ), mGroundMask );
					
					if( Physics_Character_UseCachedQuery ) 
						mCachedQuery.fProximity( ).fRayCast( ray, rayCastCallback );
					else
					{
						logic->fSceneGraph( )->fRayCastAgainstRenderable( ray, rayCastCallback );
						if( tGroundRayCastCallback::cShapesEnabledAsGround )
							logic->fSceneGraph( )->fRayCast( ray, rayCastCallback, tShapeEntity::cSpatialSetIndex );
					}

					mHitFound = rayCastCallback.mHit.fHit( );
					hitPos = ray.fPointAtTime( rayCastCallback.mHit.mT );
					standingOn = rayCastCallback.mFirstEntity;
					mSurfaceNormal = rayCastCallback.mHit.mN;
				}
			}
			else
			{
				--mSkipFrames;

				// "free" result
				mHitFound = true;
				hitPos = currentPos;
				mSurfaceNormal = tVec3f::cYAxis;
			}
		}
		else
		{
			mJumpTimer -= dt;
		}

		if( mHitFound )
		{
			if( fTestBits( mInternalFlags, cFlagFalling ) )
			{
				mInternalFlags = fClearBits( mInternalFlags, cFlagFalling );
				mInternalFlags = fSetBits( mInternalFlags, cFlagJustLanded );
			}

			// hit ground, so stay locked to it
			mTransform.fSetTranslation( hitPos );

			mV = tVec3f::cZeroVector;
			mStandingOn.fReset( standingOn );
		}
		else
		{
			// falling
			if( !fTestBits( mInternalFlags, cFlagFalling ) )
			{
				mPositionAtStartOfLastFall = currentPos;
				mFallingTimer = 0.f;
				mInternalFlags = fSetBits( mInternalFlags, cFlagStartedFalling );
			}
			else
				mFallingTimer += dt;

			mInternalFlags = fSetBits( mInternalFlags, cFlagFalling );

			mV += tVec3f(0,mGravity,0) * dt;
			mTransform.fSetTranslation( currentPos + mV * dt );
			mStandingOn.fRelease( );
			mSurfaceNormal = tVec3f::cZeroVector;
		}
	}

	void tCharacterPhysics::fCollideAndResolve( tLogic* logic, f32 dt, const tSpheref& collisionSphere, const Math::tSpheref& prevCollisionSphere, const tGrowableArray< tEntity* >& offenders )
	{
		if( fDisabled( ) )
			return;

		const f32 distMoved = ( prevCollisionSphere.mCenter - collisionSphere.mCenter ).fLength( );
		const f32 minRadius = fMin( prevCollisionSphere.mRadius, collisionSphere.mRadius );

		sigassert( minRadius > 0.f );
		const u32 numSteps = fMax( 1u, fRoundUp<u32>( distMoved / minRadius ) );

		for( u32 i = 0; i < numSteps; ++i )
		{
			const Math::tSpheref tweenCollisionSphere = prevCollisionSphere.fInterpolate( collisionSphere, ( i + 1.f ) / numSteps );

			u32 collisionCount = 0;
			for( u32 i = 0; i < offenders.fCount( ); ++i )
			{
				tEntity* e = offenders[ i ];

				sigassert( e->fHasGameTagsAny( mCollisionMask ) );
				
				tShapeEntity * sE = e->fDynamicCast<tShapeEntity>( );
				if( !sE ) continue;

				switch( sE->fShapeType( ) )
				{
				case tShapeEntityDef::cShapeTypeBox:
					{
						const tObbf& theirObb = ((tBoxEntity*)sE)->fBox( );
						tIntersectionSphereObbWithContact<f32> intersect( tweenCollisionSphere, theirObb );

						if( intersect.fIntersects( ) )
						{
							if( Physics_Character_RenderCollision )
							{
								logic->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( 
									theirObb, tVec4f( 1,0,0,0.5f ) );
								logic->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( 
									mTransform.fGetTranslation( ), intersect.fContactPt( ), tVec4f( 0,1,0,0.5f ) );
							}

							tVec3f sep = intersect.fSphereNormal( ) * -intersect.fPenetration( ) + 
								( tweenCollisionSphere.mCenter - collisionSphere.mCenter );
							sep.y = fMax( sep.y, 0.f ); // Dont push down or we'll go through the world
							mTransform.fTranslateGlobal( sep );

							++collisionCount;
						}
					} break;
				case tShapeEntityDef::cShapeTypeSphere:
					{
						//do narrow phase to see if we collided
						const tSpheref& theirSphere = ((tSphereEntity*)sE)->fSphere( );
						tIntersectionSphereSphereWithContact<f32> intersect( tweenCollisionSphere, theirSphere );

						if( intersect.fIntersects( ) )
						{
							if( Physics_Character_RenderCollision )
							{
								logic->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( 
									theirSphere, tVec4f( 1,0,0,0.5f ) );
								logic->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( 
									mTransform.fGetTranslation( ), 
									tweenCollisionSphere.fCenter( ) + intersect.mResult.mNormal * -intersect.mResult.mDepth, 
									tVec4f( 0,1,0,0.5f ) );
							}

							tVec3f sep = intersect.mResult.mNormal * -intersect.mResult.mDepth + 
								( tweenCollisionSphere.mCenter - collisionSphere.mCenter );
							sep.y = fMax( sep.y, 0.f ); //dont push down or we'll go through the world
							mTransform.fTranslateGlobal( sep );

							++collisionCount;
						}

					} break;
				}
			}

			if( collisionCount > 0 )
				break;
		}
	}

	void tCharacterPhysics::fCoRenderMT( tLogic* logic, f32 dt )
	{
		if( Physics_Character_UseCachedQuery ) 
		{
			if( !fDisabled( ) )
			{
				mCachedQuery.fProximity( ).fFilter( ).fAddTag( mQueryMask );
				mCachedQuery.fProximity( ).fFilter( ).fAddTag( mGroundMask );
				mCachedQuery.fProximity( ).fFilter( ).fAddTag( mCollisionMask );
				sigassert( !mV.fIsNan( ) );

				if( Perf_Character_TwoTimesAhead )
					dt *= 2.f;
				mCachedQuery.fUpdateIdeal( logic, mV, mV * dt );
			}
		}
	}

	void tCharacterPhysics::fThinkST( tLogic* logic, f32 dt )
	{
		mCachedQuery.fCleanST( );
	}

	void tCharacterPhysics::fConfigureCachedQuery( )
	{
		tDynamicArray<u32> spatialSetIndices;
		Gfx::tRenderableEntity::fAddRenderableSpatialSetIndices( spatialSetIndices );
		spatialSetIndices.fPushBack( tShapeEntity::cSpatialSetIndex );
		mCachedQuery.fProximity( ).fSetSpatialSetIndices( spatialSetIndices );

		// setup shape
		f32 width = 1.0f, length = Physics_Character_CacheRadius;
		f32 offset = length * 0.2f; //area behind the character
		tAabbf bounds( tVec3f( -width, (mExtentLength + mExtraLength) * 2.0f, -offset )
			, tVec3f( width, mStepHeight * 2.0f, -offset + length ) );

		mCachedQuery.fSetBounds( bounds );

		mCachedQuery.fProbePoints( ).fPushBack( tVec3f( 0, mStepHeight, 0 ) );
		mCachedQuery.fProbePoints( ).fPushBack( tVec3f( 0, mStepHeight + mExtentLength + mExtraLength, 0 ) );
	}

	void tCharacterPhysics::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass< tCharacterPhysics, tStandardPhysics, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		vm.fNamespace(_SC("Physics")).Bind(_SC("Character"), classDesc);
	}

	//---------------------------------------------------------------------------------------------------------
	tObstructionRecord::tObstructionRecord( )
	{
		//nothing
	}

	//---------------------------------------------------------------------------------------------------------
	tObstructionRecord::tObstructionRecord( tRigidBody* body ) : mRigidBody( body )
	{
		for( u32 i = 0; i < mRigidBody->fShapes().fCount(); ++i )
			mRigidBodyShapes.fPushBack( mRigidBody->fShapes()[i]->fLocalAABB() );

		mIsStatic = false;
	}

	//---------------------------------------------------------------------------------------------------------
	tObstructionRecord::tObstructionRecord( const Math::tAabbf& shape ) : mStaticShape( shape )
	{
		mIsStatic = true;
	}

	//---------------------------------------------------------------------------------------------------------
	b32 tObstructionRecord::fIsStaticObstructor( ) const
	{
		return mIsStatic;
	}

	//---------------------------------------------------------------------------------------------------------
	b32 tObstructionRecord::fCollidesWithHypotheticalTranslation( tRigidBody& movingBody, const Math::tVec3f& testDirection )
	{
		//test a direction...
		const tGrowableArray< Physics::tCollisionShapePtr > shapes = movingBody.fShapes();
		for( u32 i = 0; i < shapes.fCount(); ++i )
		{
			if( !shapes[ i ]->fOwner( ) )
				continue;

			//get the shape early to compute a good movement direction offset
			Math::tAabbf shape_aabb = shapes[i]->fWorldAABB();
			float offset = 0.25f * Math::fSqrt( Math::fSquare( shape_aabb.fWidth() ) + Math::fSquare( shape_aabb.fDepth() ) );
			shape_aabb = shape_aabb.fTranslate( offset * testDirection );
			shape_aabb = shape_aabb.fInflate( -.25f * shape_aabb.fMinAxisLength() );
			if( fIsStaticObstructor() )
			{
				if( shape_aabb.fIntersects( mStaticShape ) )
					return true;
			}
			else
			{
				if( mRigidBody->fRefCount() > 0 )
				{
					Math::tMat3f rbxform = mRigidBody->fTransform();
					for( u32 j = 0; j < mRigidBodyShapes.fCount(); ++j )
					{
						Math::tAabbf obstructor_aabb = mRigidBodyShapes[j];
						obstructor_aabb.fTransform( rbxform );
						if( shape_aabb.fIntersects( obstructor_aabb ) )
							return true;
					}
				}
			}

		}

		return false;
	}


	// NEW STUFF
	devvar( bool, Physics_Debug_Character_DrawProjectedAnim, false );
	namespace
	{
		const u32 cFallingBuffer = 2;
	}

	tCharacterController::tCharacterController( tRigidBody* body )
		: mBody( body )
		, mDesiredTranslation( tVec3f::cZeroVector )
		, mFalling( false )
		, mFallingLast( false )
		, mFallBuffer( 0 )
		, mDisabled( false )
		, mDisabledFromWorld( NULL )
		, mSurfaceNormal( tVec3f::cYAxis )
		, mSurfaceMaxDepth( 0 )
	{
		sigassert( body );
		mBody->fSetInertiaInv( tVec3f::cZeroVector );
		mBodyA.fReset( body ); //for islands
	}

	tCharacterController* tCharacterController::fCreateBasic( f32 height, f32 width, f32 extraGroundReach, tCollisionShape* shape, b32 raycastAlso )
	{
		tRigidBody* b = NEW tRigidBody( );
		tCharacterController* control =  NEW tCharacterController( b );

		if( !shape )
		{
			f32 radius = width * 0.5f;
			f32 center = radius;
			if( raycastAlso )
				center *= 2.f; //move it up we dont want it dragging on the ground.

			shape = NEW tCollisionShapeSphere( Math::tSpheref( tVec3f( 0, center, 0 ), radius ) );
		}

		b->fAddShape( shape );

		if( raycastAlso )
		{
			f32 rayStart = height * 0.5f;
			f32 rayBroadPhase = 1.f;
			tCollisionShapeRay* r = NEW tCollisionShapeRay( Math::tRayf( tVec3f( 0,rayStart,0 ), tVec3f( 0, -rayStart - extraGroundReach, 0 ) ), rayBroadPhase );
			b->fAddShape( r );
			control->mRay.fReset( r );
		}

		return control;
	}

	void tCharacterController::fSetWorld( tPhysicsWorld* world )
	{
		if( mInWorld )
		{
			tConstraint::fSetWorld( NULL );
			mBody->fRemoveFromWorld( );
			mDisabled = false;
			mDisabledFromWorld = NULL;
		}

		if( world )
		{
			world->fAddObject( mBody.fGetRawPtr( ) );
			tConstraint::fSetWorld( world );
		}
	}

	void tCharacterController::fSetFacing( const Math::tVec3f& facing )
	{
		tMat3f xform;
		xform.fOrientZAxis( facing );
		xform.fSetTranslation( mBody->fTransform( ).fGetTranslation( ) );
		mBody->fSetTransform( xform );
	}

	void tCharacterController::fDisable( b32 disable )
	{
		if( !mDisabled )
		{
			if( disable && mInWorld )
			{
				tPhysicsWorld* world = mInWorld;
				fRemoveFromWorld( );
				mDisabledFromWorld = world;
			}
		}
		else
		{
			if( !disable && mDisabledFromWorld )
			{
				mDisabledFromWorld->fAddObject( this );
				mDisabledFromWorld = NULL;
				
				//fill the falling buffer, so if we're enabled into a falling state, we fall right away.
				mFallBuffer = cFallingBuffer;
			}
		}

		mDisabled = disable;
	}

	b32 tCharacterController::fDisabled( ) const
	{
		return mDisabled;
	}

	tEntity* tCharacterController::fStandingOn( ) const
	{
		if( mStandingOn )
			return mStandingOn->fUserData( ).fGetRawPtr( );
		else
			return NULL;
	}

	void tCharacterController::fJump( const Math::tVec3f& velocity )
	{
		mBody->fSetVelocity( mBody->fVelocity( ) + velocity );
		mFallBuffer = cFallingBuffer; // Signal that we should do ground checks
	}

	void tCharacterController::fOnDelete( )
	{
		fRemoveFromWorld( );
		
		mRay.fRelease( );
		mStandingOn.fRelease( );
		mBody.fRelease( );
	}

	void tCharacterController::fSetTransform( const Math::tMat3f& xform )	
	{ 
		if( mBody )
			mBody->fReset( xform );
	}

	void tCharacterController::fSetDesiredTranslation( const Math::tVec3f& trans ) 
	{ 
		mDesiredTranslation = trans; 
		sigassert( !mDesiredTranslation.fIsNan( ) );

		if( mDesiredTranslation.fXZ( ).fLengthSquared( ) > (cEpsilon*cEpsilon) )
			fIslandData( ).fAwaken( );
	}

	const Math::tMat3f& tCharacterController::fTransform( ) const
	{ 
		return mBody->fTransform( ); 
	}

	void tCharacterController::fDisableRecentObstructionTracking( )
	{
		mRecentObstructions.fResize( 0 );
	}

	void tCharacterController::fEnableRecentObstructionTracking( u32 numObstructionToTrack )
	{
		mRecentObstructions.fResize( numObstructionToTrack );
	}

	devvar_clamp( f32, Physics_Character_FallingSlidingFriction, 0.25f, 0.00f, 1.00f, 2 );
	devvar_clamp( f32, Physics_Character_MinFallingSlide, 1.0f, 0.0f, 10.0f, 1 );

	void tCharacterController::fStepConstraintInternal( f32 dt, f32 percentage )
	{
		mFallingLast = mFalling;
		mFalling = true; //will be set to false during clamp
		mSurfaceMaxDepth = -cInfinity;
		mStandingOn.fRelease( );

		tVec3f originalDesired = mDesiredTranslation;
		fSlideAndClampDesiredTranslationOnManifolds( );
		tVec3f obstruction = mDesiredTranslation - originalDesired;
		mObstructed = ( obstruction.fLengthSquared() > 0.8f * originalDesired.fLengthSquared() );
		

		// helps us get over holes in the ground :(
		if( mFalling )
			++mFallBuffer;
		else
			mFallBuffer = 0;
		mFalling = mFallBuffer >= cFallingBuffer;

		if( !mFalling )
		{
			b32 moving = (mDesiredTranslation.fXZ( ).fLengthSquared( ) > (cEpsilon*cEpsilon) );

			//project remaining translation onto surface.
			// cast a ray in the Y direction. find intersection pt, scale resultant vector.
			const tVec3f centerPt = mBody->fTransform( ).fGetTranslation( );
			const tRayf ray( centerPt + mDesiredTranslation, tVec3f::cYAxis );

			f32 t = (centerPt - ray.mOrigin).fDot( mSurfaceNormal ) / ray.mExtent.fDot( mSurfaceNormal );
			const tVec3f intersectPt = ray.fPointAtTime( t );

			tVec3f dir = intersectPt - centerPt;
			if( dir.fLengthSquared( ) > mDesiredTranslation.fLengthSquared( ) )
			{
				// Only reduce the direction vector, never lengthen it. This avoids physics spazing out / oscillations
				// when our desired translation puts us e.g. into the ground.  If the intersection flattens things out,
				// and if we set the length here unconditionally, we'd end up increasing out translation on the XZ
				// plane, causing overshoots & resulting in oscillations.
				dir.fSetLength( mDesiredTranslation.fLength( ) );
			}

			if( Physics_Debug_Character_DrawProjectedAnim )
			{
				tPhysicsWorld::fDebugGeometry( ).fRenderOnce( centerPt, centerPt + mSurfaceNormal * 2.f, tVec4f( 1,1,0,1 ) );
				tPhysicsWorld::fDebugGeometry( ).fRenderOnce( centerPt, centerPt + dir * 2.f, tVec4f( 1,0,0,1 ) );
			}

			tVec3f newV = dir / dt;

			if( mRay )
			{
				if( mSurfaceMaxDepth != -cInfinity )
					mBody->mPseudoV += tVec3f::cYAxis * mSurfaceMaxDepth / dt;
			}
			else
			{
				tVec3f oldV = mBody->fVelocity( );

				// allows it to fall down as fast as it wants, but dampens upward velocity
				newV.y = fMin( oldV.y, fLerp( oldV.y, newV.y, 0.6f ) );
			}


			mBody->fSetVelocity( newV );
			mBody->fSetFriction( moving ? 0.f : 1.0f );
		}
		else
		{
			Math::tVec3f oldXZvel = mBody->fVelocity( );
			oldXZvel.y = 0;
			if( oldXZvel.fLengthSquared( ) < Math::fSquare<f32>( Physics_Character_MinFallingSlide ) )
			{
				Math::tVec3f newXZvel = oldXZvel;
				newXZvel.fNormalizeSafe( mBody->fTransform( ).fZAxis( ) );
				newXZvel *= Physics_Character_MinFallingSlide;
				mBody->fSetVelocity( mBody->fVelocity( ) - oldXZvel + newXZvel );
			}
			mBody->fSetFriction( Physics_Character_FallingSlidingFriction );
		}
	}

	namespace
	{
		const f32 cTouchingThresh = -0.001f;
		const f32 cStepThreshAngle = fCos( fToRadians( 90.f - 45.f ) ); //dot product measured from 90. 
	}

	void tCharacterController::fSlideAndClampDesiredTranslationOnManifolds( )
	{
		tGrowableArray<tPersistentContactManifold*> manifolds;
		tIslandData::fGetAllManifolds( *mBody, manifolds );

		tGrowableArray<tPlanef> contactPlanes;
		b32 entityCausedSomeObstruction = false;

		for( u32 m = 0; m < manifolds.fCount( ); ++m )
		{
			tPersistentContactManifold& manifold = *manifolds[ m ];
			if( manifold.fPassive( ) && (!mRay || !manifold.fForShape( *mRay )) )
				continue;

			const b32 flipped = manifold.fB( ) == mBody;
			const tPersistentContactManifold::tPtArray& pts = manifold.fContacts( );

			b32 entityCausedSomeObstructionThisManifold = false;

			for( u32 c = 0; c < pts.fCount( ); ++c )
			{
				const tPersistentContactPt& pt = pts[ c ];

				const tVec3f normal = (flipped ? -1 : +1) * pt.mWorldNormal;
				const f32 angle = pt.mWorldNormal.fDot( tVec3f::cYAxis );
				const b32 standingSurface = mRay ? manifold.fPassive( ) : angle > cStepThreshAngle;

				if( standingSurface )
				{
					fHandleSurfaceContact( manifold, pt, normal, angle, flipped );
				}
				else if( angle < cStepThreshAngle )
				{
					const b32 obstructed = fHandleWallContact( pt, normal, contactPlanes );
					entityCausedSomeObstructionThisManifold |= obstructed;
					entityCausedSomeObstruction |= obstructed;
				}
			}

			if( entityCausedSomeObstructionThisManifold && (mRecentObstructions.fCapacity() > 0) )
				fGatherRecentObstruction( flipped, manifold );
		}

		if( Physics_Character_ProjectAndClamp_Resolve && entityCausedSomeObstruction )
			fSlideAndClampDesiredTranslationOnPlanes( contactPlanes );

		// make sure we're not standing on ourselves :)
		sigassert( !mBody->fShapes( ).fFind( mStandingOn ) );
	}

	void tCharacterController::fHandleSurfaceContact
		( const tPersistentContactManifold&		manifold
		, const tPersistentContactPt&			pt
		, const tVec3f&							normal
		, f32									angle
		, b32									flipped )
	{
		if( angle >= 0.f )
		{
			mFalling = false;

			f32 contactDepth = 0.f;

			if( mRay )
			{
				// ray will always be "A" due to it's nature
				sigassert( &manifold.fAShape( ) == mRay.fGetRawPtr( ) );

				tVec3f worldPt = pt.fBWorldPt( manifold.fB( ) );
				contactDepth = worldPt.y - mBody->fTransform( ).fGetTranslation( ).y;
			}
			else if( pt.mDepth > cTouchingThresh )
			{
				contactDepth = pt.mDepth;
			}

			if( contactDepth > mSurfaceMaxDepth )
			{
				mSurfaceMaxDepth = contactDepth;
				mSurfaceNormal = normal;
				mStandingOn.fReset( (flipped && !mRay) ? &manifold.fAShape( ) : &manifold.fBShape( ) );
			}
		}
	}

	b32 tCharacterController::fHandleWallContact
		( const tPersistentContactPt&	pt
		, const tVec3f&					normal_
		, tGrowableArray<tPlanef>&		contactPlanes )
	{
		tVec3f normal = normal_;
		// blocker
		//if( angle < 0.f ) //block everything
		{
			// Forces collision onto the XZ plane?
			normal.y = 0.f;
			normal.fNormalizeSafe( tVec3f::cYAxis );
		}

		// speculative. solid as a rock.
		// I must admit I totally fail to grok this equation, but it appears to be the correct one.
		const f32 normalAmount = mDesiredTranslation.fDot( normal ) - fMin( pt.mDepth, 0.f );

		if( Physics_Character_ProjectAndClamp_Resolve )
		{
			// Gather all planes even if we're not necessarily intersecting just yet.
			Math::tPlanef plane( normal, mDesiredTranslation - normal * normalAmount );
			contactPlanes.fPushBack( plane );
		}

		if( normalAmount >= 0 )
			return false; // no obstruction

		if( !Physics_Character_ProjectAndClamp_Resolve )
		{
			// Resolve immediately instead (has problems with <90 angle corners)
			mDesiredTranslation -= normal * normalAmount;
		}
		return true;
	}

	void tCharacterController::fGatherRecentObstruction
		( b32							flipped
		, tPersistentContactManifold&	manifold )
	{
		if( flipped )
		{
			if( manifold.fA( ) )
				mRecentObstructions.fPushLast( manifold.fA( ) );
			else
			{
				mRecentObstructions.fPushLast( manifold.fAShape( ).fWorldAABB() );
			}
		}
		else
		{
			if( manifold.fB( ) )
				mRecentObstructions.fPushLast( manifold.fB( ) );
			else
			{
				mRecentObstructions.fPushLast( manifold.fBShape().fWorldAABB() );
			}
		}
	}

	void tCharacterController::fSlideAndClampDesiredTranslationOnPlanes
		( const tGrowableArray<Math::tPlanef>& contactPlanes )
	{
		if( Physics_Character_ProjectAndClamp_Spam )
		{
			log_line( 0, "[Physics_Character_ProjectAndClamp] Resolution begin" );
			log_line( 0, "\tBody translation:    " << mBody->fTransform( ).fGetTranslation( ) );
			log_line( 0, "\tDesired translation: " << mDesiredTranslation );
		}

		// Projection phases: each projection can leave the movement vector still within a different
		// plane, including ones that had previously projected the vector back outside of them.
		// Still, this allows for nice sliding so we want to use it if possible.
		b32 slidLastPass = true;
		for( u32 pass = 0; slidLastPass && pass < Physics_Character_ProjectAndClamp_ProjectPasses; ++pass )
			slidLastPass = fSlideDesiredTranslationOnPlanesPass( pass, contactPlanes );

		// Clamp phases: we find the closest intersection and clamp based on that.  In theory, float
		// precision aside, we can never be left inside any of the planes after one of these passes.
		// If they're not moving.
		if( slidLastPass )
			for( u32 pass = 0; pass < Physics_Character_ProjectAndClamp_ClampPasses; ++pass )
				if( !fClampDesiredTranslationOnPlanesPass( pass, contactPlanes ) )
					break;

		if( Physics_Character_ProjectAndClamp_Spam )
		{
			log_line( 0, "\tFinal desired translation: " << mDesiredTranslation );
		}
	}

	namespace
	{
		/// desiredTranslation is desired relative movement (so 0,0,0 is no movement desired)
		/// plane is relative to a potential contact point on the player:  So a plane on 0,0,0 is in contact with the player, a plane at 0,0,1 is +1z away from being in contact with the player.
		/// 	This lets us treat the player as a point at origin, significantly simplifying the math here.
		b32 fShouldCollideWithPlane( const Math::tPlanef& plane, const Math::tVec3f& desiredTranslation )
		{
			if( plane.fSignedDistance( desiredTranslation ) >= 0.0f )
				return false; // end point is on correct side of plane
			if( plane.fSignedDistance( desiredTranslation ) >= plane.fSignedDistance( Math::tVec3f::cZeroVector ) )
				return false; // end point is on the wrong side of plane, but less so than the origin
			if( plane.fSignedDistance( Math::tVec3f::cZeroVector ) <= Physics_Character_ProjectAndClamp_TooDeepThreshold )
				return false; // at this point, we no longer give a damn.

			return true; // we're moving into the plane
		}

		void fDebugHitPlane( const Math::tVec3f translate, const Math::tPlanef& plane, f32 dist, const Math::tVec4f& rgba )
		{
			if( Physics_Character_ProjectAndClamp_RenderHitPlanes )
				tPhysicsWorld::fDebugGeometry( ).fRenderOnce( plane, translate - dist * plane.fGetNormal( ), 10, rgba );
		}
	}

	b32 tCharacterController::fSlideDesiredTranslationOnPlanesPass
		( u32									pass
		, const tGrowableArray<Math::tPlanef>&	contactPlanes )
	{
		b32 projectedThisPass = false;
		for( u32 planeI = 0; planeI < contactPlanes.fCount( ); ++planeI )
		{
			const Math::tPlanef plane = contactPlanes[ planeI ].fTranslatedAlongNormal( Physics_Character_ProjectAndClamp_ProjectPushValue );
			if( !fShouldCollideWithPlane( plane, mDesiredTranslation ) )
				continue; // moving away from this plane

			if( Physics_Character_ProjectAndClamp_Spam )
				log_line( 0, "\tProject pass " << (pass+1) << " of " << Physics_Character_ProjectAndClamp_ProjectPasses
					<< " projecting plane " << (planeI+1) << " of " << contactPlanes.fCount( )
					);

			fDebugHitPlane( fTransform( ).fGetTranslation( ), plane, Physics_Character_ProjectAndClamp_RenderHitPlanesProjectDist, Math::tVec4f( 0, 0, 1, 0.5f ) );
			mDesiredTranslation = plane.fProject( mDesiredTranslation );
			projectedThisPass = true;
		}
		return projectedThisPass;
	}

	b32 tCharacterController::fClampDesiredTranslationOnPlanesPass
		( u32									pass
		, const tGrowableArray<Math::tPlanef>&	contactPlanes )
	{
		b32 clampedThisPass = false;
		for( u32 planeI = 0; planeI < contactPlanes.fCount( ); ++planeI )
		{
			const Math::tPlanef plane = contactPlanes[ planeI ].fTranslatedAlongNormal( Physics_Character_ProjectAndClamp_ClampPushValue );
			if( !fShouldCollideWithPlane( plane, mDesiredTranslation ) )
				continue; // moving away from this plane

			if( Physics_Character_ProjectAndClamp_Spam )
				log_line( 0, "\tClamp pass " << (pass+1) << " of " << Physics_Character_ProjectAndClamp_ProjectPasses
					<< " clamping plane " << (planeI+1) << " of " << contactPlanes.fCount( )
					);

			fDebugHitPlane( fTransform( ).fGetTranslation( ), plane, Physics_Character_ProjectAndClamp_RenderHitPlanesClampDist, Math::tVec4f( 1, 1, 0, 0.5f ) );
			mDesiredTranslation = Math::tVec3f::cZeroVector; // plane.fProject( mDesiredTranslation );
			clampedThisPass = true;
		}

		return clampedThisPass;
	}
}}
