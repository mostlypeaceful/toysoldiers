#ifndef __tAnimatedSkeleton__
#define __tAnimatedSkeleton__
#include "tAnimTrack.hpp"
#include "tResource.hpp"
#include "tLogicEvent.hpp"

namespace Sig { namespace Gfx
{
	class tDebugGeometryContainer;
}}

namespace Sig { namespace Anim
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
		tBoneProxy( tEntity& parent, const Math::tMat3f & bindPose, u32 boneIndex );
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
		inline tKeyFrameEvent( f32 t, f32 bs, u32 logicEventId, u32 keyFrameEventTypeCPPValue, const tStringPtr& tag ) 
			: mTime( t )
			, mBlendStrength( bs )
			, mLogicEventId( logicEventId )
			, mTag( tag )
			, mEventTypeCppValue( keyFrameEventTypeCPPValue )
		{ }
	};

	struct tKeyFrameEventContext : public Logic::tEventContext, public tKeyFrameEvent
	{
		debug_watch( tKeyFrameEventContext );
		define_class_pool_new_delete( tKeyFrameEventContext, 64 );
		define_dynamic_cast( tKeyFrameEventContext, Logic::tEventContext );
	public:
		explicit tKeyFrameEventContext( const tKeyFrameEvent& kfe ) : tKeyFrameEvent( kfe ) { }

		static tKeyFrameEventContext* fConvert( const tEventContext* obj )
		{
			return obj ? obj->fDynamicCast< tKeyFrameEventContext >( ) : NULL;
		}
		static void fExportScriptInterface( tScriptVm& vm );
	};

	struct tAnimCommand;

	struct tAnimCommandBuffer : public tGrowableArray< tAnimCommand >
	{
		void fGeneratePushCmd( const tAnimTrackPtr& animTrack );
		void fGenerateUpdateCmd( const tAnimTrackPtr& animTrack );

		static tAnimTrackPtr fExecutePushCmd( const tAnimCommand& animCmd );
		static void fExecuteUpdateCmd( const tAnimCommand& animCmd, const tAnimTrackPtr& animTrack );

		template< class tArchive >
		void fSaveLoad( tArchive & archive )
		{
			archive.fSaveLoad( ( tGrowableArray< tAnimCommand >& )*this );
		}
	};

	struct tAnimCommand
	{
		enum tId
		{
			cIdPushKeyFrameTrack,
			cIdPushBlendTrack,
			cIdUpdateTrackData,
			cIdInvalid,
		};

		tAnimCommand( ) : mId( cIdInvalid ), animIndex( 0 ) { }

		u8 mId;
		tAnimTrackDesc trackDesc;
		tFilePathPtr packFileName;
		u8 animIndex;
		tAnimCommandBuffer childCommands;

		tAnimTrackData trackData;

		b32 operator==( const tAnimCommand& rhs ) const;
		b32 operator!=( const tAnimCommand& rhs ) const { return !operator==( rhs ); }

		template< class tArchive >
		void fSaveLoad( tArchive & archive )
		{
			archive.fSaveLoad( mId );

			switch( mId )
			{
			case cIdPushKeyFrameTrack:
				archive.fSaveLoad( packFileName );
				archive.fSaveLoad( animIndex );
				// Intentional fall-through

			case cIdPushBlendTrack:
				archive.fSaveLoad( trackDesc );
				archive.fSaveLoad( trackData );

				break;
			case cIdUpdateTrackData:
				archive.fSaveLoad( trackData );

				break;
			default:
				sigassert( "Invalid anim command" );
			}

			archive.fSaveLoad( childCommands );
		}
	};


	// kind of like an entity, but maybe just multiply-referenced?
	class base_export tAnimatedSkeleton : public tUncopyable, public tRefCounter
	{
		debug_watch( tAnimatedSkeleton );
	public:
		typedef tDynamicArray<Math::tMat3f> tMatrixPalette;
		typedef tGrowableArray< tAnimTrackPtr > tAnimTrackStack;
		typedef tGrowableArray< tLogic* > tEventListenerList;
		typedef tGrowableArray< tBoneProxy > tBoneProxyList;
		typedef tGrowableArray< tKeyFrameEvent > tKeyFrameEventList;

		static const u32 cMaxAnimCommandCount = 10;
	private:
		tResourcePtr			mSkeleton;
		tAnimTrackStack			mAnimTracks;
		tAnimTrackStack			mPostAnimTracks; //IK
		Math::tPRSXformf		mRefFrameDelta;
		tAnimEvaluationResult	mEvaluationResult;
		tMatrixPalette			mMatrixPalette;
		Math::tMat3f			mBindOffset, mBindOffsetInverse;
		Math::tVec3f			mBindScale;
		tEventListenerList		mEventListeners;
		tBoneProxyList			mBoneProxies;
		tKeyFrameEventList		mKeyFrameEvents;
		f32						mTimeScale;
		tEntityPtr				mOwner;
		b16						mEvaluationEnabled;
		b16						mEnableAnimCommandBuffer;
		tAnimCommandBuffer		mAnimCommandBuffer;
	public:
		static b32 fIsReachedEndOfOneShotEvent( const Logic::tEvent& event );
	public:
		explicit tAnimatedSkeleton( const tResourcePtr& skeletonRes, tEntity& owner );
		~tAnimatedSkeleton( );

		b32 fEvaluationEnabled( ) const { return mEvaluationEnabled; }
		void fEnableDisableEvaluation( b32 enabled ) { mEvaluationEnabled = enabled; }

		void fOnDelete( );
		void fReleaseOwner( ) { mOwner.fRelease( ); } // Called by on delete. But you can call it too if you want. :)

		void fPushTrack( const tAnimTrackPtr& animTrack );
		void fPushTrackBeforeTag( const tAnimTrackPtr& animTrack, const tStringPtr& tag );
		void fPushPostTrack( const tAnimTrackPtr& animTrack );

		const tEntityPtr& fOwner( ) const { return mOwner; }

		///
		/// \brief Only call in single-threaded apps, i.e., if manually stepping an animated skeleton apart from a scene graph.
		void fStep( f32 dt );

		///
		/// \brief In MT with jobs and scene graph, this method will be called by animation jobs.
		void fStepMT( f32 dt );

		///
		/// \brief This method should be called in main thread.
		void fStepST( f32 dt );

	private:
		void fEvaluate( );
		void fCullDeadTracks( );
		void fStepBoneProxiesMT( );
		void fStepBoneProxiesST( );
	public:
		b32  fApplyRefFrameDelta( Math::tMat3f& xform, f32 scale = 1.f ) const; // returns false if there was no movement

		tAnimTrackPtr fFirstTrackWithTag( const tStringPtr& tag ) const;
		tAnimTrackPtr fLastTrackWithTag( const tStringPtr& tag ) const;

		b32 fHasTrackWithTag( const tStringPtr& tag ) const;
		void fRemoveTrack( u32 index );
		void fRemovePostTrack( u32 index );
		void fRemoveTracksWithTag( const tStringPtr& tag );
		void fBlendOutTracksWithTag( const tStringPtr& tag );

		template< typename T >
		void fRemoveTracksOfType( )
		{
			for( u32 i = 0; i < mAnimTracks.fCount( ); )
			{
				if( mAnimTracks[ i ]->fDynamicCast< T >( ) ) 
					fRemoveTrack( i );
				else 
					++i;
			}
		}
		template< typename T >
		void fRemovePostTracksOfType( )
		{
			for( u32 i = 0; i < mPostAnimTracks.fCount( ); )
			{
				if( mPostAnimTracks[ i ]->fDynamicCast< T >( ) ) 
					fRemovePostTrack( i );
				else 
					++i;
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

		template< typename T >
		T* fGetTopTrackOfType( ) const
		{
			for( s32 i = mAnimTracks.fCount( ) - 1; i >= 0; --i )
				if( T* track = mAnimTracks[ i ]->fDynamicCast< T >( ) )
					return track;
			return 0;
		}

		void fClearTracks( );
		void fClearBelowTag( const tStringPtr& tag );
		void fClearPostTracks( ) { mPostAnimTracks.fSetCount( 0 ); }
		void fSetToIdentity( );
		void fAddEventListener( tLogic& logic );
		void fRemoveEventListener( tLogic& logic );
		void fQueueEvent( const tKeyFrameEvent& kfe ) { mKeyFrameEvents.fPushBack( kfe ); }
		void fFireEvents( );
		void fAddBoneProxy( tEntity& parent, const tStringPtr& boneName, b32 boneRelative );
		void fRemoveBoneProxy( tEntity& parent, const tStringPtr& boneName );
		void fDeleteBoneProxies( );
		void fSetBindOffset( const Math::tMat3f& bindOffset, const Math::tMat3f& bindOffsetInverse ) { mBindOffset = bindOffset; mBindOffsetInverse = bindOffsetInverse; mBindScale = mBindOffset.fGetScale( ); }
		const Math::tMat3f& fBindOffset( ) const { return mBindOffset; }
		const Math::tMat3f& fBindOffsetInverse( ) const { return mBindOffsetInverse; }
		void fSetTimeScale( f32 timeScale ) { mTimeScale = timeScale; }

		void fEnableAnimCommandBuffer( b32 enable );
		void fGeneratePushCommandsForAllTracks( tAnimCommandBuffer& animCommandBuffer );
		void fGenerateUpdateCommands( tAnimCommandBuffer& animCommandBuffer );
		void fExecuteAnimCommands( const tAnimCommandBuffer& animCommandBuffer );
	public: // accessors
		tAnimTrackStack& fTracks( ) { return mAnimTracks; }

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

		b32 fEnableAnimCommandBuffer( ) const { return mEnableAnimCommandBuffer; }
		tAnimCommandBuffer& fAnimCommandBuffer( ) { return mAnimCommandBuffer; }
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
		if_devmenu( void fAddDebugText( std::stringstream& ss ) const );
	private:
		u32 fHighestFullyBlendedTrack( ) const;
		u32 fHighestStackClearingTrack( ) const;
		u32 fHighestNonPartialTrack( ) const;
		void fPushAnimTrack( tAnimTrackStack & stack, const tAnimTrackPtr & track );

		void fConstructBoneLines( s32 boneIndex, tBoneLine& boneLinesRoot, const Math::tMat3f& objectToWorld ) const;
		void fRenderDebug( Gfx::tDebugGeometryContainer& debugGeometry, const tBoneLine& root, const Math::tVec4f& rgbaTint ) const;

	public: // script-specific
		static void	fExportScriptInterface( tScriptVm& vm );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tAnimatedSkeleton );

} }

#endif//__tAnimatedSkeleton__

