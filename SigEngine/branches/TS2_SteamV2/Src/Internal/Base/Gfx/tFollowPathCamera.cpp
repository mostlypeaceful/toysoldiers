#include "BasePch.hpp"
#include "tFollowPathCamera.hpp"
#include "tGameAppBase.hpp"


namespace Sig { namespace Math
{
	template<class t>
	static tQuaternion<t> fCatmullRom( const tQuaternion<t>& p0, const tQuaternion<t>& p1, const tQuaternion<t>& p2, const tQuaternion<t>& p3, t x )
	{
		const t x2 = x * x;
		const t x3 = x2 * x;
		const tQuaternion<t> m1 = 0.5f*( p2 - p0 );
		const tQuaternion<t> m2 = 0.5f*( p3 - p1 );
		tQuaternion<t> o =  
			(2*x3 - 3*x2 + 1)*p1 +
			(x3 - 2*x2 + x)*m1 +
			(-2*x3 + 3*x2)*p2 +
			(x3 - x2)*m2;
		return o.fNormalize( );
	}
	template<class t>
	static tPRSXform<t> fCatmullRom( const tPRSXform<t>& p0, const tPRSXform<t>& p1, const tPRSXform<t>& p2, const tPRSXform<t>& p3, t x )
	{
		tPRSXform<t> temp;
		temp.mP = fCatmullRom( p0.mP, p1.mP, p2.mP, p3.mP, x );
		temp.mR = fCatmullRom( p0.mR, p1.mR, p2.mR, p3.mR, x );
		temp.mS = fLerp( p1.mS, p2.mS, x );
		return temp;
	}
}}

namespace Sig { namespace Gfx
{
	tFollowPathCamera::tControlPointList::tControlPointList( ) 
		: mTotalLength( 1.f )
		, mParamsSet( false )
	{
	}
	f32 tFollowPathCamera::tControlPointList::fComputeFractionOfPath( u32 ithSegment )
	{
		const u32 cpCount = fCount( );
		if( cpCount < 2 )
			return 0.f; // dummy value
		const f32 fractionOfPath = 1.f / ( cpCount - 1.f );
		//const f32 fractionOfPath = fIndex( ithSegment ).mSegmentLength / mTotalLength;
		return fractionOfPath;
	}
	f32 tFollowPathCamera::tControlPointList::fComputeSegmentDuration( u32 ithSegment )
	{
		const f32 fractionOfPath = fComputeFractionOfPath( ithSegment );
		const f32 duration = mParams.mTotalTime * fractionOfPath;
		return duration;
	}
	void tFollowPathCamera::tControlPointList::fSeal( )
	{
		const u32 cpCount = fCount( );
		if( cpCount < 2 )
			mTotalLength = 1.f; // dummy value
		else
		{
			f32 totalPathLength = 0.f;
			for( u32 i = 0; i < cpCount - 1; ++i )
			{
				const f32 segLength = ( fIndex( i + 1 ).mXform.mP - fIndex( i ).mXform.mP ).fLength( );
				fIndex( i ).mSegmentLength = segLength;
				totalPathLength += segLength;
			}
			mTotalLength = fMax( totalPathLength, 0.001f ); // prevent a path length of zero

			for( u32 i = 1; i < cpCount; ++i )
			{
				if( fIndex( i ).mXform.mR.fDot( fIndex( i - 1 ).mXform.mR ) < 0.f )
					fIndex( i ).mXform.mR = -fIndex( i ).mXform.mR;
			}
		}
	}

	void tFollowPathCamera::fBuildControlPointList( tControlPointList& cps, tEntity& root, Math::tVec3f targetPt, tPathEntity& waypoint )
	{
		// get waypoint matrix
		Math::tMat3f xform = waypoint.fObjectToWorld( );
		xform.fNormalizeBasis( );

		tFollowPathCameraPointLogic* logic = waypoint.fLogicDerived<tFollowPathCameraPointLogic>( );
		if( logic )
		{
			if( !cps.mParamsSet )
			{
				cps.mParams = logic->mParams;
				cps.mParamsSet = true;
			}

			// look for 'lookat' attachment point
			const tStringPtr lookatWaypointName = logic->mParams.mLookAtTarget;
			if( lookatWaypointName.fExists( ) )
			{
				tEntity* lookat = root.fFirstDescendentWithName( lookatWaypointName );
				if( lookat ) targetPt = lookat->fObjectToWorld( ).fGetTranslation( );
			}
		}

		const Math::tVec3f toLookAt = ( targetPt - xform.fGetTranslation( ) ).fNormalizeSafe( Math::tVec3f::cZAxis );
		xform.fOrientZAxis( toLookAt, Math::tVec3f::cYAxis );

		cps.fPushBack( xform );
		cps.fBack( ).mPathPoint.fReset( &waypoint );

		if( !waypoint.fNextPointCount( ) )
			return; // no more

		tPathEntity* nextWaypoint = waypoint.fNextPoint( 0 );
		fBuildControlPointList( cps, root, targetPt, *nextWaypoint );
	}

	tFollowPathCamera::tFollowPathCamera( 
		const tStringPtr& key, 
		const tSkipButtonHeldFunc& skipButtonHeldFunc, 
		const tUserPtr& user, 
		tCameraControllerStack* stack, 
		const tLens& lens, 
		const tControlPointList& cps, 
		const tOnEndOfPathReached& callAtEndOfPath )
		: tCameraController( user->fViewport( ) )
		, mStack( stack )
		, mKey( key )
		, mSkipped( false )
		, mOnEndOfPathReached( callAtEndOfPath )
		, mControlPoints( cps )
		, mCurrentCp( 0 )
		, mTimeToReachWayPoint( 4.f )
		, mTimer( 0.f )
		, mEntirePathPos( 0.f )
		//, mGamePad( gp )
	{
		fAddUser( user );
		if( !skipButtonHeldFunc.fNull( ) )
		{
			mSkipButtonHeldFunc = skipButtonHeldFunc;
			fAddGamePad( user );
		}

		const u32 cpCount = mControlPoints.fCount( );
		if( cpCount >= 2 )
		{
			mPre  = mControlPoints[0].mXform + ( mControlPoints[0].mXform - mControlPoints[1].mXform );
			mPost = mControlPoints[cpCount-1].mXform + ( mControlPoints[cpCount-1].mXform - mControlPoints[cpCount-2].mXform );

			mControlPoints.fSeal( );
			mTimeToReachWayPoint = mControlPoints.fComputeSegmentDuration( mCurrentCp );
		}
		else
		{
			mPre = mPost = Math::tPRSXformf::cIdentity;
		}

		fViewport( )->fSetCameras( tCamera( lens, tTripod( Math::tMat3f::cIdentity ) ) );
	}

	tFollowPathCamera::~tFollowPathCamera( )
	{
		const u32 gpCount = mGamepads.fCount( );
		for( u32 gp = 0; gp < gpCount; ++gp )
		{
			mGamepads[ gp ].mUser->fDecInputFilterLevel( mGamepads[ gp ].mFilter );
		}
	}


	void tFollowPathCamera::fOnTick( f32 dt )
	{
		fOnTickInternal( dt );
		fSetCamera( );
	}

	const Gfx::tTripod& tFollowPathCamera::fStepSlave( f32 dt )
	{
		fOnTickInternal( dt );
		return mTripod;
	}

	void tFollowPathCamera::fOnTickInternal( f32 dt )
	{
		if( !tGameAppBase::fInstance( ).fSceneGraph( )->fIsPaused( ) )
		{
			const u32 cpCount = mControlPoints.fCount( );
			b32 skipPressed = false;

			//for( u32 i = 0; i < mGamepads.fCount( ); ++i )
			//{
			//	const Input::tGamepad & pad = mGamepads[ i ].mUser->fFilteredGamepad( mGamepads[ i ].mFilter );
			//	if( pad.fButtonHeld( Input::tGamepad::cButtonA ) || pad.fButtonHeld( Input::tGamepad::cButtonB ) )
			//	{
			//		skipPressed = true;
			//		mSkipped = true;
			//	}
			//}
			
			for( u32 i = 0; i < mGamepads.fCount( ); ++i )
			{
				if( mSkipButtonHeldFunc( *this, mGamepads[ i ].mUser ) )
				{
					skipPressed = true;
					mSkipped = true;
				}
			}

			if( ( s32 )mCurrentCp >= ( s32 )cpCount - 1 || skipPressed )
			{
				fCutToEnd( );

				tCameraControllerPtr preserveMe( this );
				mOnEndOfPathReached( *this );
				return;
			}


			f32 dtScaleFactor = 1.f;
			if( mEntirePathPos <= 0.5f )
			{
				const f32 easeIn = 0.5f * mControlPoints.mParams.mEaseIn;
				if( mEntirePathPos < easeIn )
					dtScaleFactor = mEntirePathPos / easeIn;
			}
			else
			{
				const f32 easeOut = 0.5f * mControlPoints.mParams.mEaseOut;
				if( mEntirePathPos > 1.f - easeOut )
					dtScaleFactor = (1.f-mEntirePathPos) / easeOut;
			}

			const f32 derivedSpeedMultiplier = fPathSpeedMultiplier( dt );
			dt = dt * Math::fLerp( 0.05f, 1.f, dtScaleFactor ) * derivedSpeedMultiplier;

			const f32 t = mTimer / mTimeToReachWayPoint;

			fSetTripod( mCurrentCp, t );

			mTimer += dt;
			if( mTimer >= mTimeToReachWayPoint )
			{
				fArrivedAtNextWaypoint( );

				++mCurrentCp;
				mTimer = 0.f;
				mTimeToReachWayPoint = mControlPoints.fComputeSegmentDuration( mCurrentCp );
			}
			else
				mEntirePathPos += ( dt / mTimeToReachWayPoint ) * mControlPoints.fComputeFractionOfPath( mCurrentCp );
		}
	}

	void tFollowPathCamera::fSetTripod( u32 cp, f32 t )
	{
		const u32 cpCount = mControlPoints.fCount( );
		const Math::tPRSXformf prs0 = ( cp == 0 ) ? mPre : mControlPoints[ cp - 1 ].mXform;
		const Math::tPRSXformf prs1 = mControlPoints[ cp + 0 ].mXform;
		const Math::tPRSXformf prs2 = mControlPoints[ cp + 1 ].mXform;
		const Math::tPRSXformf prs3 = ( cp == cpCount - 2 ) ? mPost : mControlPoints[ cp + 2 ].mXform;
		const Math::tPRSXformf prs = Math::fCatmullRom( prs0, prs1, prs2, prs3, t );

		Math::tMat3f xform;
		prs.fToMatrix( xform );

		mTripod = tTripod( xform );
	}

	void tFollowPathCamera::fSetCamera( )
	{
		tCamera camera = fViewport( )->fLogicCamera( );
		camera.fSetTripod( mTripod );
		fViewport( )->fSetCameras( camera );
	}

	void tFollowPathCamera::fCutToEnd( )
	{
		if( mControlPoints.fCount( ) < 2 )
			return;

		fSetTripod( mControlPoints.fCount( ) - 2, 1.0f );
		fSetCamera( );
	}

	void tFollowPathCamera::fAddGamePad( const tUserPtr& user )
	{
		// Safety check that we don't already have it
		const u32 gpCount = mGamepads.fCount( );
		for( u32 gp = 0; gp < gpCount; ++gp )
		{
			if( mGamepads[ gp ].mUser == user )
				return;
		}

		tFilteredPad pad;
		pad.mUser = user;
		pad.mFilter = user->fIncInputFilterLevel( );
		mGamepads.fPushBack( pad );
	}

}}



namespace Sig { namespace Gfx
{
	void tFollowPathCameraPointLogic::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class<tFollowPathCameraControlParams, Sqrat::DefaultAllocator<tFollowPathCameraControlParams> > classDesc( vm.fSq( ) );
			classDesc
				.Var(_SC("TotalTime"),		&tFollowPathCameraControlParams::mTotalTime)
				.Var(_SC("EaseIn"),			&tFollowPathCameraControlParams::mEaseIn)
				.Var(_SC("EaseOut"),		&tFollowPathCameraControlParams::mEaseOut)
				.Var(_SC("GameBlendIn"),	&tFollowPathCameraControlParams::mGameBlendIn)
				.Var(_SC("LookAtTarget"),	&tFollowPathCameraControlParams::mLookAtTarget)
				;
			vm.fRootTable( ).Bind( _SC("FollowPathCameraControlParams"), classDesc );
		}
		{
			Sqrat::DerivedClass<tFollowPathCameraPointLogic, tLogic, Sqrat::NoCopy<tFollowPathCameraPointLogic> > classDesc( vm.fSq( ) );
			classDesc
				.Prop(_SC("Params"),			&tFollowPathCameraPointLogic::fParamsForScript)
				;
			vm.fRootTable( ).Bind( _SC("FollowPathCameraPointLogic"), classDesc );
		}
	}
} }
