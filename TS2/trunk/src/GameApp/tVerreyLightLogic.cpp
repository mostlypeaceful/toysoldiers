#include "GameAppPch.hpp"
#include "tVerreyLightLogic.hpp"
#include "tSceneGraph.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tSync.hpp"

#include "Wwise_IDs.h"

using namespace Sig::Math;

namespace Sig
{
	devvar( f32, Gameplay_Generators_VerreyLights_Height, 20.0f );
	devvar( f32, Gameplay_Generators_VerreyLights_HorzVel, 2.0f );
	devvar_clamp( f32, Gameplay_Generators_VerreyLights_Gravity, -11.0f, -20.0f, 20.0f, 2 );

	namespace {
		const tFilePathPtr cVerreyLightPath( "Effects/Entities/Projectiles/verrey_light.sigml" );
	} //unnamed namespace

	tVerreyLightLogic::tVerreyLightLogic( )
		: mDeathHeight( 0.f )
	{ 
	}

	tVerreyLightLogic::~tVerreyLightLogic( )
	{ }

	void tVerreyLightLogic::fOnSpawn( )
	{
		fOnPause( false );

		mAudioSource.fReset( NEW Audio::tSource( "VerryLight" ) );
		mAudioSource->fSpawnImmediate( *fOwnerEntity( ) );
		mAudioSource->fUpdatePosition( );
		mAudioSource->fHandleEvent( AK::EVENTS::PLAY_FLARE_FIRE );
	}

	void tVerreyLightLogic::fOnDelete( )
	{ 
		mAudioSource.fRelease( );
		tLogic::fOnDelete( );
	}

	void tVerreyLightLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
			fRunListRemove( cRunListMoveST );
		}
		else
		{
			fRunListInsert( cRunListActST );
			fRunListInsert( cRunListMoveST );
		}
	}

	void tVerreyLightLogic::fActST( f32 dt )
	{
		if( fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ).y <= mDeathHeight )
			fOwnerEntity( )->fDelete( );
	}

	void tVerreyLightLogic::fMoveST( f32 dt )
	{
		Math::tVec3f nextPos = mPhysics.fStep( dt );

		fOwnerEntity( )->fMoveTo( nextPos );
	}

	/*static*/ void tVerreyLightLogic::fSpawn( const Math::tVec3f& origin )
	{
		tEntity* proj = tGameApp::fInstance( ).fCurrentLevel( )->fOwnerEntity( )->fSpawnChild( cVerreyLightPath );

		if( proj )
		{
			proj->fMoveTo( origin );

			tVerreyLightLogic *vl = NEW tVerreyLightLogic( );
			tLogicPtr *vlp = NEW tLogicPtr( vl );
			proj->fAcquireLogic( vlp );

			f32 gravity = Gameplay_Generators_VerreyLights_Gravity;
			f32 fallTime = fSqrt( Gameplay_Generators_VerreyLights_Height / (-0.5f * gravity) );
			f32 fallSpeed = -0.5f * gravity * fallTime * fallTime;
			f32 speed = fallSpeed;

			Math::tVec3f launchVec = sync_rand( fVecNorm< Math::tVec3f >( ) ) * Gameplay_Generators_VerreyLights_HorzVel;
			launchVec.y = speed;

			vl->mPhysics.mGravity = Math::tVec3f( 0, gravity, 0 );
			vl->mPhysics.mPosition = origin;
			vl->mPhysics.mVelocity = launchVec;
			vl->mDeathHeight = origin.y - 5.0f;
		}
	}

}


namespace Sig
{
	void tVerreyLightLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tVerreyLightLogic, tLogic, Sqrat::NoCopy<tVerreyLightLogic> > classDesc( vm.fSq( ) );
		classDesc
			;

		vm.fRootTable( ).Bind(_SC("VerreyLightLogic"), classDesc);
	}
}

