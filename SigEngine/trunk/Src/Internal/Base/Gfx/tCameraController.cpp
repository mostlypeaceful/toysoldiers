#include "BasePch.hpp"
#include "tCameraController.hpp"
#include "tUser.hpp"
#include "tCamera.hpp"
#include "tRandom.hpp"
#include "tSync.hpp"

namespace Sig { namespace Gfx
{

	class base_export tCameraShake : public tRefCounter
	{
	private:
		Math::tVec2f mStrength;
		f32 mLength;
		Math::tVec2f mCurrentDelta;
		f32 mCurrentTime;

	public:
		tCameraShake( const Math::tVec2f& strength, f32 length )
			: mStrength( strength )
			, mLength( length )
			, mCurrentTime( -1.f )
			, mCurrentDelta( 0.f )
		{
		}

		b32 fStepShake( f32 dt, Math::tVec3f& randomOut )
		{
			if( mLength <= 0.f )
				return false;

			if( mCurrentTime < 0.f )
			{
				mCurrentTime = sync_randc( fFloatInRange( 0.01f, 0.05f ), "CurrentTime" );
				mCurrentDelta = sync_randc( fVecNorm<Math::tVec2f>( ), "CurrentDelta" ) * mStrength;
			}

			randomOut += Math::tVec3f( mCurrentDelta * sync_randc( fFloatInRange( 0.5f, 1.5f ), "MoveLocal" ), 0 );

			mLength -= dt;
			mCurrentTime -= dt;
			return true;
		}
	};

	tCameraController::tCameraController( const tViewportPtr& viewport )
		: mViewport( viewport )
		, mIsActive( false )
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
		mCameraShakes.fPushBack( tCameraShakePtr( NEW tCameraShake( strength, length ) ) );
	}

	void tCameraController::fStepCameraShake( f32 dt, tCamera& camera )
	{
		if( mCameraShakes.fCount( ) )
		{
			Math::tVec3f randomMag = Math::tVec3f::cZeroVector;

			for( s32 i = mCameraShakes.fCount( ) - 1; i >= 0; --i )
			{
				if( !mCameraShakes[ i ]->fStepShake( dt, randomMag ) )
					mCameraShakes.fPopBack( );
			}

			camera.fMoveLocal( randomMag );
		}
	}

}}

