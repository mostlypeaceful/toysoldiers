#include "GameAppPch.hpp"
#include "tUnitInterpolatePathAnimTrack.hpp"
#include "tKeyFrameAnimation.hpp"
#include "tAnimatedSkeleton.hpp"
#include "Logic/tPhysical.hpp"
#include "tSceneGraph.hpp"

using namespace Sig::Math;

namespace Sig
{
	namespace
	{
		static const f32 cBlendEpsilon = 0.0001f;
	}

	devvar( bool, Debug_ContextAnims_RenderInterpPaths, false );
	devvar( f32, Debug_ContextAnims_FinalLerpVal, 0.25f );
	devvar( f32, Debug_ContextAnims_DistTolerance, 0.05f );

	tUnitInterpolatePathAnimTrack::tUnitInterpolatePathAnimTrack( const tUnitInterpolatePathAnimDesc& desc )
		: tAnimTrack( desc.mBlendIn, desc.mBlendOut, desc.mTimeScale, desc.mBlendScale, 0.f, Math::cInfinity, 0.f, cFlagPartial )
		, mUnitPath( desc.mUnitPath )
		, mFirstTick( true )
	{
	}

	void tUnitInterpolatePathAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		tEntity *owner = mUnitPath->fOwnerEntity( );

		if( forceFullBlend || fBlendStrength( ) > cBlendEpsilon ) // only compute delta if we're "enough" blended-in
		{
			const tVec3f position = owner->fObjectToWorld( ).fGetTranslation( );
			f32 speed = refFrameDelta.mP.fLength( );

			tVec3f pathPoint, futurePoint;
			f32 pathDist;
			mUnitPath->fDistanceToPath( position, pathPoint, pathDist, futurePoint, speed );

			if( mFirstTick )
			{
				mOffset = position - pathPoint;
				mFirstTick = false;
			}
			else
				mOffset = fLerp( mOffset, tVec3f::cZeroVector, (f32)Debug_ContextAnims_FinalLerpVal );
			
			tVec3f worldDelta = futurePoint - position + mOffset;
			refFrameDelta.mP = owner->fObjectToWorld( ).fInverseXformVector( worldDelta );

			mUnitPath->fSetDistanceTolerance( Debug_ContextAnims_DistTolerance );

			if( Debug_ContextAnims_RenderInterpPaths )
			{
				owner->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( futurePoint, futurePoint + tVec3f( 0,1,0 ) * speed, tVec4f(0,1,0,1) );
				owner->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( position, mUnitPath->fTargetPosition( ), tVec4f(0,1,0,1) );
			}
		}
	}
}


namespace Sig
{
	namespace
	{
		static void fPushAnim( const tUnitInterpolatePathAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			stack->fPushTrack( tAnimTrackPtr( NEW tUnitInterpolatePathAnimTrack( *desc ) ) );
		}
	}
	void tUnitInterpolatePathAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tUnitInterpolatePathAnimDesc,Sqrat::DefaultAllocator<tUnitInterpolatePathAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("BlendIn"), &tUnitInterpolatePathAnimDesc::mBlendIn)
			.Var(_SC("BlendOut"), &tUnitInterpolatePathAnimDesc::mBlendOut)
			.Var(_SC("TimeScale"), &tUnitInterpolatePathAnimDesc::mTimeScale)
			.Var(_SC("BlendScale"), &tUnitInterpolatePathAnimDesc::mBlendScale)
			.Var(_SC("UnitPath"), &tUnitInterpolatePathAnimDesc::mUnitPath)
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("UnitInterpolatePathTrack"), classDesc );
	}
}
