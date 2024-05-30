#include "BasePch.hpp"
#include "tCameraController.hpp"
#include "tUser.hpp"
#include "tCamera.hpp"
#include "tRandom.hpp"
#include "tSync.hpp"

namespace Sig { namespace Gfx
{
	class base_export tCameraHitByExplosion : public tRefCounter
	{
		Math::tVec3f mExplosionPosition;
		Math::tVec3f mExplosivePush;
		f32 mExplosionStrength;
		f32 mExplosionRadius;
		b32 mDoCalculations;
		f32 mTimer;

	public:
		tCameraHitByExplosion( const Math::tVec3f& explosionPosition, f32 explosionStrength, f32 explosionRadius )
			: mExplosionPosition( explosionPosition )
			, mExplosionStrength( explosionStrength )
			, mExplosionRadius( explosionRadius )
			, mDoCalculations( true )
			, mTimer( 0.f )
		{
			
		}

		b32 fStep( f32 dt, tCamera& camera, tUserPtr& user )
		{
			if( mExplosionStrength < 1.f )
				return false;

			if( mDoCalculations )
			{
				const Math::tVec3f sub = camera.fGetTripod( ).mEye - mExplosionPosition;
				f32 len = sub.fLength( );
				f32 minRadius = mExplosionRadius * 2.f;
				if( len > minRadius )
					return false;

				f32 strength = minRadius / len;
				mExplosivePush = sub;
				mExplosivePush.fNormalizeSafe( Math::tVec3f::cZeroVector );
				mExplosivePush *= mExplosionStrength * fMin( 3.f, strength );

				mDoCalculations = false;

				if( user )
				{
					//f32 rumbleMultiplier = 1.f - ( len / minRadius );

					log_warning( Log::cFlagInput, "need to re-enable gamepad rumble!" );
					//user->mGamepad.fSetRumble( 1.f*rumbleMultiplier, 0.f );
					//user->mGamepad.fSetRumbleSlowdown( 0.5f, 0.5f );
				}
			}

			f32 invDelta = 1.f - ( mTimer / 1.f );
			camera.fMoveGlobal( mExplosivePush * ( invDelta*invDelta ) * dt );
			mTimer += dt;
			if( mTimer > 1.f )
				return false;

			return true;
		}
	};

	class base_export tCameraShake : public tRefCounter
	{
	private:
		Math::tVec2f mStrength;
		f32 mLength;
		Math::tVec2f mCurrentDelta;
		f32 mCurrentTime;

	public:
		tCameraShake( );
		void fBeginShake( const Math::tVec2f& strength, f32 length );
		b32  fStepShake( f32 dt, tCamera& camera, f32 scale );
	};

	tCameraShake::tCameraShake( )
		: mStrength( 0.f )
		, mLength( -1.f )
		, mCurrentTime( -1.f )
		, mCurrentDelta( 0.f )
	{
	}
	void tCameraShake::fBeginShake( const Math::tVec2f& strength, f32 length )
	{
		mStrength = strength;
		mLength = length;
	}
	b32 tCameraShake::fStepShake( f32 dt, Gfx::tCamera& camera, f32 scale )
	{
		if( mLength <= 0.f )
			return false;

		if( mCurrentTime < 0.f )
		{
			mCurrentTime = sync_randc( fFloatInRange( 0.01f, 0.05f ), "CurrentTime" );
			mCurrentDelta = sync_randc( fVecNorm<Math::tVec2f>( ), "CurrentDelta" ) * mStrength;
		}

		camera.fMoveLocal( Math::tVec3f( mCurrentDelta * (scale * sync_randc( fFloatInRange( 0.5f, 1.5f ), "MoveLocal" )), 0.f ) );

		mLength -= dt;
		mCurrentTime -= dt;
		return true;
	}

	tCameraController::tCameraController( const tViewportPtr& viewport )
		: mViewport( viewport )
		, mIsActive( false )
		, mCameraShakeScale( 1.f )
	{
	}

	tCameraController::~tCameraController( )
	{
	}

	void tCameraController::fAddUser( const tUserPtr& user )
	{
		if( !mUsers.fFind( user ) )
			mUsers.fPushBack( user );
	}

	void tCameraController::fRemoveUser( const tUserPtr& user )
	{
		mUsers.fFindAndErase( user );
	}

	void tCameraController::fClearUsers( )
	{
		mUsers.fDeleteArray( );
	}

	void tCameraController::fBeginCameraShake( const Math::tVec2f& strength, f32 length )
	{
		if( !mCameraShake )
			mCameraShake.fReset( NEW tCameraShake( ) );
		mCameraShake->fBeginShake( strength, length );
	}

	void tCameraController::fHitCameraWithExplosion( const Math::tVec3f& explosionPos, f32 explosionStrength, f32 explosionRadius )
	{
		if( !mCameraExplosion )
			mCameraExplosion.fReset( NEW tCameraHitByExplosion( explosionPos, explosionStrength, explosionRadius ) );
	}

	b32 tCameraController::fEvaluateLookAt( Math::tMat3f& lookAtReferenceFrame )
	{
		if( mLookAtCb.fNull( ) || mLookAtCb->fNull( ) )
		{
			mLookAtCb.fRelease( );
			return false;
		}

		(*mLookAtCb)( lookAtReferenceFrame );
		return true;
	}

	void tCameraController::fStepCameraShake( f32 dt, tCamera& camera )
	{
		if( mCameraShake )
		{
			if( !mCameraShake->fStepShake( dt, camera, mCameraShakeScale ) )
				mCameraShake.fRelease( );
		}
	}

	void tCameraController::fStepCameraExplosion( f32 dt, tCamera& camera )
	{
		if( mCameraExplosion )
		{
			if( !mCameraExplosion->fStep( dt, camera, mUsers.fFront( ) ) )
				mCameraExplosion.fRelease( );
		}
	}

}}

