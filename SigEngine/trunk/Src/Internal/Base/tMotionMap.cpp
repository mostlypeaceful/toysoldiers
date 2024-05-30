#include "BasePch.hpp"
#include "tMotionMap.hpp"
#include "tSceneRefEntity.hpp"


namespace Sig { namespace Anim
{
	Sqrat::Function tScriptMotionMap::fMapState( const char* stateName )
	{ 
		log_assert( !fScriptObject( ).IsNull( ), "No MotionMap when looking for MotionState: " << stateName );
		Sqrat::Function f = Sqrat::Function( fScriptObject( ), stateName );
		if( f.IsNull( ) )
		{
			log_warning( "The motion state [" << stateName << "] is not present in the MoMap script." );
			tScriptVm::fDumpCallstack( );
		}
		return f;
	}
} }


#include "tApplication.hpp"
#include "tAnimPackFile.hpp"
namespace Sig { namespace Anim
{
	namespace
	{
		static tAnimPackFile* fGetAnimPack( const tMotionMap* momap, const tFilePathPtr& path )
		{
			const tResourcePtr res = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake<tAnimPackFile>( tAnimPackFile::fAnipkPathToAnib( path ) ) );
			if( res->fLoaded( ) )
				return res->fCast<tAnimPackFile>( );
			else
			{
				log_warning( "AnimPack does not exist: " << path );
				return NULL;
			}
		}
	}

	tAnimatedSkeleton* tMotionMap::fAnimationStack( ) const
	{
#ifdef sig_devmenu
		if( !mStack )
		{
			tSceneRefEntity* ref = mLogic && mLogic->fOwnerEntity( ) ? mLogic->fOwnerEntity( )->fDynamicCast<tSceneRefEntity>( ) : 0;
			log_warning( "No animation stack for: " << (mLogic ? mLogic->fDebugTypeName( ) : "") << " res: " << ( ref ? ref->fSgResource( )->fGetPath( ).fCStr( ) : "{UNKNOWN LOGIC/ENT}" ) );
			sigassert( mStack );
		}
#endif//sig_devmenu
		return mStack.fGetRawPtr( );
	}

	void tMotionMap::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tMotionMap, Sqrat::NoCopy<tMotionMap> > classDesc( vm.fSq( ) );

		classDesc
			.GlobalFunc(_SC("GetAnimPack"),		&fGetAnimPack)
			.Prop(_SC("Stack"),					&tMotionMap::fAnimationStack)
			.Prop(_SC("Logic"),					&tMotionMap::fGetLogicForScript)
			;

		vm.fNamespace(_SC("Anim")).Bind(_SC("MotionMap"), classDesc);
	}


	tSigAnimMoMap::tBlendData::tBlendData( const tStringPtr& name, b32 digital, b32 oneShot )
		: mName( name )
		, mCurrentBlendValue( 0.f )
		, mDigital( digital )
		, mOneShot( oneShot )
		, mBlendControl( NULL )
		, mTimeScaleControl( NULL )
	{ }
	
	void tSigAnimMoMap::tBlendData::fAddTrack( tBlendTrack* track ) 
	{ 
		mBlends.fPushBack( track ); 
	}

	void tSigAnimMoMap::tBlendData::fSetBlend( f32 blend )
	{
		mCurrentBlendValue = blend;
		for( u32 i = 0; i < mBlends.fCount( ); ++i )
			mBlends[ i ]->fSetBlend( blend );
	}

	void tSigAnimMoMap::tBlendData::fSetTimeScale( f32 scale )
	{
		for( u32 i = 0; i < mBlends.fCount( ); ++i )
			mBlends[ i ]->fSetTimeScale( scale );
	}

	void tSigAnimMoMap::tBlendData::fAddReference( tBlendReference* ref )
	{
		sigassert( mDigital );
		sigassert( !mReferences.fFind( ref ) );
		mReferences.fPushBack( ref );

		const b32 enable = mReferences.fCount( );
		for( u32 i = 0; i < mBlends.fCount( ); ++i )
			mBlends[ i ]->fEnableDigitalBlend( enable );
	}

	void tSigAnimMoMap::tBlendData::fRemoveReference( tBlendReference* ref )
	{
		sigassert( mDigital );
		b32 found = mReferences.fFindAndErase( ref );
		//sigassert( found );

		const b32 enable = mReferences.fCount( );
		for( u32 i = 0; i < mBlends.fCount( ); ++i )
			mBlends[ i ]->fEnableDigitalBlend( enable );
	}

	void tSigAnimMoMap::tBlendData::fStep( f32 dt )
	{
		if( mBlendControl )
			fSetBlend( *mBlendControl );

		if( mTimeScaleControl )
			fSetTimeScale( *mTimeScaleControl );

		for( u32 i = 0; i < mBlends.fCount( ); ++i )
			mBlends[ i ]->fStep( dt );
	}

	void tSigAnimMoMap::tBlendData::fAnimEnded( )
	{
		if( mOneShot )
			fInformRefsAnimEnded( );
	}

	void tSigAnimMoMap::tBlendData::fInformRefsAnimEnded( )
	{
		// one shot ended, inform refs.
		for( u32 i = 0; i < mReferences.fCount( ); ++i )
			mReferences[ i ]->fAnimEnded( );
	}

	tSigAnimMoMap::tBlendTrack::tBlendTrack( tBlendAnimTrack* track, tMotionMapFile::tBlendTrackData* def, tBlendData* owner, tBlendTrack* parent )
		: mTrack( track )
		, mBlendDef( def )
		, mDigitalBlendEnabled( false )
		, mThresholdTriggered( false )
		, mDigitalDirty( true )
		, mCurrentBlend( 0.f )
		, mAVisible( false )
		, mBVisible( false )
		, mOwner( owner )
		, mParent( parent )
	{ 
	}

	void tSigAnimMoMap::tBlendTrack::fEnableDigitalBlend( b32 enable )
	{
		if( enable != mDigitalBlendEnabled )
			mDigitalDirty = true;

		mDigitalBlendEnabled = enable;
	}

	void tSigAnimMoMap::tBlendTrack::fSetBlend( f32 blend )
	{
		f32 target = blend;

		if( mBlendDef->mDigital )
		{
			const b32 triggered = blend > mBlendDef->mDigitalThresold;
			if( triggered != mThresholdTriggered )
			{
				mThresholdTriggered = triggered;
				mDigitalDirty = true;
			}
		}
		else
		{
			mCurrentBlend = blend;
			fApplyBlend( );
		}
	}

	void tSigAnimMoMap::tBlendTrack::fStep( f32 dt )
	{
		if( mBlendDef->mDigital && mDigitalDirty )
		{
			const f32 cBlendTime = 0.2f;
			const f32 cMaxDelta = 1.0f / cBlendTime * dt;

			const b32 blendIn = (mThresholdTriggered || mDigitalBlendEnabled);
			const f32 target = blendIn ? 1.f : 0.f;
			const f32 delta = fClamp( target - mCurrentBlend, -cMaxDelta, cMaxDelta );

			mCurrentBlend += delta;
			fApplyBlend( );

			const b32 complete = fEqual( mCurrentBlend, target );
			if( complete )
			{
				//if( mBlendDef->mOneShot && !blendIn )
				//	fInformRefsAnimEnded( );

				mDigitalDirty = false;
			}
		}
	}

	void tSigAnimMoMap::tBlendTrack::fApplyBlend( )
	{
		f32 mABRange = mBlendDef->mBCurve - mBlendDef->mACurve;
		f32 mVal = (mCurrentBlend - mBlendDef->mACurve) / mABRange;
		f32 blendFactor = fClamp( mVal, 0.f, 1.f );

		const f32 aBlend = 1.0f; 
		const f32 bBlend = blendFactor;

		b32 aVisible = aBlend > 0.f;
		b32 bVisible = bBlend > 0.f;

		if( mAVisible != aVisible ) 
		{
			fTrackVisibilityChanged( 0, aVisible );
			mAVisible = aVisible;
		}

		if( mBVisible != bVisible ) 
		{
			fTrackVisibilityChanged( 1, bVisible );
			mBVisible = bVisible;
		}

		mTrack->fSubTracks( )[ 0 ]->fSetBlendScale( aBlend );
		mTrack->fSubTracks( )[ 1 ]->fSetBlendScale( bBlend );
	}

	void tSigAnimMoMap::tBlendTrack::fSetTimeScale( f32 scale )
	{
		mTrack->fSubTracks( )[ 0 ]->fSetTimeScale( scale );
		mTrack->fSubTracks( )[ 1 ]->fSetTimeScale( scale );
	}

	void tSigAnimMoMap::tBlendTrack::fTrackVisibilityChanged( u32 logicalIndex, b32 visible )
	{
		if( visible && mOwner->fOneShot( ) )
			mTrack->fSubTracks( )[ logicalIndex ]->fRestart( );
	}

	void tSigAnimMoMap::tBlendTrack::fAnimEnded( )
	{
		mOwner->fAnimEnded( );

		// Propagate message to the root.
		if( mParent )
			mParent->fAnimEnded( );
	}

	namespace
	{
		/*
			Recursively evaluate Animap state, context tree.

			 If "matchingData" is set. this means you are pursuing a context switch.
		*/
		const tAniMapFile::tAnimRef* fFindAnim( const tAniMapFile::tContextSwitch& contextTree, const tGrowableArray< tSigAnimMoMap::tContextData >& context, const tSigAnimMoMap::tContextData* matchingData )
		{
			if( !matchingData )
			{
				// this node is eligible for context switches.
				for( u32 i = 0; i < contextTree.mBranches.fCount( ); ++i )
				{
					const tAniMapFile::tContextSwitch& next = *contextTree.mBranches[ i ];
					const tSigAnimMoMap::tContextData& data = context[ next.mIndex ];

					const tAniMapFile::tAnimRef* result = fFindAnim( next, context, &data );
					if( result )
						return result;
				}
			}
			else
			{
				// last node was a context switch, so its branches were actually switch values.
				//  compare values and pursue matches
				for( u32 i = 0; i < contextTree.mBranches.fCount( ); ++i )
				{
					const tAniMapFile::tContextSwitch& next = *contextTree.mBranches[ i ];
					if( matchingData->fCurrentValue( ) == next.mIndex )
					{
						// this context value matched.
						//  continue deeper into the tree, with no current context to compare against.
						const tAniMapFile::tAnimRef* result = fFindAnim( next, context, NULL );
						if( result )
							return result;
					}
				}
			}

			// branches took us no where. if we have leaves. use it.

			if( contextTree.mLeaves.fCount( ) )
			{
				// todo, random or something
				return contextTree.mLeaves[ 0 ].fTypedPtr( );
			}

			return NULL;
		}
	}

	void tSigAnimMoMap::tAnimData::fEvaluateContext( const tGrowableArray< tContextData >& context, tEvalutateData& data )
	{
		if( mAnimMap )
		{
			const tAniMapFile::tAnimRef* newAnim = fFindAnim( *mAnimMap->mRoot, context, NULL );

			if( newAnim != mCurrentAnim && newAnim )
			{
				mCurrentAnim = newAnim;

				tAnimPackFile* animPack = newAnim->mAnimPack->fGetResourcePtr( )->fCast<tAnimPackFile>( );
				if( animPack )
				{
					const tKeyFrameAnimation* anim = animPack->fFindAnim( newAnim->mAnimName );
					if( anim )
					{
						tKeyFrameAnimDesc desc;
						desc.mAnim = anim;

						if( mOneShot )
						{
							desc.mFlags = tAnimTrack::cFlagClampTime;
							desc.mBlendOut = 0.2f;
						}

						if( data.mFirstRun )
							desc.mBlendIn = 0.f;

						const f32 currentBlend = mAnimTrack ? mAnimTrack->fBlendScale( ) : 1.f;
						const f32 currentTimeScale = mAnimTrack ? mAnimTrack->fTimeScale( ) : (mData->mTimeScale * newAnim->mTimeScale);

						mAnimTrack.fReset( NEW tKeyFrameAnimTrack( desc ) );
						mAnimTrack->fSetBlendScale( currentBlend );
						mAnimTrack->fSetTimeScale( currentTimeScale );


						fTrackPtr( ).fReset( mAnimTrack.fGetRawPtr( ) );
					}
					else
						log_warning( "Anim missing: '" << newAnim->mAnimName << "'" );
				}
				else
					log_warning( "Anim pack missing: '" << newAnim->mAnimPack->fGetResourcePtr( )->fGetPath( ) << "'" );
			}
		}

		// as a fail safe, instantiate a null track if we didn't end up with one
		if( !mAnimTrack )
		{
			fTrackPtr( ).fReset( NEW tBlendAnimTrack( tBlendAnimTrack::tTrackList( ), tAnimTrackDesc( ) ) );
		}
	}

	tAnimTrackPtr& tSigAnimMoMap::tAnimData::fTrackPtr( )
	{
		if( mOwner )
			return mOwner->fSubTracks( )[ mOwnerSlot ];
		
		sigassert( mSkeleton );
		return mSkeleton->fTracks( )[ mOwnerSlot ];
	}

	void tSigAnimMoMap::tAnimData::fStep( f32 dt )
	{
		if( mOneShot && mAnimTrack )
		{
			if( mAnimTrack->fHasEnded( ) )
			{
				sigassert( mParentBlendTrack );
				mParentBlendTrack->fAnimEnded( );			
			}
		}
	}

	void tSigAnimMoMap::tContextData::fSetCurrentValue( u32 value )
	{
		if( mCurrentValue != value )
		{
			mCurrentValue = value;
			for( u32 i = 0; i < mAnims.fCount( ); ++i )
				mAnims[ i ]->fEvaluateContext( *mContextDataArray, tEvalutateData( false ) );
		}
	}

	void tSigAnimMoMap::tContextData::fAddAnimData( tAnimData* data ) 
	{ 
		sigassert( !mAnims.fFind( data ) );
		mAnims.fPushBack( data ); 
	}

	void tSigAnimMoMap::fEvaluate( tEvalutateData& data )
	{
		for( u32 i = 0; i < mAnimDatas.fCount( ); ++i )
			mAnimDatas[ i ].fEvaluateContext( mContextData, data );
	}

	void tSigAnimMoMap::fStep( f32 dt )
	{
		for( u32 i = 0; i < mBlendDatas.fCount( ); ++i )
			mBlendDatas[ i ].fStep( dt );

		for( u32 i = 0; i < mAnimDatas.fCount( ); ++i )
			mAnimDatas[ i ].fStep( dt );
	}

	void tSigAnimMoMap::fInitialize( )
	{
		// Apply initial tracks
		fEvaluate( tEvalutateData( true ) );

		// Apply initial values
		for( u32 i = 0; i < mBlendDatas.fCount( ); ++i )
			mBlendDatas[ i ].fSetBlend( mBlendDatas[ i ].fBlend( ) );
	}

} }

