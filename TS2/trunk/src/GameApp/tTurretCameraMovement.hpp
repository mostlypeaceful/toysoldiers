#ifndef __tTurretCameraMovement__
#define __tTurretCameraMovement__

namespace Sig
{

	// Common place for turret camera movement so it is common between all units.
	//  Turrets, vehicle turrets, hover units, etc. All will have the same feel.
	class base_export tTurretCameraMovement
	{
	public:
		tTurretCameraMovement( );

		void fReset( const Math::tVec3f& direction );
		void fSetSpeed( const Math::tVec2f& speed ) { mSpeed = speed; }
		void fSetDamping( const Math::tVec2f& damping ) { mDamping = Math::tVec2f( 1 ) - damping; }
		void fSetMinMaxPitch( f32 min, f32 max ) { mPitchMin = min; mPitchMax = max; }
		void fUpdate( f32 dt, Math::tVec2f stickInput );

		void				fSetYawPitchVelocity( const Math::tVec2f& vel ) { mYawPitchVel = vel; }		
		const Math::tVec2f& fYawPitchVelocity( ) const { return mYawPitchVel; }		
		f32					fCurrentPitch( ) const { return mCurrentCameraPitch; }
		const Math::tMat3f&	fCameraBasis( ) const	{ return mCameraBasis; }
		Math::tMat3f&		fCameraBasis( )			{ return mCameraBasis; }
		const Math::tMat3f& fFlatCamera( ) const	{ return mFlatCamera; }
		Math::tMat3f&		fFlatCamera( )			{ return mFlatCamera; }


	private:
		Math::tVec2f mYawPitchVel;
		Math::tVec2f mSpeed;
		Math::tVec2f mDamping;
		f32 mPitchMax, mPitchMin;
		f32 mCurrentCameraPitch; //Radians
		Math::tMat3f mCameraBasis, mFlatCamera;
	};

}

#endif//__tTurretCameraMovement__

