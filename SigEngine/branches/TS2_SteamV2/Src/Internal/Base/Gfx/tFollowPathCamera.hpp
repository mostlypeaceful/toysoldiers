#ifndef __tFollowPathCamera__
#define __tFollowPathCamera__
#include "tCameraController.hpp"
#include "tPathEntity.hpp"

namespace Sig { namespace Gfx
{

	class tCameraControllerStack;

	class tFollowPathCameraControlParams
	{
	public:
		f32 mTotalTime; // in seconds
		f32 mEaseIn; // [0,1], 0 is no ease in, 1 means reach full speed at half way point
		f32 mEaseOut; // [0,1] same concept as ease in
		f32 mGameBlendIn; // in seconds, amount of time the game cam takes to blend in
		tStringPtr mLookAtTarget;

		tFollowPathCameraControlParams( )
			: mTotalTime( 1.f )
			, mEaseIn( 0.2f )
			, mEaseOut( 0.2f )
			, mGameBlendIn( 2 )
		{ }
	};

	class tFollowPathCameraPointLogic : public tLogic
	{
		define_dynamic_cast( tFollowPathCameraPointLogic, tLogic );
	public:
		tFollowPathCameraControlParams mParams;

		tFollowPathCameraControlParams* fParamsForScript( ) { return &mParams; }

		static void fExportScriptInterface( tScriptVm& vm );
	};

	///
	/// \brief Camera follows a specified path
	class tFollowPathCamera : public tCameraController
	{
		define_dynamic_cast( tFollowPathCamera, tCameraController );
	public:
		struct tControlPoint
		{
			Math::tPRSXformf	mXform;
			f32					mSegmentLength;
			tPathEntityPtr		mPathPoint;
			tControlPoint( ) : mXform( Math::tPRSXformf::cIdentity ), mSegmentLength( 0.f ) { }
			tControlPoint( const Math::tMat3f& xform ) : mXform( xform ), mSegmentLength( 0.f ) { }
		};
		class tControlPointList : public tGrowableArray<tControlPoint>
		{
		public:
			f32 mTotalLength; // in world units
			tFollowPathCameraControlParams mParams;
			b32 mParamsSet;
		public:
			tControlPointList( );
			void fSeal( ); // after all points have been added, should call this
			f32 fComputeFractionOfPath( u32 ithSegment );
			f32 fComputeSegmentDuration( u32 ithSegment );
		};
		typedef tDelegate< void ( tFollowPathCamera& camera ) > tOnEndOfPathReached;
		typedef tDelegate< b32 ( tFollowPathCamera& camera, tUserPtr& user ) > tSkipButtonHeldFunc;

	protected:
		tOnEndOfPathReached mOnEndOfPathReached;
		tControlPointList mControlPoints;
		Math::tPRSXformf mPre, mPost;
		u32 mCurrentCp;
		f32 mTimeToReachWayPoint;
		f32 mTimer;
		f32 mEntirePathPos; // in [0,1]
		b32 mSkipped;
		tStringPtr mKey; //a unique identifier for this camera follower.
		tCameraControllerStack* mStack;
		tSkipButtonHeldFunc mSkipButtonHeldFunc;
		
		struct tFilteredPad
		{
			tUserPtr mUser;
			u32 mFilter;
		};

		tGrowableArray< tFilteredPad > mGamepads;

		Gfx::tTripod mTripod; 
	public:
		static void fBuildControlPointList( tControlPointList& cps, tEntity& root, Math::tVec3f targetPt, tPathEntity& firstWaypoint );
	public:
		tFollowPathCamera( const tStringPtr& key, const tSkipButtonHeldFunc& skipButtonHeldFunc, const tUserPtr& user, tCameraControllerStack* stack, const tLens& lens, const tControlPointList& cps, const tOnEndOfPathReached& callAtEndOfPath );
		~tFollowPathCamera( );
		
		virtual void fOnTick( f32 dt );
		const tStringPtr& fKey( ) const { return mKey; }
		b32 fSkipped( ) const { return mSkipped; }
		const tControlPointList& fControlPoints( ) const { return mControlPoints; }
		void fCutToEnd( );
		tCameraControllerStack* fStack( ) const { return mStack; }
		void fAddGamePad( const tUserPtr& user );
		const Gfx::tTripod& fStepSlave( f32 dt );

	protected:
		virtual f32		fPathSpeedMultiplier( f32 dt ) { return 1.f; }
		virtual void	fArrivedAtNextWaypoint( ) { }
		void			fSetTripod( u32 cp, f32 t );
		void			fSetCamera( );
		void			fOnTickInternal( f32 dt );
	};


}}

#endif//__tFollowPathCamera__

