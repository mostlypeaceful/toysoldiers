#include "BasePch.hpp"
#include "tCharacterPhysics.hpp"
#include "tGroundRayCastCallback.hpp"
#include "tSceneGraphCollectTris.hpp"
#include "tShapeEntity.hpp"

#include "Math/tIntersectionSphereObb.hpp"
#include "Math/tIntersectionSphereSphere.hpp"

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
	devvar( f32, Physics_Character_CriticalStepHeight, 0.25f );
	devvar( f32, Physics_Character_CriticalStepHeightFalling, 1.75f );

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

			explicit tHighPriRayCastCallback( tEntity& owner, tVec3f& currentPos, f32 stepHeight, tEntityTagMask groundMask ) 
				: mIgnoreEntity( &owner )
				, mGroundMask( groundMask )
				, mHitIndex( ~0 )
			{
				f32 extraHeight = 0.1f;
				mRay.mExtent = tVec3f::cYAxis * -Physics_Character_CriticalRayLen;
				mRay.mOrigin = currentPos - mRay.mExtent - tVec3f::cYAxis * extraHeight;
				mStepHeightT = (Physics_Character_CriticalRayLen + extraHeight - stepHeight) / Physics_Character_CriticalRayLen;
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
				if( i->fQuickRejectByFlags( ) )
					return;
				tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i );
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
		, mGravity( -10.5f )
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
		const f32 extraFactor = 0.125f; //extra distance to add to probe length
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
					tHighPriRayCastCallback rayCastCallback( *logic->fOwnerEntity( ), currentPos, fFalling( ) ? Physics_Character_CriticalStepHeightFalling: Physics_Character_CriticalStepHeight, mGroundMask );
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
						hitPos = highPriRay.fEvaluate( hit.mT );
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
					hitPos = ray.fEvaluate( rayCastCallback.mHit.mT );
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
				tShapeEntity *sE = e->fDynamicCast<tShapeEntity>( );
				if( sE )
				{
					if( sE->fShapeType( ) == tShapeEntityDef::cShapeTypeBox )
					{	
						//do narrow phase to see if we collided

						const tObbf& theirObb = sE->fBox( );
						tIntersectionSphereObbWithContact<f32> intersect( tweenCollisionSphere, theirObb );

						if( intersect.fIntersects( ) )
						{
							if( Physics_Character_RenderCollision )
							{
								logic->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( theirObb, tVec4f( 1,0,0,0.5f ) );
								logic->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( mTransform.fGetTranslation( ), intersect.fContactPt( ), tVec4f( 0,1,0,0.5f ) );
							}

							tVec3f sep = intersect.fSphereNormal( ) * -intersect.fPenetration( ) + ( tweenCollisionSphere.mCenter - collisionSphere.mCenter );
							sep.y = fMax( sep.y, 0.f ); //dont push down or we'll go through the world
							mTransform.fTranslateGlobal( sep );

							++collisionCount;
						}
					}
					else if( sE->fShapeType( ) == tShapeEntityDef::cShapeTypeSphere )
					{	
						//do narrow phase to see if we collided

						const tSpheref& theirSphere = sE->fSphere( );
						tIntersectionSphereSphereWithContact<f32> intersect( tweenCollisionSphere, theirSphere );

						if( intersect.fIntersects( ) )
						{
							if( Physics_Character_RenderCollision )
							{
								logic->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( theirSphere, tVec4f( 1,0,0,0.5f ) );
								logic->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( mTransform.fGetTranslation( ), tweenCollisionSphere.fCenter( ) + intersect.fNormalA( ) * tweenCollisionSphere.fRadius( ), tVec4f( 0,1,0,0.5f ) );
							}

							tVec3f sep = intersect.fNormalA( ) * intersect.fPenetration( ) + ( tweenCollisionSphere.mCenter - collisionSphere.mCenter );
							sep.y = fMax( sep.y, 0.f ); //dont push down or we'll go through the world
							mTransform.fTranslateGlobal( sep );

							++collisionCount;
						}
					}
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

		//This seems to work fine, and less checking, but potentially characters fall out of map
		//mCachedQuery.fProbePoints( ).fPushBack( tVec3f( 0 ) );
	}

	void tCharacterPhysics::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass< tCharacterPhysics, tStandardPhysics, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		vm.fNamespace(_SC("Physics")).Bind(_SC("Character"), classDesc);
	}
}}
