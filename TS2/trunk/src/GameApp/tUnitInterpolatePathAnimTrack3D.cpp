#include "GameAppPch.hpp"
#include "tUnitInterpolatePathAnimTrack3D.hpp"
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

	devvar( bool, Debug_ContextAnims3D_RenderInterpPaths, false );
	devvar( f32, Debug_ContextAnims3D_DistTolerance, 0.05f );

	tUnitInterpolatePathAnimTrack3D::tUnitInterpolatePathAnimTrack3D( const tUnitInterpolatePath3DAnimDesc& desc )
		: tOrientBasisAnimTrack( tOrientBasisAnimDesc( desc.mBlendIn, desc.mBlendOut, desc.mTimeScale, desc.mBlendScale, desc.mRotateSpeed ) )
		, mUnitPath( desc.mUnitPath )
		, mFirstTick( true )
		, mBlendInSpring( 1.f - fClamp(desc.mBlendInSpring, 0.f, 1.f) )
		, mOffset( tVec3f::cZeroVector )
	{
	}

	void tUnitInterpolatePathAnimTrack3D::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		tEntity *owner = mUnitPath->fOwnerEntity( );

		if( forceFullBlend || fBlendStrength( ) > cBlendEpsilon ) // only compute delta if we're "enough" blended-in
		{
			// Path stuff
			const tVec3f position = owner->fObjectToWorld( ).fGetTranslation( );
			f32 speed = refFrameDelta.mP.fLength( );

			tVec3f pathPoint;
			tVec3f targetDir;
			tVec3f upDir = mUnitPath->fPathMode( ) == tUnitPath::cPathModeFollow ? mUnitPath->fCurrentSegment( ) : tVec3f::cYAxis; // mUnitPath->fWaypoint( )->fObjectToWorld( ).fYAxis( );

			mUnitPath->fInterpolatePath( speed, pathPoint, targetDir );
			targetDir.fNormalizeSafe( tVec3f::cZAxis );
			upDir.fNormalizeSafe( tVec3f::cYAxis );

			tMat3f targ( tMat3f::cIdentity );
			targ.fOrientYWithZAxis( upDir, targetDir );

			// tOrientBasisAnimTrackStuff
			mSource = tQuatf( mUnitPath->fOwnerEntity( )->fObjectToWorld( ) );
			mTarget = tQuatf( targ );
			tOrientBasisAnimTrack::fStepInternal( refFrameDelta, animSkel, forceFullBlend, dt, wrapSign );

			if( mFirstTick && mBlendInSpring != 0.f )
				mOffset = position - mUnitPath->fPrevTargetPosition( );
			else
				mOffset *= mBlendInSpring;

			mFirstTick = false;
			
			tVec3f worldDelta = pathPoint - position + mOffset;
			refFrameDelta.mP = owner->fObjectToWorld( ).fInverseXformVector( worldDelta );

			mUnitPath->fSetDistanceTolerance( Debug_ContextAnims3D_DistTolerance );
			mUnitPath->fSetYDistanceTolerance( Debug_ContextAnims3D_DistTolerance );

			if( Debug_ContextAnims3D_RenderInterpPaths )
			{
				owner->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( position, pathPoint, tVec4f( 0,0,1,1 ) );
				owner->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( position, position + targetDir, tVec4f( 0,1,0,1 ) );
				owner->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( position, position + upDir, tVec4f( 1,0,0,1 ) );

				owner->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( pathPoint, pathPoint + tVec3f( 0,1,0 ) * speed, tVec4f(0,1,0,1) );
				owner->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( position, mUnitPath->fTargetPosition( ), tVec4f(0,1,0,1) );
			}
		}
	}
}


namespace Sig
{
	namespace
	{
		static void fPushAnim( const tUnitInterpolatePath3DAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			stack->fRemoveTracksOfType< tOrientBasisAnimTrack >( );
			stack->fPushTrack( tAnimTrackPtr( NEW tUnitInterpolatePathAnimTrack3D( *desc ) ) );
		}
	}
	void tUnitInterpolatePath3DAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tUnitInterpolatePath3DAnimDesc,Sqrat::DefaultAllocator<tUnitInterpolatePath3DAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("BlendIn"), &tUnitInterpolatePath3DAnimDesc::mBlendIn)
			.Var(_SC("BlendOut"), &tUnitInterpolatePath3DAnimDesc::mBlendOut)
			.Var(_SC("TimeScale"), &tUnitInterpolatePath3DAnimDesc::mTimeScale)
			.Var(_SC("BlendScale"), &tUnitInterpolatePath3DAnimDesc::mBlendScale)
			.Var(_SC("UnitPath"), &tUnitInterpolatePath3DAnimDesc::mUnitPath)
			.Var(_SC("RotateSpeed"), &tUnitInterpolatePath3DAnimDesc::mRotateSpeed)
			.Var(_SC("BlendInSpring"), &tUnitInterpolatePath3DAnimDesc::mBlendInSpring)
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("UnitInterpolatePathTrack3D"), classDesc );
	}
}
