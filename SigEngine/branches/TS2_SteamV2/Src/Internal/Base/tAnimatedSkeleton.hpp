#ifndef __tAnimatedSkeleton__
#define __tAnimatedSkeleton__
#include "tAnimTrack.hpp"
#include "tResource.hpp"

namespace Sig { namespace Gfx
{
	class tDebugGeometryContainer;
}}

namespace Sig
{
	class tAnimatedSkeleton;

	class base_export tBoneProxy // : public tUncopyable, public tRefCounter
	{
		//define_class_pool_new_delete( tBoneProxy, 64 );
	private:
		Math::tMat3f			mBindPose;
		Math::tMat3f			mParentRelativeOg;
		Math::tMat3f			mParentRelativeCur;
		tEntityPtr				mParent;
		u32						mBoneIndex;
	public:
		tBoneProxy( ) { }
		tBoneProxy( tEntity& parent, tAnimatedSkeleton& skeleton, u32 boneIndex );
		void fRecomputeParentRelative( const tAnimatedSkeleton& skeleton );
		void fUpdateParent( ) const;
		void fResetParent( ) const;
		const tEntityPtr& fParent( ) const { return mParent; }
		inline b32 fIsMatch( tEntity& parent, u32 boneIndex ) const { return &parent == mParent && boneIndex == mBoneIndex; }
	};

	//define_smart_ptr( base_export, tRefCounterPtr, tBoneProxy );

	struct tKeyFrameEvent
	{
		f32 mTime, mBlendStrength;
		u32 mLogicEventId;
		tStringPtr mTag;
		u32 mEventTypeCppValue;
		inline tKeyFrameEvent( ) { }
		inline tKeyFrameEvent( f32 t, f32 bs, u32 logicEventId, u32 keyFrameEventTypeCPPValue, const tStringPtr& tag ) : mTime( t ), mBlendStrength( bs ), mLogicEventId( logicEventId ), mTag( tag ), mEventTypeCppValue( keyFrameEventTypeCPPValue ) { }
	};
	struct tKeyFrameEventContext : public Logic::tEventContext, public tKeyFrameEvent
	{
		define_class_pool_new_delete( tKeyFrameEventContext, 64 );
		define_dynamic_cast( tKeyFrameEventContext, Logic::tEventContext );
	public:
		explicit tKeyFrameEventContext( const tKeyFrameEvent& kfe ) : tKeyFrameEvent( kfe ) { }
	};

	// kind of like an entity, but maybe just multiply-referenced?
	class base_export tAnimatedSkeleton : public tUncopyable, public tRefCounter
	{
	public:
		typedef tDynamicArray<Math::tMat3f> tMatrixPalette;
		typedef tGrowableArray< tAnimTrackPtr > tAnimTrackStack;
		typedef tGrowableArray< tLogic* > tEventListenerList;
		typedef tGrowableArray< tBoneProxy > tBoneProxyList;
		typedef tGrowableArray< tKeyFrameEvent > tKeyFrameEventList;
	private:
		tResourcePtr			mSkeleton;
		tAnimTrackStack			mAnimTracks;
		tAnimTrackStack			mPostAnimTracks; //IK
		Math::tPRSXformf		mRefFrameDelta;
		tAnimEvaluationResult	mEvaluationResult;
		tMatrixPalette			mMatrixPalette;
		Math::tMat3f			mBindOffset, mBindOffsetInverse;
		tEventListenerList		mEventListeners;
		tBoneProxyList			mBoneProxies;
		tKeyFrameEventList		mKeyFrameEvents;
		f32						mTimeScale;
	public:
		static b32 fIsReachedEndOfOneShotEvent( const Logic::tEvent& event );
	public:
		explicit tAnimatedSkeleton( const tResourcePtr& skeletonRes );
		~tAnimatedSkeleton( );
		void fPushTrack( const tAnimTrackPtr& animTrack );
		void fPushTrackBeforeTag( const tAnimTrackPtr& animTrack, const tStringPtr& tag );
		void fPushPostTrack( const tAnimTrackPtr& animTrack );


		///
		/// \brief Only call in single-threaded apps, i.e., if manually stepping an animated skeleton apart from a scene graph.
		void fStep( f32 dt );

		///
		/// \brief In MT with jobs and scene graph, this method will be called by animation jobs.
		void fStepMTInJobs( f32 dt );

		///
		/// \brief In MT with jobs and scene graph, this method should be called in main thread.
		void fStepMTMainThread( );

		void fEvaluate( ); // called automatically from fStep, so don't call needlessly
		void fCullDeadTracks( );
	private:
		void fStepBoneProxiesMTInJobs( );
		void fStepBoneProxiesMTMainThread( );
	public:
		b32  fApplyRefFrameDelta( Math::tMat3f& xform, f32 scale = 1.f ) const; // returns false if there was no movement
		f32  fComputeLinearSpeed( const Math::tMat3f& objToWorld, f32 scale = 1.f ) const;
		tAnimTrackPtr fFirstTrackWithTag( const tStringPtr& tag ) const;
		tAnimTrackPtr fLastTrackWithTag( const tStringPtr& tag ) const;
		b32 fHasTrackWithTag( const tStringPtr& tag ) const;
		void fRemoveTrack( u32 index );
		void fRemoveTracksWithTag( const tStringPtr& tag );
		void fBlendOutTracksWithTag( const tStringPtr& tag );
		template< typename T >
		void fRemoveTracksOfType( )
		{
			for( u32 i = 0; i < mAnimTracks.fCount( ); )
			{
				if( mAnimTracks[ i ]->fDynamicCast< T >( ) ) fRemoveTrack( i );
				else ++i;
			}
		}
		template< typename T >
		b32 fHasTracksOfType( ) const
		{
			for( u32 i = 0; i < mAnimTracks.fCount( ); ++i)
				if( mAnimTracks[ i ]->fDynamicCast< T >( ) ) 
					return true;
			return false;
		}

		void fClearTracks( );
		void fClearBelowTag( const tStringPtr& tag );
		void fSetToIdentity( );
		void fAddEventListener( tLogic& logic );
		void fRemoveEventListener( tLogic& logic );
		void fClearResponseToEndEvent( );
		void fQueueEvent( const tKeyFrameEvent& kfe ) { mKeyFrameEvents.fPushBack( kfe ); }
		void fFireEvents( );
		void fAddBoneProxy( tEntity& parent, const tStringPtr& boneName );
		void fRemoveBoneProxy( tEntity& parent, const tStringPtr& boneName );
		void fDeleteBoneProxies( );
		void fSetBindOffset( const Math::tMat3f& bindOffset, const Math::tMat3f& bindOffsetInverse ) { mBindOffset = bindOffset; mBindOffsetInverse = bindOffsetInverse; }
		const Math::tMat3f& fBindOffset( ) const { return mBindOffset; }
		const Math::tMat3f& fBindOffsetInverse( ) const { return mBindOffsetInverse; }
		void fSetTimeScale( f32 timeScale ) { mTimeScale = timeScale; }
	public: // accessors
		u32 fTrackCount( ) const { return mAnimTracks.fCount( ); }
		const tAnimTrack& fTrack( u32 ithTrack ) const { return *mAnimTracks[ ithTrack ]; }
		tAnimTrack& fTrack( u32 ithTrack ) { return *mAnimTracks[ ithTrack ]; }
		tAnimTrackStack& fPostTracks( ) { return mPostAnimTracks; }

		u32 fEventListenerCount( ) const { return mEventListeners.fCount( ); }
		const tResourcePtr& fSkeletonResource( ) const { return mSkeleton; }
		const Math::tPRSXformf& fRefFrameDelta( ) const { return mRefFrameDelta; }
		tMatrixPalette& fMatrixPalette( ) { return mMatrixPalette; }
		const tMatrixPalette& fMatrixPalette( ) const { return mMatrixPalette; }
		tBoneProxyList::tConstIterator fBoneProxiesBegin( ) const { return mBoneProxies.fBegin( ); }
		tBoneProxyList::tConstIterator fBoneProxiesEnd( ) const { return mBoneProxies.fEnd( ); }
		f32 fTimeScale( ) const { return mTimeScale; }
	public: // debuggish methods
		struct tBoneLine
		{
			tBoneLine( ) : mXform( Math::tMat3f::cIdentity ) { }
			Math::tMat3f				mXform;
			tStringPtr					mName;
			tDynamicArray<tBoneLine>	mChildren;
		};
		void fConstructBoneLines( tBoneLine& boneLinesRoot, const Math::tMat3f& objectToWorld ) const;
		void fRenderDebug( Gfx::tDebugGeometryContainer& debugGeometry, const Math::tMat3f& objectToWorld, const Math::tVec4f& rgbaTint ) const;
	private:
		u32 fHighestFullyBlendedTrack( ) const;
		u32 fHighestStackClearingTrack( ) const;
		void fPushAnimTrack( tAnimTrackStack & stack, const tAnimTrackPtr & track );

		void fConstructBoneLines( s32 boneIndex, tBoneLine& boneLinesRoot, const Math::tMat3f& objectToWorld ) const;
		void fRenderDebug( Gfx::tDebugGeometryContainer& debugGeometry, const tBoneLine& root, const Math::tVec4f& rgbaTint ) const;

	public: // script-specific
		static void	fExportScriptInterface( tScriptVm& vm );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tAnimatedSkeleton );

}

#endif//__tAnimatedSkeleton__

