//------------------------------------------------------------------------------
// \file tSkeletableSgFileRefEntity.hpp - 23 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tSkeletableSgFileRefEntity__
#define __tSkeletableSgFileRefEntity__

#include "tAnimatedSkeleton.hpp"
#include "Editor/tEditableSgFileRefEntity.hpp"
#include "tSkeletonVisualizer.hpp"

namespace Sig
{
	struct toolsgui_export tAnimPackInfo
	{
		// NOTE: If the number of skeletons and animPacks gets large enough
		// we may wish to switch these arrays to a sorted container
		tGrowableArray<tResourcePtr> mSkeletonsToProcess;
		tGrowableArray<tResourcePtr> mSkeletonsProcessing;
		tGrowableArray<tFilePathPtr> mSkeletonsProcessed;

		tGrowableArray<tResourcePtr> mAnimPacks;
	};

	namespace Anim
	{
		class tSigAnimMoMap;
	}

	//------------------------------------------------------------------------------
	// tSkeletableSgFileRefEntity
	//------------------------------------------------------------------------------
	class toolsgui_export tSkeletableSgFileRefEntity : public tEditableSgFileRefEntity
	{
		define_dynamic_cast( tSkeletableSgFileRefEntity, tEditableSgFileRefEntity );

	public:

		tSkeletableSgFileRefEntity( 
			tEditableObjectContainer & container, 
			const tResourceId & rid,
			const tResourcePtr & sigbResource );

		tSkeletableSgFileRefEntity( 
			tEditableObjectContainer& container, 
			const tResourceId & rid,
			const tResourcePtr & sigbResource,
			const tResourcePtr & skelResource );

		virtual ~tSkeletableSgFileRefEntity( );

		void fSetSkeletonResource( const tResourcePtr & skelRes );
		const tResourcePtr & fSkeletonResource( ) const { return mSkelResource; }
		tFilePathPtr fSkeletonResourcePath( ) const { 
			return mSkelResource.fNull( ) ? tFilePathPtr( ) : mSkelResource->fGetPath( ) ; }
		b32 fSkeletonResIsFromSgFile( ) const;

		tAnimPackInfo & fAnimPackInfo( ) { return mAnimPackInfo; }

		void fAddAnimPack( const tResourcePtr & pack );
		virtual tEntityPtr fClone( );

		const Anim::tAnimatedSkeletonPtr& fSkeleton( ) const { return mSkeleton; }

		b32 fIsCursor ( ) const { return mIsCursor; }
		void fSetIsCursor( b32 isCursor ) { mIsCursor = isCursor; }

		b32 fPaused( )const { return mIsPaused; }
		void fSetPaused( b32 pause = true ) { mIsPaused = pause; }

		b32 fReverseTime( ) const { return mTimeReversed; }
		void fSetReverseTime( b32 reverse ) { mTimeReversed = reverse; }

		b32 fApplyRefFrame( ) const { return mApplyRefFrame; }
		void fSetApplyRefFrame( b32 apply ) { mApplyRefFrame = apply; }

		b32 fRenderSkeleton( ) const { return mRenderSkeleton; }
		void fSetRenderSkeleton( b32 render ) { mRenderSkeleton = render; }
		tSkeletonVisualizer * fGetRenderSkeleton( ) 
			{ return mRenderSkeleton ? &mRenderableSkeleton : 0; }
		
		f32 fTimeScale( ) const { return mTimeScale; }
		void fSetTimeScale( float scale ) { mTimeScale = scale; }

		f32 fMotionRange( ) const { return mMotionRange; }
		void fSetMotionRange( f32 range ) { mMotionRange = range; }

		void fForceStep( f32 step ) { mForceStep = step; fSetPaused( ); }

		void fEnableSkeletonVisualizer( );
		void fDisableSkeletonVisualizer( );

		void fResetBaseXform( ) { mBaseXform = fObjectToWorld( ); }

		static b32 fIsCursor( tSkeletableSgFileRefEntity * entity )
		{
			return entity->fIsCursor( );
		}

		void fSetMotionMap( Anim::tSigAnimMoMap* moMap );

	protected:

		virtual void fOnSpawn( );
		virtual void fOnMoved( b32 recomputeParentRelative );
		virtual void fThinkST( f32 dt );

	private:
		
		void fCommonConstruct( );
		void fOnSkelLoaded( tResource& theResource, b32 success );
		void fOnResLoaded( tResource & theResource, b32 success );

	private:
		
		Math::tMat3f mBaseXform;
		tResourcePtr mSkelResource;
		Anim::tAnimatedSkeletonPtr mSkeleton;
		tResource::tOnLoadComplete::tObserver  mOnSkelResourceLoaded;
		tResource::tOnLoadComplete::tObserver  mOnResourceLoaded;
		tAnimPackInfo mAnimPackInfo;

		tSkeletonVisualizer mRenderableSkeleton;
		tRefCounterPtr<Anim::tSigAnimMoMap> mMoMap;

		b32 mIsCursor;
		b32 mIsPaused;
		f32 mForceStep;
		f32 mTimeScale;
		b32 mTimeReversed;
		b32 mApplyRefFrame;
		b32 mMotionRange;
		b32 mRenderSkeleton;
		b32 mUpdatingXform;
	};
}

#endif//__tSkeletableSgFileRefEntity__
