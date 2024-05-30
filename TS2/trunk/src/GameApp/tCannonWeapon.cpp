#include "GameAppPch.hpp"
#include "tCannonWeapon.hpp"
#include "tPlayer.hpp"
#include "tUnitLogic.hpp"
#include "tProximity.hpp"
#include "Gfx/tRenderableEntity.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tShellLogic.hpp"
#include "tRtsCursorDisplay.hpp" //only so that the range rings and ui fade out at the same speed

namespace Sig
{
	devvar_clamp( u32, Gameplay_Turrets_NumArcSegmants, 30, 1, 500, 0 );
	devvar( f32, Gameplay_Turrets_TargetingExtraTime, 3.0f );
	devvar( f32, Gameplay_Turrets_TargetingSampleFreq, 10.0f );
	devvar( bool, Gameplay_Turrets_TargetingUseObb, true );
	devvar( bool, Gameplay_Turrets_TargetingUseDistibutedLoop, true );
	devvar( f32, Gameplay_Turrets_TargetingRadiusHeight, 7.5f );

	devvar( f32, Gameplay_Turrets_TargetingPulseScale, 1.0f );
	devvar( f32, Gameplay_Turrets_TargetingPulseFreq, 5.5f );

	devvar( bool, Perf_ArcQuery, false );


	namespace
	{
		struct tCannonTargetingRayCastCallback
		{
			mutable Math::tRayCastHit	mHit;
			mutable tEntity*			mFirstEntity;
			const tEntity*				mIgnoreEntity;
			const tEntityTagMask		mGroundMask;

			explicit tCannonTargetingRayCastCallback( const tEntity& ignore, tEntityTagMask groundMask ) : mFirstEntity( 0 ), mIgnoreEntity( &ignore ), mGroundMask( groundMask ) { }
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
				if( hit.fHit( ) && hit.mT < mHit.mT )
				{
					mHit			= hit;
					mFirstEntity	= spatial;
				}
			}
		};
	}

	tCannonWeapon::tCannonWeapon( const tWeaponDesc& desc, const tWeaponInstData& inst )
		: tWeapon( desc, inst )
		, mRenderUI( false )
		, mRenderFadeOut( false )
		, mTargetingAlpha( 0.f )
		, mLastUIArcTime( 0.f )
		, mBlinkTimer( 0.f )
		, mBlinkIntensity( 0.f )
	{
	}
	void tCannonWeapon::fOnDelete( )
	{
		if( mTargetVisual )
			mTargetVisual->fDelete( );

		mTargetVisual.fRelease( );
		mTargetingLines.fRelease( );

		tWeapon::fOnDelete( );
	}
	void tCannonWeapon::fProcessST( f32 dt )
	{
		if( mRenderUI )
			fUpdateUI( dt );

		if( fUnderUserControl( ) )
		{
			mAITarget.fRelease( );

			if( mDesc.mRaycastAdjustTargets )
			{
				fRayCastAndAdjustTarget( );
				mUseRayCastedTarget = false; //dont actually shoot right at it
			}

			fUpdateTargetRelatedData( );
		}

		tWeapon::fProcessST( dt );

		fDebugRenderShootArc( );
	}
	void tCannonWeapon::fProcessMT( f32 dt )
	{
		tWeapon::fProcessMT( dt );
	}

	void tCannonWeapon::fBeginRendering( tPlayer* player )
	{
		if( !mRendering )
		{
			mRendering = true;
			mRenderUI = mDesc.mWantsWorldSpaceUI;
			mRenderFadeOut = false;
			mLastUIArcTime = 10.f; // seconds

			if( mRenderUI )
			{
				if( !mTargetingLines )
				{
					// lazy constructions so not all wave units have these
					mTargetingLines.fReset( new tWorldSpaceArc( fFirstAnchorPoint( ) ) );
					mTargetVisual.fReset( new tSgFileRefEntity( tGameApp::fInstance( ).fTrajectoryTargetVisual( ) ) );
					mTargetVisual->fSpawn( *tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( ) );
					mTargetVisual->fAddGameTagsRecursive( GameFlags::cFLAG_DUMMY );
					Gfx::tRenderableEntity::fSetInvisible( *mTargetVisual, true );
				}

				mTargetingLines->fSetInvisible( false );
				Gfx::tRenderableEntity::fSetInvisible( *mTargetVisual, false );

				u32 viewportMask = 0;
				viewportMask = fSetBits( viewportMask, 1 << player->fUser( )->fViewport( )->fViewportIndex( ) );
				mTargetingLines->fSetViewportMask( viewportMask );
				Gfx::tRenderableEntity::fSetViewportMask( *mTargetVisual, viewportMask );
			}

			tWeapon::fBeginRendering( player );
		}
	}
	void tCannonWeapon::fEndRendering( )
	{
		mRendering = false;
		mRenderFadeOut = true;
	}
	void tCannonWeapon::fReallyEndRendering( )
	{
		mRenderUI = false;
		mRenderFadeOut = false;

		if( mTargetingLines )
		{
			mTargetingLines->fSetInvisible( true );
			Gfx::tRenderableEntity::fSetInvisible( *mTargetVisual, true );
		}

		tWeapon::fEndRendering( );
	}


	class tArc
	{
	public:
		tArc( const Math::tVec3f& origin = Math::tVec3f::cZeroVector
			, const Math::tVec3f& rate = Math::tVec3f::cZeroVector
			, f32 gravity = -10.0f )
			: mOrigin( origin ), mOriginalRate( rate ), mGravity( gravity )
		{ }

		Math::tVec3f fEvaluate( f32 t ) const
		{
			// assume xz rates are constant
			Math::tVec3f result = mOrigin + mOriginalRate * t;
			
			// y axis is accelerating
			result.y += 0.5f * mGravity * (t*t);

			return result;
		}

		f32 fGetApexT( ) const
		{
			return -mOriginalRate.y / mGravity;
		}

		Math::tAabbf fGetBounds( f32 totalTime ) const 
		{
			Math::tAabbf bounds( mOrigin, mOrigin );
			bounds |= fEvaluate( totalTime );
			bounds |= fEvaluate( fGetApexT( ) );
			bounds.fInflate( 0.1f );
			return bounds;
		}

		Math::tMat3f fGetXform( const Math::tVec3f& xAxis ) const 
		{
			Math::tVec3f z = mOriginalRate;
			z.y = 0;
			z.fNormalizeSafe( Math::tVec3f::cZAxis );

			Math::tMat3f xform;
			xform.fSetTranslation( mOrigin );
			xform.fOrientZWithXAxis( z, xAxis );

			return xform;
		}	

		Math::tAabbf fGetBoundsOriented( f32 totalTime, const Math::tMat3f& xform ) const 
		{
			Math::tMat3f invXform = xform.fInverse( );
			Math::tVec3f origin = invXform.fXformPoint( mOrigin );
			Math::tAabbf bounds( origin, origin );
			bounds |= invXform.fXformPoint( fEvaluate( totalTime ) );
			bounds |= invXform.fXformPoint( fEvaluate( fGetApexT( ) ) );
			bounds.fInflate( 0.1f );

			return bounds;
		}

		Math::tVec3f mOrigin;
		Math::tVec3f mOriginalRate;
		f32 mGravity;
	};

	class tArcCasterMT
	{
	public:
		tArcCasterMT( )
			: mProx( NULL )
			, mIgnoreEnt( NULL )
		{ }

		b32 fDoIt( u32 index )
		{
			Math::tRayf ray;
			ray.mOrigin = mArc.fEvaluate( index * mStepRate );
			ray.mExtent = mArc.fEvaluate( (index + 1) * mStepRate ) - ray.mOrigin;

			tCannonTargetingRayCastCallback rayCastCallback( *mIgnoreEnt, GameFlags::cFLAG_GROUND );
			if( Perf_ArcQuery )
				mProx->fRayCast( ray, rayCastCallback );
			else
				tGameApp::fInstance( ).fSceneGraph( )->fRayCast( ray, rayCastCallback );

			if( rayCastCallback.mHit.fHit( ) )
				mHitTimes[ index ] = rayCastCallback.mHit.mT;
			else
				mHitTimes[ index ] = Math::cInfinity;

			return true;
		}

		void fSetup( const tArc& arc, f32 totalTime, u32 steps, f32 stepRate, const tProximity& prox, tEntity& ignoreEnt )
		{
			mArc = arc;
			mStepRate = stepRate;
			mHitTimes.fSetCount( steps );

			mProx = &prox;
			mIgnoreEnt = &ignoreEnt;
		}

		Threads::tDistributedForLoopCallback fMakeCallback( )
		{
			return make_delegate_memfn(  Threads::tDistributedForLoopCallback, tArcCasterMT, fDoIt );
		}

		const tGrowableArray<f32>& fResults( ) const { return mHitTimes; }

	private:
		const tEntity* mIgnoreEnt;
		const tProximity* mProx;

		tArc mArc;
		f32 mStepRate;
		tGrowableArray<f32> mHitTimes;
	};


	void tCannonWeapon::fUpdateUI( f32 dt )
	{
		// handling fading
		b32 hide = mShellCamPushed || mRenderFadeOut || !mEnable; //hide for shell cam and fade out on exit
		f32 targetAlpha = 1.0f;
		if( hide ) 
			targetAlpha = 0.0f;

		const f32 frameRateCompensation = dt * 30.0f; //fps
		mTargetingAlpha = Math::fLerp(mTargetingAlpha, targetAlpha, frameRateCompensation * tRtsCursorDisplay::fRangeRingFadeLerp( ));	

		if( fEqual( mTargetingAlpha, 0.0f ) )
		{
			if( mRenderFadeOut )
			{
				// fade out complete
				mTargetingAlpha = 0.0f;
				fReallyEndRendering( );
			}

			return;
		}	


		tGrowableArray< Math::tVec3f > points;
		Math::tVec3f launchVelocity = fComputeLaunchVector( );
		Math::tVec3f lastPos;

		//if( !launchVelocity.fIsZero( ) )
		if( !mInst.mTargettingParent )
		{
			f32 totalTime = fEstimateTimeToImpactArc( mPredictedTargetPosition );
			if( totalTime > 0.f )
			{
				// add some more projectile life so targeting most definitely intersects the ground
				totalTime += Gameplay_Turrets_TargetingExtraTime; 
				mLastUIArcTime = totalTime;
			}
			else
			{
				// when switching from user to ai control
				//  while the arc is fading out, before the ai acquires a target
				//  resort to previous arc time so it can update and fade out.
				totalTime = mLastUIArcTime;
				if( totalTime < 0.f ) return;
			}

			
			Math::tVec3f pos( fFirstAnchorPoint( )->fObjectToWorld( ).fGetTranslation( ) );
			Math::tVec3f xAxis = fFirstAnchorPoint( )->fObjectToWorld( ).fXAxis( );
			tArc arc( pos, launchVelocity, mDesc.fShellGravity( ) );

			tProximity proximity;
			
			if( Perf_ArcQuery )
			{
				// cache queries that contain the arc
				tDynamicArray<u32> spatialSetIndices;
				Gfx::tRenderableEntity::fAddRenderableSpatialSetIndices( spatialSetIndices );
				proximity.fSetSpatialSetIndices( spatialSetIndices );
				proximity.fFilter( ).fAddTag( GameFlags::cFLAG_GROUND );

				if( Gameplay_Turrets_TargetingUseObb )
					proximity.fAddObb( arc.fGetBoundsOriented( totalTime, arc.fGetXform( xAxis ) ) );
				else
					proximity.fAddAabb( arc.fGetBounds( totalTime ) );

				{
					profile( cProfilePerfArcQuery );
					if( Gameplay_Turrets_TargetingUseObb )
						proximity.fRefreshMT( 0.f, *tGameApp::fInstance( ).fCurrentLevel( )->fRootEntity( ), &arc.fGetXform( xAxis ) );
					else
						proximity.fRefreshMT( 0.f, *tGameApp::fInstance( ).fCurrentLevel( )->fRootEntity( ), &Math::tMat3f::cIdentity );
				}
			}

			// MT ray cast

			f32 stepRate = 1.0f / Gameplay_Turrets_TargetingSampleFreq;
			u32 steps = u32( totalTime * Gameplay_Turrets_TargetingSampleFreq );

			points.fSetCapacity( steps + 2 ); //2 for good measure :P
			points.fPushBack( pos ); //time = 0

			tArcCasterMT caster;
			caster.fSetup( arc, totalTime, steps, stepRate, proximity, *mInst.mOwnerUnit->fOwnerEntity( ) );

			{
				profile( cProfilePerfArcRay );

				if( Gameplay_Turrets_TargetingUseDistibutedLoop )
				{
					Threads::tDistributedForLoopCallback callback = caster.fMakeCallback( );
					tGameApp::fInstance( ).fCurrentLevel( )->fRootEntity( )
						->fSceneGraph( )->fDistributeForLoop( callback, steps );
				}
				else
				{
					Threads::tDistributedForLoopCallback callback = caster.fMakeCallback( );
					for( u32 i = 0; i < steps; ++i )
						callback( i );
				}
			}

			// find hit time
			f32 minHitTime = totalTime;

			for( u32 i = 0; i < steps; ++i )
			{
				f32 localTime = caster.fResults( )[ i ];
				if( localTime < Math::cInfinity )
				{
					minHitTime = (i + localTime) * stepRate;
					break; //first hit is good enough.
				}
			}

			// populate arc
			for( f32 time = stepRate; time < minHitTime; time += stepRate )
			{ 
				Math::tVec3f nextPos = arc.fEvaluate( time );
				points.fPushBack( nextPos );
			}

			lastPos = arc.fEvaluate( minHitTime );
			points.fPushBack( lastPos );
		}
		else
		{
			//degenerate
			points.fPushBack( Math::tVec3f::cZeroVector );
			points.fPushBack( Math::tVec3f::cZeroVector );
			lastPos = Math::tVec3f::cZeroVector;
		}

		// apply results
		sigassert( mTargetingLines );
		mTargetingLines->fSetPoints( points, mDesc.mArrowWidth );
		mTargetingLines->fSetRgbaTint( Math::tVec4f( 1, 1, 1, mTargetingAlpha ) );

		f32 pulse = 1.f;

		//// see if the explosion will hit something
		//{
		//	b32 willHit = false;

		//	tProximity proximity;
		//	tDynamicArray<u32> spatialSetIndices( 1 );
		//	spatialSetIndices[ 0 ] = tShapeEntity::cSpatialSetIndex;
		//	proximity.fSetSpatialSetIndices( spatialSetIndices );
		//	proximity.fFilter( ).fAddTag( GameFlags::cFLAG_COLLISION );
		//	proximity.fAddSphere( Math::tSpheref( lastPos, fDesc( ).mExplosionFullSize ) );
		//	proximity.fSetFilterByLogicEnts( true );

		//	proximity.fRefreshMT( 0.f, *tGameApp::fInstance( ).fCurrentLevel( )->fRootEntity( ), &Math::tMat3f::cIdentity );

		//	for( u32 i = 0; i < proximity.fEntityCount( ); ++i )
		//	{
		//		tUnitLogic* logic = proximity.fEntityList( )[ i ]->fLogicDerived<tUnitLogic>( );
		//		if( logic && logic->fIsValidTarget( ) )
		//		{
		//			willHit = true;
		//			break;
		//		}
		//	}

		//	f32 targetIntensity = willHit ? 1.f : 0.f;
		//	mBlinkIntensity = Math::fLerp( mBlinkIntensity, targetIntensity, 0.2f );
		//	mBlinkTimer += dt * Gameplay_Turrets_TargetingPulseFreq;

		//	pulse += (Math::fSin( mBlinkTimer ) * 0.5f + 0.5f) * mBlinkIntensity * Gameplay_Turrets_TargetingPulseScale;
		//}


		Math::tVec3f scale( fDesc( ).mExplosionFullSize, Gameplay_Turrets_TargetingRadiusHeight, fDesc( ).mExplosionFullSize );
		Math::tMat3f scalingMatrix( Math::tMat3f::cIdentity );
		scalingMatrix.fSetDiagonal( scale );
		scalingMatrix.fSetTranslation( lastPos );
		mTargetVisual->fMoveTo( scalingMatrix );
		fSetWorldSpaceArcTargetPosition( lastPos );

		f32 brightness = 1.f * pulse;
		Gfx::tRenderableEntity::fSetRgbaTint( *mTargetVisual, Math::tVec4f( brightness, brightness, brightness, mTargetingAlpha ) );

	}
}

