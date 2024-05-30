#include "BasePch.hpp"
#include "tGameEffects.hpp"
#include "tApplication.hpp"
#include "tSceneGraphFile.hpp"
#include "tResourceLoadList2.hpp"
#include "Audio/tSource.hpp"
#include "gfx/tCameraController.hpp"

namespace Sig
{

	devvar( bool, Gameplay_GameEffects_Enable, true );
	devvar( bool, Gameplay_GameEffects_Log, false );
	devvar( bool, Gameplay_GameEffects_FrustumTest, true );
	devvar( bool, Resources_PermaLoad_GameEffects, true );
	devvar( bool, Perf_Audio_GameAndAnimEffects, false );

	namespace
	{

		enum tEffectsTableParams
		{
			cEffectTableParamSigml,
			cEffectTableParamAudioEvent,
			cEffectTableParamAudioStopEvent,
			cEffectTableParamMinBlendStrength,
			cEffectTableParamAttachTo,
			cEffectTableParamPositionOnce,
			cEffectTableParamLogicEvent,
			cEffectTableParamCameraShake,
			cEffectTableParamCameraShakeTime,
			cEffectTableParamCameraShakeMaxDist,
			cEffectTableParamRumble,
			cEffectTableParamRumbleMix,
			cEffectTableParamRumbleTime,
			cEffectTableParamRumbleDecay,
			cEffectTableParamRumbleMaxDistance,
			cEffectTableParamRumbleFallOff,
			cEffectTableParamRumbleDontFrustumCull,
			cEffectTableParamRumblePersistent
		};

		static const tStringPtr cDefaultSurfaceType( "DEFAULT" );
	}

	//------------------------------------------------------------------------------
	// tEffectPlayer::tEffectData
	//------------------------------------------------------------------------------
	tEffectPlayer::tEffectData::tEffectData( Gfx::tCameraController* cam, const Input::tGamepad* pad, const Math::tVec3f& refPt, b32 fullStrength )
		: mCamera( cam )
		, mGamePad( pad )
		, mEffectRefPt( refPt )
		, mFullStrength( fullStrength )
	{
	}

	//------------------------------------------------------------------------------
	void tEffectPlayer::tEffectData::fProcessEvent( const tEffectProperties& effect, const Math::tVec3f& pos, b32 persistent, f32 scale )
	{
		if( mCamera )
		{
			if( effect.mFrustumCull && Gameplay_GameEffects_FrustumTest && !mCamera->fViewport( )->fRenderCamera( ).fGetWorldSpaceFrustum( ).fContains( pos ) )
				return;

			Math::tVec3f delta = mEffectRefPt - pos;

			f32 shakeFactor;
			f32 rumbleFactor;

			if( mFullStrength )
			{
				shakeFactor = 1.f;
				rumbleFactor = 1.f;
			}
			else
			{
				f32 deltaLen = delta.fLength( );

				if( effect.mScreenShake.mMaxDistance > 0.0001f ) 
				{
					shakeFactor = fClamp( deltaLen / effect.mScreenShake.mMaxDistance, 0.f, 1.f );
					shakeFactor = 1.f - shakeFactor;
					shakeFactor *= shakeFactor * shakeFactor;
				}
				else
					shakeFactor = 1.f;

				if( effect.mRumble.mMaxDist > 0.0001f ) 
				{
					rumbleFactor = fClamp( deltaLen / effect.mRumble.mMaxDist, 0.f, 1.f );
					rumbleFactor = 1.f - rumbleFactor;
					rumbleFactor = pow( rumbleFactor, effect.mRumble.mFallOff );
				}
				else
					rumbleFactor = 1.f;
			}

			shakeFactor *= scale;
			rumbleFactor *= scale;

			if( Gameplay_GameEffects_Enable ) 
			{
				if( shakeFactor > 0 )
					mCamera->fBeginCameraShake( Math::tVec2f( effect.mScreenShake.mMagnitude * shakeFactor ), effect.mScreenShake.mTime * shakeFactor );

				if( rumbleFactor > 0 )
				{
					if( persistent )
						mGamePad->fRumble( ).fSetExplicitRumble( effect.mRumble.mIntensity * rumbleFactor );
					else
						mGamePad->fRumble( ).fAddEvent( effect.mRumble, rumbleFactor );
				}
			}
		}
	}
 
	//------------------------------------------------------------------------------
	// tEffectPlayer
	//------------------------------------------------------------------------------
	tEffectPlayer::tEffectPlayer( )
	{
	}

	//------------------------------------------------------------------------------
	tEffectPlayer::~tEffectPlayer( )
	{
	}

	//------------------------------------------------------------------------------
	tEffectPlayer::tEffectData tEffectPlayer::fGetEffectsData( tEntity* owner ) const
	{
		return tEffectPlayer::tEffectData( );
	}

	//------------------------------------------------------------------------------
	void tEffectPlayer::fAddPersistentEffect( tPersistentEffect* pe )
	{
		mPersistent.fPushBack( tRefCounterPtr<tPersistentEffect>( pe ) );
	}

	//------------------------------------------------------------------------------
	void tEffectPlayer::fClearPersistentEffects( )
	{
		mPersistent.fSetCount( 0 );
	}

	//------------------------------------------------------------------------------
	void tEffectPlayer::fStepPersistentEffects( f32 dt )
	{
		for( s32 i = mPersistent.fCount( ) - 1; i >= 0; --i )
		{
			tEffectProperties& effect = mPersistent[ i ]->mEffect;
			tEntity* fxOwner = effect.mOwner.fObject( );
			tEntity* fxTarget = effect.mTarget.fObject( );

			if( !fxOwner || !fxTarget )
			{
				mPersistent.fErase( i );
				continue;
			}

			if( !fxOwner->fSceneGraph( ) || !fxTarget->fSceneGraph( ) )
			{
				mPersistent.fErase( i );
				continue;
			}

			if( mPersistent[ i ].fRefCount( ) == 1 )
			{
				mPersistent.fErase( i );
				continue;
			}

			tEffectPlayer::tEffectData data = fGetEffectsData( fxOwner );
			data.fProcessEvent( effect, fxTarget->fObjectToWorld( ).fGetTranslation( ), true, mPersistent[ i ]->mUserScale );
		}
	}

	//------------------------------------------------------------------------------
	// tEffectLogicEventContext
	//------------------------------------------------------------------------------
	tEffectLogicEventContext::tEffectLogicEventContext( const tEffectArgs& args, Audio::tSource* audio, tStringPtr& audioEvent, tPersistentEffect* persistent )
		: mArgs( args )
		, mAudio( audio )
		, mAudioEvent( audioEvent )
		, mPersistent( persistent )
	{
	}

	//------------------------------------------------------------------------------
	// tGameEffects::tTableInfo
	//------------------------------------------------------------------------------
	tGameEffects::tTableInfo::tTableInfo( )
		: mTable( NULL )
		, mRow( ~0 )
	{ }

	//------------------------------------------------------------------------------
	// tGameEffects::tAudioStopEvent
	//------------------------------------------------------------------------------
	tGameEffects::tAudioStopEvent::tAudioStopEvent( const tStringPtr& stopEvent, tEntity* owner, Audio::tSource* source )
		: mStopEvent( stopEvent )
		, mOwner( owner )
		, mSource( source )
	{ 
	}

	//------------------------------------------------------------------------------
	b32 tGameEffects::tAudioStopEvent::fCheck( )
	{
		sigassert( mOwner );
		sigassert( mSource );

		// update audio engine.
		mSource->fMoveTo( mOwner->fObjectToWorld( ) );
		mSource->fUpdatePosition( );

		if( !mOwner->fSceneGraph( ) && !mOwner->fInSpawnList( ) )
		{
			mSource->fHandleEvent( mStopEvent );
			mSource->fDelete( );
			return true;
		}
		//else
		//	mOwner->fSceneGraph( )->fDebugGeometry( ).fRenderOnce( Math::tSpheref( mSource->fObjectToWorld( ).fGetTranslation( ), 5.f ), Math::tVec4f::cOnesVector );

		return false;
	}

	//------------------------------------------------------------------------------
	// tGameEffects
	//------------------------------------------------------------------------------
	tGameEffects::tGameEffects( )
		: mLogicEventID( ~0 )
	{
	}

	//------------------------------------------------------------------------------
	tGameEffects::~tGameEffects( )
	{
	}

	//------------------------------------------------------------------------------
	tEntity* tGameEffects::fPlayEffect( tEntity* ownerEntity, const tStringPtr& _effect, const tEffectArgs& args )
	{
		sigcheckfail( ownerEntity, return NULL );

		profile( cProfilePerfGameEffects );

		// Remap the effect if need be
		tStringPtr effect = _effect;
		if( mEffectsMapTable.fIsSet( ) && args.mEffectMapTable.fLength( ) ) do
		{
			const tStringHashDataTable* table = mEffectsMapTable.fFindTable( args.mEffectMapTable );
			log_sigcheckfail( table, "Tried to remap effect for missing table: " << args.mEffectMapTable, break );

			const u32 rowIdx = table->fRowIndex( _effect );
			if( rowIdx != ~0 )
			{
				effect = table->fIndexByRowCol<tStringPtr>( rowIdx, 0 );

				if( Gameplay_GameEffects_Log )
					log_line( 0, "Map Effect: " << _effect << " to " << effect );
			}

		} while( false );

		if( Gameplay_GameEffects_Log )
			log_line( 0, "Play Effect: " << effect << " on: " << ownerEntity->fName( ) );

		const tStringHashDataTable& table = mEffectsTable.fIndexTable( 0 );
		u32 rowIndex = table.fRowIndex( effect );
		if( rowIndex == ~0 ) 
		{
			log_warning( "No effect entry with name: '" << effect << "'" );
			return NULL;
		}

		f32 minBlendStrength = table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamMinBlendStrength );
		if( args.mBlendStrength < minBlendStrength )
			return NULL;

		tFilePathPtr path = fSurfaceLookup( table, rowIndex, args );

		tStringPtr attach		= table.fIndexByRowCol<tStringPtr>( rowIndex, cEffectTableParamAttachTo );
		tStringPtr audioEvent	= table.fIndexByRowCol<tStringPtr>( rowIndex, cEffectTableParamAudioEvent );
		tStringPtr audioStopEvent	= table.fIndexByRowCol<tStringPtr>( rowIndex, cEffectTableParamAudioStopEvent );
		b32 positionOnce		= table.fIndexByRowCol<b32>( rowIndex, cEffectTableParamPositionOnce );
		b32 logicEvent			= table.fIndexByRowCol<b32>( rowIndex, cEffectTableParamLogicEvent );

		// If we're surface orienting, need to spawn to the scene graph root. ie world space. and only position it once, ie not parent relative.
		const b32 surfaceOrient = (args.mSurfaceNormal != NULL);
		positionOnce = positionOnce || surfaceOrient;

		tEffectProperties effectProps;
		effectProps.mFrustumCull			= !table.fIndexByRowCol<b32>(rowIndex, cEffectTableParamRumbleDontFrustumCull );
		effectProps.mRumble.mIntensity		= table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamRumble );
		effectProps.mRumble.mVibeMix		= table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamRumbleMix );
		effectProps.mRumble.mDuration		= table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamRumbleTime );
		effectProps.mRumble.mDecay			= table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamRumbleDecay );
		effectProps.mRumble.mMaxDist		= table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamRumbleMaxDistance );
		effectProps.mRumble.mFallOff		= table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamRumbleFallOff );
		effectProps.mScreenShake.mMagnitude	= table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamCameraShake );
		effectProps.mScreenShake.mTime		= table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamCameraShakeTime );
		effectProps.mScreenShake.mMaxDistance = table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamCameraShakeMaxDist );

		const b32 persistent = table.fIndexByRowCol<b32>( rowIndex, cEffectTableParamRumblePersistent );

		tRefCounterPtr<tPersistentEffect> persistentP;
		if( persistent )
			persistentP.fReset( NEW_TYPED( tPersistentEffect )( effectProps ) );

		tApplication& app = tApplication::fInstance( );
		tEntity *parent = NULL;

		Audio::tSourcePtr audio;

		if( !Perf_Audio_GameAndAnimEffects && audioEvent.fExists( ) )
		{
			audio.fReset( NEW Audio::tSource( effect.fCStr( ) ) );
		}

		// Fire the event before any use of args
		if( logicEvent && mLogicEventID != ~0 )
		{
			Logic::tEvent event = Logic::tEvent( mLogicEventID, Logic::tEventContextPtr( NEW_TYPED( tEffectLogicEventContext )( args, audio.fGetRawPtr( ), audioEvent, persistentP.fGetRawPtr( ) ) ) );
			ownerEntity->fHandleLogicEvent( event );
		}

		// Determine owner
		tGrowableArray<tEntity*> spawnPts;
		if( attach.fExists( ) )
		{
			tEntity* origin = args.mParentOverride ? args.mParentOverride : ownerEntity;

			if( !mTargetAcculuator.fNull() )
				mTargetAcculuator( origin, attach, spawnPts );
			else
				origin->fAllDescendentsWithName( attach, spawnPts );


			if( !spawnPts.fCount( ) )
				log_warning( "No fx attach pt found named: '" << attach << "' referenced in effect: '" << effect << "'" );
		}

		if( !spawnPts.fCount( ) ) 
		{
			// no parent yet, try some fall backs.
			if( args.mParentOverride )
				spawnPts.fPushBack( args.mParentOverride );
			else if( ownerEntity )
				spawnPts.fPushBack( ownerEntity );
			else
				spawnPts.fPushBack( &app.fSceneGraph( )->fRootEntity( ) );
		}

		// These are guaranteed to be initialized, since spawnPts is asserted to be set to something.
		Math::tMat3f eventXform;
		Math::tVec3f eventPos;
		tEntity* ent = NULL;

		sigassert( spawnPts.fCount( ) );
		for( u32 i = 0; i < spawnPts.fCount( ); ++i )
		{
			tEntity* parent = spawnPts[ i ];

			// Compute transform, if transformOverride != NULL, *transformOverride will be equal to eventXform
			eventXform = parent->fObjectToWorld( );
			const Math::tMat3f* transformOverride = args.mTransformOverride;

			if( transformOverride )
				eventXform = *transformOverride;
			else if( positionOnce )
				transformOverride = &eventXform;
			//else
			//	transformOverride = NULL; //this is redundant but here for clarity

			eventPos = eventXform.fGetTranslation( );

			// The spawnParent is who we spawn as a child of. the parent pointer is for transformation data only (if they're different)
			tEntity *spawnParent = positionOnce ? &app.fSceneGraph( )->fRootEntity( ) : parent;
			if( args.mInsertParent ) 
			{ 
				args.mInsertParent->fSpawn( *spawnParent ); 
				spawnParent = args.mInsertParent; 
			}

			// Actually spawn
			if( !args.mDontSpawnEffect && path.fLength( ) )
			{
				const tResourceId rid = tResourceId::fMake< tSceneGraphFile >( path );
				const tResourcePtr res = tApplication::fInstance( ).fResourceDepot( )->fQueryLoadBlock( rid, this );

				if( !args.mSurfaceNormal )
					ent = spawnParent->fSpawnChild( path );
				else
				{
					sigassert( args.mInputNormal );
					transformOverride = NULL;
					ent = spawnParent->fSpawnSurfaceFxSystem( path, eventPos, *args.mSurfaceNormal, *args.mInputNormal );
				}
			}

			// if we were positioned once, or for any other reason bound to the scene root, set full transform.
			//  otherwise leave transform as identity (this will be the parent relative xform before spawning.)
			if( ent ) 
			{
				if( spawnParent == &app.fSceneGraph( )->fRootEntity( ) )
					ent->fMoveTo( eventXform );

				if( transformOverride )
					ent->fSetLockedToParent( false );
			}

			if( args.mEntitiesOut )
				args.mEntitiesOut->fPushBack( ent );
		}

		if( audio )
		{
			audio->fMoveTo( eventXform );
			audio->fUpdatePosition( );
			audio->fHandleEvent( audioEvent );

			if( ent && audioStopEvent.fLength( ) )
				mAudioStopEvents.fPushBack( tAudioStopEvent( audioStopEvent, ent, audio.fGetRawPtr( ) ) );
		}

		for( u32 i = 0; i < gPlayers.fCount( ); ++i )
		{
			const tEffectPlayerPtr& player = gPlayers[ i ];
			if( persistentP )
			{
				persistentP->mEffect.mOwner.fReset( ownerEntity );
				persistentP->mEffect.mTarget.fReset( parent );
				player->fAddPersistentEffect( persistentP.fGetRawPtr( ) );
			}
			else
			{
				tEffectPlayer::tEffectData data = player->fGetEffectsData( ownerEntity );
				data.fProcessEvent( effectProps, eventPos, false, 1.0f );
			}
		}

		return ent;
	}	

	//------------------------------------------------------------------------------
	tFilePathPtr tGameEffects::fSurfaceLookup( const tStringHashDataTable& table, u32 row, const tEffectArgs& args )
	{
		const tStringPtr& surfaceName = args.mSurfaceType < mSurfaceTypeEnum.fCount( ) 
			? mSurfaceTypeEnum.fEnumToStr( args.mSurfaceType ) 
			: cDefaultSurfaceType;
		
		tTableInfo surfaceInfo;
		{
			const tStringPtr sigmlOrTable = table.fIndexByRowCol<tStringPtr>( row, cEffectTableParamSigml );
			surfaceInfo = fFindTableInfo( sigmlOrTable, surfaceName );
			
			// Didn't find the table, must be a path
			if( !surfaceInfo.mTable )
				return tFilePathPtr( sigmlOrTable );
		}

		sigassert( surfaceInfo.mTable && "Sanity!!!" );
		
		if( surfaceInfo.mRow == ~0 )
			surfaceInfo.mRow = surfaceInfo.mTable->fRowIndex( cDefaultSurfaceType );

		const u32 max3rdDim = surfaceInfo.mTable->fTable( ).fColCount( ) - 1;
		const u32 thirdIndex = (args.m3rdDimsension == ~0) ? tRandom::fObjectiveRand( ).fIntInRange( 0, max3rdDim ) : fMin( args.m3rdDimsension, max3rdDim );

		tFilePathPtr effectsSigmlToSpawn;
		for( s32 i = thirdIndex; i > -1; --i )
		{
			effectsSigmlToSpawn = surfaceInfo.mTable->fIndexByRowCol<tFilePathPtr>( surfaceInfo.mRow, i );
			if( effectsSigmlToSpawn.fLength( ) )
				break;
		}

		return effectsSigmlToSpawn;
	}

	//------------------------------------------------------------------------------
	tEntity* tGameEffects::fPlayEffectForScript( tEntity* ownerEntity, const tStringPtr& effect )
	{
		return fPlayEffect( ownerEntity, effect );
	}

	//------------------------------------------------------------------------------
	void tGameEffects::fInitialize( 
		const tResourcePtr& effectTable, 
		const tResourcePtr& effectsMapTable, 
		const tGameEnum& surfaceEnum, 
		u32 logicEventID /* = ~0 */ )
	{
		mResources.fReset( NEW_TYPED( tResourceLoadList2 )( ) );
		mEffectsTable.fSet( effectTable );

		if( effectsMapTable )
			mEffectsMapTable.fSet( effectsMapTable );

		mSurfaceTypeEnum = surfaceEnum;
		mLogicEventID = logicEventID;
	}

	//------------------------------------------------------------------------------
	void tGameEffects::fShutdown( )
	{
		mResources.fRelease( );
		mEffectsTable = tStringHashDataTableFile( );
		mEffectsMapTable = tStringHashDataTableFile( );
		mLogicEventID = ~0u;
	}

	//------------------------------------------------------------------------------
	void tGameEffects::fLoadResourcesAndValidate( )
	{
		sigassert( mEffectsTable.fIsSet( ) );

		if( Resources_PermaLoad_GameEffects && mEffectsTable.fTableCount( ) )
		{
			// First table, load sigmls
			for( u32 r = 0; r < mEffectsTable.fIndexTable( 0 ).fTable( ).fRowCount( ); ++r )
			{
				const tFilePathPtr path = fTryGetSigmlPath( mEffectsTable.fIndexTable( 0 ), r );
				const tResourceId rid = tResourceId::fMake< tSceneGraphFile >( path );

				if( path.fLength( ) )
				{
					log_assert( tApplication::fInstance( ).fResourceDepot( )->fResourceExists( rid.fGetPath( ) ), "[" << path << "] referenced by the [" << mEffectsTable.fGetPath( ) << "] table, but it doesn't exist!" );
					mResources->fAdd( rid );
				}
			}
		}

		// surface tables - first table is effects table above, don't validate it
		for( u32 t = 1; t < mEffectsTable.fTableCount( ); ++t )
		{
			for( u32 r = 0; r < mEffectsTable.fIndexTable( t ).fTable( ).fRowCount( ); ++r )
			{
				const tStringPtr& rowName = mEffectsTable.fIndexTable( t ).fTable( ).fRowName( r );

				if( mSurfaceTypeEnum.fStringToEnum( rowName ) >= mSurfaceTypeEnum.fCount( ) && rowName != cDefaultSurfaceType )
					log_warning( "Invalid rowname in surface lookup table: " << mEffectsTable.fIndexTable( t ).fTable( ).fName( ) << " row: " << rowName );

				for( u32 s = 0; s < mEffectsTable.fIndexTable( t ).fTable( ).fColCount( ); ++s )
				{
					const tFilePathPtr path = mEffectsTable.fIndexTable( t ).fIndexByRowCol<tFilePathPtr>( r, s );
					const tResourceId rid = tResourceId::fMake< tSceneGraphFile >( path );

					if( path.fLength( ) )
					{
						log_assert( tApplication::fInstance( ).fResourceDepot( )->fResourceExists( rid.fGetPath( ) ), "[" << path << "] referenced by the [" << mEffectsTable.fGetPath( ) << "] table, but it doesn't exist!" );
						mResources->fAdd( rid );
					}
				}
			}
		}

#ifdef sig_assert

		if( mEffectsMapTable.fIsSet( ) )
		{
			const tStringHashDataTable& fxTable = mEffectsTable.fIndexTable( 0 );
			for( u32 t = 0; t < mEffectsMapTable.fTableCount( ); ++t )
			{
				const tStringHashDataTable& table = mEffectsMapTable.fIndexTable( t );
				const tStringPtr& tableName = table.fTable( ).fName( );

				for( u32 r = 0; r < table.fTable( ).fRowCount( ); ++r )
				{
					const tStringPtr effectFrom = table.fRowName( r );
					const tStringPtr effectTo = table.fIndexByRowCol<tStringPtr>( r, 0 );

					const u32 effectFromIdx = fxTable.fRowIndex( effectFrom );
					const u32 effectToIdx = fxTable.fRowIndex( effectTo );
					log_assert( effectFromIdx !=~0, "Could not find effect(" << effectFrom << ") to map from in " << tableName );
					log_assert( effectToIdx !=~0, "Could not find effect(" << effectTo << ") to map to in " << tableName );
				}
			}
		}

#endif // sig_assert
	}

	//------------------------------------------------------------------------------
	void tGameEffects::fTickST( f32 dt )
	{
		for( s32 i = mAudioStopEvents.fCount( ) - 1; i >= 0; --i )
		{
			if( mAudioStopEvents[ i ].fCheck( ) )
				mAudioStopEvents.fErase( i );
		}
	}

	//------------------------------------------------------------------------------
	tGameEffects::tTableInfo tGameEffects::fFindTableInfo( const tStringPtr& tableName, const tStringPtr& rowName )
	{
		tTableInfo info;
		if( tableName.fLength( ) )
		{
			info.mTable = mEffectsTable.fFindTable( tableName );
			if( info.mTable )
				info.mRow = info.mTable->fRowIndex( rowName );
		}
		return info;
	}

	//------------------------------------------------------------------------------
	tFilePathPtr tGameEffects::fTryGetSigmlPath( const tStringHashDataTable& table, u32 row )
	{
		tStringPtr sigmlOrTable = table.fIndexByRowCol<tStringPtr>( row, cEffectTableParamSigml );

		if( !sigmlOrTable.fExists( ) || mEffectsTable.fFindTable( sigmlOrTable ) )
			return tFilePathPtr::cNullPtr;

		return tFilePathPtr( sigmlOrTable );
	}

}//Sig

namespace Sig
{
	void tGameEffects::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tGameEffects, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("PlayEffect"),	&tGameEffects::fPlayEffectForScript)
			;

		vm.fRootTable( ).Bind( _SC("tGameEffects"), classDesc );
		vm.fRootTable( ).SetInstance(_SC("GameEffects"), &fInstance( ));
	}
}
