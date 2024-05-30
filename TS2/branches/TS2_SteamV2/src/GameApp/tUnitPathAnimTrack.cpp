#include "GameAppPch.hpp"
#include "tUnitPathAnimTrack.hpp"
#include "tKeyFrameAnimation.hpp"
#include "tAnimatedSkeleton.hpp"
#include "Logic/tPhysical.hpp"
#include "tGameApp.hpp" //for GameEvents

namespace Sig
{
	namespace
	{
		static const f32 cBlendEpsilon = 0.0001f;
	}

	devvar_clamp( f32, Anim_OrientationAlignedThreshold, 0.02f, 0.0f, 0.5f, 2 );

	tUnitPathAnimTrack::tUnitPathAnimTrack( const tUnitPathAnimDesc& desc )
		: tOrientAnimTrack( tOrientAnimDesc( desc.mBlendIn, desc.mBlendOut, desc.mTimeScale, desc.mBlendScale, desc.mRotateSpeed ) )
		, mUnitPath( desc.mUnitPath )
		, mFireEventWhenAligned( desc.mFireEventWhenAligned )
	{
	}

	void tUnitPathAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		Physics::tStandardPhysics* physical = mUnitPath->fOwnerEntity( )->fLogic( )->fQueryPhysicalDerived< Physics::tStandardPhysics >( );
		if( !physical || !physical->fFalling( ) )
		{
			mSourceVector = mUnitPath->fOwnerEntity( )->fObjectToWorld( ).fZAxis( );
			mTargetVector = fGetTarget( );
			
			tOrientAnimTrack::fStepInternal( refFrameDelta, animSkel, forceFullBlend, dt, wrapSign );

			if( mFireEventWhenAligned && fAbs( fGetErrorAngle( ) ) < Anim_OrientationAlignedThreshold )
			{
				animSkel.fQueueEvent( tKeyFrameEvent( 0.0f, 0.0f, GameFlags::cEVENT_UNIT_ALIGNED, ~0, tStringPtr( ) ) );
			}
		}
	}

	const Math::tVec3f tUnitPathAnimTrack::fGetTarget( )
	{
		return mUnitPath->fTargetPosition( ) - mUnitPath->fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
	}
}


namespace Sig
{
	namespace
	{
		static void fPushAnim( const tUnitPathAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			sigassert( desc );
			stack->fRemoveTracksOfType<tOrientAnimTrack>( );
			stack->fPushTrack( tAnimTrackPtr( NEW tUnitPathAnimTrack( *desc ) ) );
		}
	}
	void tUnitPathAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tUnitPathAnimDesc,Sqrat::DefaultAllocator<tUnitPathAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("BlendIn"), &tUnitPathAnimDesc::mBlendIn)
			.Var(_SC("BlendOut"), &tUnitPathAnimDesc::mBlendOut)
			.Var(_SC("TimeScale"), &tUnitPathAnimDesc::mTimeScale)
			.Var(_SC("BlendScale"), &tUnitPathAnimDesc::mBlendScale)
			.Var(_SC("UnitPath"), &tUnitPathAnimDesc::mUnitPath)
			.Var(_SC("RotateSpeed"), &tUnitPathAnimDesc::mRotateSpeed)
			.Var(_SC("FireEventWhenAligned"), &tUnitPathAnimDesc::mFireEventWhenAligned)
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("UnitPathTrack"), classDesc );
	}
}
