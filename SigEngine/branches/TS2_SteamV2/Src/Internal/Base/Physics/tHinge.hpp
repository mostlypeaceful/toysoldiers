#ifndef __tHinge__
#define __tHinge__


namespace Sig { namespace Physics
{

	class tOneWayHinge : public tRefCounter
	{
	public:
		// These two transforms are relative to body A. The Y axis of the hinge xform is the hinge axis.
		tOneWayHinge( const Math::tMat3f& hingeXform, const Math::tMat3f& bXform );

		void fStepMT( const Math::tVec3f& worldVel, const Math::tVec3f& worldGravity, const Math::tMat3f& aXform, f32 dt );

		const Math::tMat3f& fBRelativeToA( ) const { return mBRelToA; }
		Math::tVec3f fHingePoint( ) const { return mAOffset.fGetTranslation( ); }

		f32 fAngle( ) const { return mAngle; }
		void fSetAngle( f32 angle );

		f32 fAngleVel( ) const { return mAngleVel; }
		void fSetAngleVel( f32 angVel );

		const f32 fLowerLimit( ) const { return mLowerLimit; }
		const f32 fUpperLimit( ) const { return mUpperLimit; }

		void fLatch( f32 angle );
		void fUnLatch( );
		b32 fLatched( ) const { return mLatched; }
		void fAutoLatch( f32 delay );

		static void fExportScriptInterface( tScriptVm& vm );

	private:
		Math::tMat3f mAOffset, mBOffset;
		Math::tMat3f mBRelToA;
		Math::tVec3f mLastHingeVel;
		f32 mAngle;
		f32 mAngleVel;
		f32 mLowerLimit;
		f32 mUpperLimit;
		f32 mDamping;
		f32 mBtoHingeRadius;
		b16 mLatched;
		b16 mAutoLatch;
		f32 mAutoLatchDelay;

		void fRecomputeBRelToA( );
	};

	typedef tRefCounterPtr<tOneWayHinge> tOneWayHingePtr;

}}

#endif//__tHinge__
