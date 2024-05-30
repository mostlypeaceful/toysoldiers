#include "GameAppPch.hpp"
#include "tTurretOrientAnimTrack.hpp"
#include "tAnimatedSkeleton.hpp"
#include "tTurretLogic.hpp"

namespace Sig
{
	namespace
	{
		static const f32 cBlendEpsilon = 0.0001f;
		static const tStringPtr cTurretOrientTrackTag("TurretOrientTrackTag");
	}

	tTurretOrientAnimTrack::tTurretOrientAnimTrack( const tTurretOrientAnimDesc& desc )
		: tOrientAnimTrack( tOrientAnimDesc( desc.mBlendIn, desc.mBlendOut, desc.mTimeScale, desc.mBlendScale, desc.mTurret->fUnitAttributeAIRotateSpeed( ) ) )
		, mTurret( desc.mTurret )
	{
		log_assert( mTurret, "You must specify a valid Turret when pushing a TurretOrientTrack anim track." );
		fSetTag( cTurretOrientTrackTag );
	}

	void tTurretOrientAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		if( !mTurret->fUnderUserControl( ) )
		{
			mSourceVector = mTurret->fWorldToUser( mTurret->fOwnerEntity( )->fObjectToWorld( ).fZAxis( ) );
			mTargetVector = mTurret->fDesiredFacingUserDirection( );
			mAlwaysSmooth = true;
			tOrientAnimTrack::fStepInternal( refFrameDelta, animSkel, forceFullBlend, dt, wrapSign );
		}
	}
}


namespace Sig
{
	namespace
	{
		static void fPushAnim( const tTurretOrientAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			for( u32 i = 0; i < stack->fTrackCount( ); ++i )
				if( stack->fTrack( i ).fTag( ) == cTurretOrientTrackTag )
					stack->fTrack( i ).fBeginBlendingOut( 0.125f );
			stack->fPushTrack( tAnimTrackPtr( NEW tTurretOrientAnimTrack( *desc ) ) );
		}
	}
	void tTurretOrientAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tTurretOrientAnimDesc,Sqrat::DefaultAllocator<tTurretOrientAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("BlendIn"), &tTurretOrientAnimDesc::mBlendIn)
			.Var(_SC("BlendOut"), &tTurretOrientAnimDesc::mBlendOut)
			.Var(_SC("TimeScale"), &tTurretOrientAnimDesc::mTimeScale)
			.Var(_SC("BlendScale"), &tTurretOrientAnimDesc::mBlendScale)
			.Var(_SC("Turret"), &tTurretOrientAnimDesc::mTurret)
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("TurretOrientTrack"), classDesc );
	}
}
