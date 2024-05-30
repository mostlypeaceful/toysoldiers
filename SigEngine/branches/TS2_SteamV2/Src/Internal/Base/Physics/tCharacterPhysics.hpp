#ifndef __tCharacterPhysics__
#define __tCharacterPhysics__
#include "Logic/tPhysical.hpp"
#include "tProximity.hpp"

namespace Sig { namespace Physics
{
	class tCharacterPhysics : public tStandardPhysics
	{
		define_dynamic_cast( tCharacterPhysics, tStandardPhysics );
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	public:
		tCharacterPhysics( );
		void fOnDelete( );

		void fBasicSetup( u32 groundMask, u32 collisionMask, f32 height );

		void fPhysicsMT( tLogic* logic, f32 dt, b32 highPri );
		void fCoRenderMT( tLogic* logic, f32 dt );
		void fThinkST( tLogic* logic, f32 dt );

		void fCollideAndResolve( tLogic* logic, f32 dt, const Math::tSpheref& collisionSphere, const Math::tSpheref& prevCollisionSphere, const tGrowableArray< tEntity* >& offenders );

		void fSetCharacterHeight( f32 height );

		/// Add some time here for when you know the character will be in the air, so their collision query is deprioritied. (may fall out of level)
		void fSetDeprioritizedCollisionTimer( f32 fbt ) { mDeprioritizedColTimer = fbt; }

		tEntityTagMask fCollisionMask( ) const { return mCollisionMask; }
		void           fSetCollisionMask( tEntityTagMask mask ) { mCollisionMask = mask; }

		// Additional mask to include with query
		tEntityTagMask fQueryMask( ) const { return mQueryMask; }
		void           fSetQueryMask( tEntityTagMask mask ) { mQueryMask = mask; }

		inline const tProximity& fCachedProximity( ) const { return mCachedQuery.fProximity( ); }
		inline const tEntityPtr& fStandingOn( ) const { return mStandingOn; }
		inline const Math::tVec3f& fSurfaceNormal( ) const { return mSurfaceNormal; }
		inline const Math::tVec3f& fPositionAtStartOfLastFall( ) const { return mPositionAtStartOfLastFall; }
		inline f32 fLastFallDistance( ) const { return mPositionAtStartOfLastFall.y - mTransform.fGetTranslation( ).y; } // negative values mean character 'fell' upward (i.e., a jump or whatever)
		inline f32 fLastFallTime( ) const { return mFallingTimer; }

		void					fJump( const Math::tVec3f& velocity, f32 minTime = 0.0f ) { mJumpVel = velocity; mInternalFlags = fSetBits( mInternalFlags, cFlagWantsJump ); mJumpTimer = minTime; }
		void					fForceFall( ) { fSetFlag( cFlagStartedFalling | cFlagFalling ); }

		f32						fGravity( ) const { return mGravity; }
		void					fSetGravity( f32 g ) { mGravity = g; }

	protected:
		f32				mStepHeight;
		f32				mExtentLength;
		f32				mExtraLength;
		f32				mDeprioritizedColTimer;
		f32				mGravity;
		Math::tVec3f	mJumpVel;
		f32				mJumpTimer;
		Math::tVec3f	mPositionAtStartOfLastFall;
		f32				mFallingTimer;

		u32				mSkipFrames;

		tEntityTagMask mCollisionMask;
		tEntityTagMask mQueryMask; 
		tEntityPtr mStandingOn;
		Math::tVec3f mSurfaceNormal;


		void fConfigureCachedQuery( );
		tMovingProximity mCachedQuery;
	};

}}

#endif//__tCharacterPhysics__
