#ifndef __tCharacterPhysics__
#define __tCharacterPhysics__
#include "Logic/tPhysical.hpp"
#include "tProximity.hpp"
#include "tRigidBody.hpp"
#include "tConstraint.hpp"

namespace Sig { namespace Physics
{
	class base_export tCharacterPhysics : public tStandardPhysics
	{
		debug_watch( tCharacterPhysics );
		declare_uncopyable( tCharacterPhysics );
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

	class tObstructionRecord
	{
		debug_watch( tObstructionRecord );
	public:
		tObstructionRecord( );
		tObstructionRecord( tRigidBody* );
		tObstructionRecord( const Math::tAabbf& );

		b32 fIsStaticObstructor( ) const;
		b32 fCollidesWithHypotheticalTranslation( tRigidBody& movingBody, const Math::tVec3f& testMove ); 
	private:
		b32 mIsStatic;
		tRigidBodyPtr mRigidBody;
		tGrowableArray< Math::tAabbf > mRigidBodyShapes;
		Math::tAabbf mStaticShape;

	};


	class tCharacterController : public tConstraint
	{
		debug_watch( tCharacterController );
		declare_uncopyable( tCharacterController );
		define_dynamic_cast( tCharacterController, tConstraint );
	public:
		tCharacterController( tRigidBody* body );

		// An easy way to get started.
		static tCharacterController* fCreateBasic( f32 height, f32 width, f32 extraGroundReach = 0.125f, tCollisionShape* shape = NULL, b32 raycastAlso = false );

		// Get this from an animation for example.
		const Math::tVec3f& fDesiredTranslation( ) const { return mDesiredTranslation; }
		void fSetDesiredTranslation( const Math::tVec3f& trans );
		void fSetFacing( const Math::tVec3f& facing );

		b32 fHasBody( ) const { return mBody.fGetRawPtr( ) != NULL; }
		tRigidBody& fBody( ) { return *mBody; }
		const tRigidBody& fBody( ) const { return *mBody; }

		// dont' call this every frame, only during big moves.
		void fSetTransform( const Math::tMat3f& xform );
		const Math::tMat3f& fTransform( ) const;
		const Math::tVec3f& fVelocity( ) const			{ return mBody->fVelocity( ); }
		f32 fGravity( ) const							{ return mBody->mGravity; }
		void fSetGravity( f32 gravity )					{ mBody->mGravity = gravity; }

		void fDisable( b32 disable );
		b32  fDisabled( ) const;
		tEntity* fStandingOn( ) const;
		Math::tVec3f fSurfaceNormal( ) const { return mSurfaceNormal; }

		void fJump( const Math::tVec3f& velocity );

		b32 fFalling( ) const { return mFalling; }
		b32 fObstructed( ) const { return mObstructed; }
		b32 fFallingLast( ) const { return mFallingLast; }
		b32 fStartedFalling( ) const { return fFalling( ) && !fFallingLast( ); }
		b32 fJustLanded( ) const { return !fFalling( ) && fFallingLast( ); }

		void fOnDelete( );

		void fDisableRecentObstructionTracking( );
		void fEnableRecentObstructionTracking( u32 numObstructionToTrack = 5 );
		tRingBuffer< tObstructionRecord >& fGetRecentObstructions( ) { return mRecentObstructions; };

	private:
		virtual void fSetWorld( tPhysicsWorld* world );
		virtual void fStepConstraintInternal( f32 dt, f32 percentage );



		void fSlideAndClampDesiredTranslationOnManifolds( );

		void fHandleSurfaceContact
			( const tPersistentContactManifold&	manifold
			, const tPersistentContactPt&		pt
			, const Math::tVec3f&				normal
			, f32								angle
			, b32								flipped );

		///\brief Returns true if any of the collisions were considered 'obstructing'
		b32 fHandleWallContact
			( const tPersistentContactPt&		pt
			, const Math::tVec3f&				normal
			, tGrowableArray<Math::tPlanef>&	contactPlanes );

		void fGatherRecentObstruction
			( b32								flipped
			, tPersistentContactManifold&		manifold );

		void fSlideAndClampDesiredTranslationOnPlanes( const tGrowableArray<Math::tPlanef>& planes );

		///\brief Returns true if mDesiredTranslation was actually clamped.
		b32 fSlideDesiredTranslationOnPlanesPass( u32 pass, const tGrowableArray<Math::tPlanef>& planes );

		///\brief Returns true if mDesiredTranslation was actually clamped.
		b32 fClampDesiredTranslationOnPlanesPass( u32 pass, const tGrowableArray<Math::tPlanef>& planes );



		tRigidBodyPtr mBody;
		Math::tVec3f mDesiredTranslation;

		tRefCounterPtr<tCollisionShapeRay> mRay;
		b32 mFalling;
		b32 mFallingLast;
		u32 mFallBuffer;
		b32 mObstructed;
		tRingBuffer< tObstructionRecord > mRecentObstructions;

		b32 mDisabled;
		tPhysicsWorld* mDisabledFromWorld;

		Math::tVec3f mSurfaceNormal;
		f32 mSurfaceMaxDepth;

		tCollisionShapePtr mStandingOn;
	};

	typedef tRefCounterPtr< tCharacterController > tCharacterControllerPtr;

}}

#endif//__tCharacterPhysics__
