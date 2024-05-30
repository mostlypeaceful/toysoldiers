#include "BasePch.hpp"
#include "tGameEffects.hpp"
#include "tGameAppBase.hpp"
#include "tSceneGraphFile.hpp"
#include "Input/tRumbleManager.hpp"
#include "tProfiler.hpp"
#include "tSync.hpp"

namespace Sig
{

	devvar( b32, Gameplay_GameEffects_Enable, true );
	devvar( b32, Gameplay_GameEffects_Log, false );
	devvar( b32, Gameplay_GameEffects_FrustumTest, true );

	devvar( bool, Perf_Audio_GameAndAnimEffects, false );

	namespace
	{

		enum tEffectsTableParams
		{
			cEffectTableParamSigml,
			cEffectTableParamAudioEvent,
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

		enum tSurfaceLookupColumns
		{
			cSurfaceLookupSigml01,
			cSurfaceLookupSigml02,
			cSurfaceLookupSigml03,
			cSurfaceLookupSigmlCount,
		};

		static const tStringPtr cDefaultSurfaceType( "DEFAULT" );
	}
 

	void tEffectPlayer::fStepPersistentEffects( f32 dt )
	{
		for( s32 i = mPersistent.fCount( ) - 1; i >= 0; --i )
		{
			tEffectProperties& effect = mPersistent[ i ]->mEffect;
			sigassert( effect.mTarget );
			sigassert( effect.mOwner );

			// kill if our target has died or the user is no longer holding onto the effect
			if( !effect.mTarget->fSceneGraph( ) || !effect.mOwner->fSceneGraph( ) || mPersistent[ i ].fRefCount( ) == 1 )
			{
				mPersistent.fErase( i );
				continue;
			}

			tEffectPlayer::tEffectData data = fGetEffectsData( effect.mOwner.fGetRawPtr( ) );
			fProcessEvent( data, effect, effect.mTarget->fObjectToWorld( ).fGetTranslation( ), true, mPersistent[ i ]->mUserScale );
		}
	}

	tGameEffects::tGameEffects( )
		: mLogicEventID( ~0 )
	{
	}

	tGameEffects::~tGameEffects( )
	{
	}

	tEntity* tGameEffects::fPlayEffect( tEntity* ownerEntity, const tStringPtr& effect, tEffectArgs& args )
	{
		sigassert( ownerEntity );

		profile( cProfilePerfGameEffects );

		if( Gameplay_GameEffects_Log )
			log_line( 0, "Play Effect: " << effect << " on: " << ownerEntity->fName( ) );

		const tStringHashDataTable& table = fInstance( ).mEffectsTable.fIndexTable( 0 );
		u32 rowIndex = table.fRowIndex( effect );
		if( rowIndex == ~0 ) 
		{
			log_warning_nospam( 0, "No effect entry with name: '" << effect << "'" );
			return NULL;
		}

		f32 minBlendStrength = table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamMinBlendStrength );
		if( args.mBlendStrength < minBlendStrength ) return NULL;

		tStringPtr path = table.fIndexByRowCol<tStringPtr>( rowIndex, cEffectTableParamSigml );
		if( path.fExists( ) )
			path = fSurfaceLookup( path, args.mSurfaceType );

		tStringPtr attach		= table.fIndexByRowCol<tStringPtr>( rowIndex, cEffectTableParamAttachTo );
		tStringPtr audioEvent	= table.fIndexByRowCol<tStringPtr>( rowIndex, cEffectTableParamAudioEvent );
		b32 positionOnce		= (u32)table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamPositionOnce );
		b32 logicEvent			= (u32)table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamLogicEvent );

		tEffectProperties effectProps;
		effectProps.mFrustumCull			= !(b32)table.fIndexByRowCol<f32>(rowIndex, cEffectTableParamRumbleDontFrustumCull );
		effectProps.mRumble.mIntensity		= table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamRumble );
		effectProps.mRumble.mVibeMix		= table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamRumbleMix );
		effectProps.mRumble.mDuration		= table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamRumbleTime );
		effectProps.mRumble.mDecay			= table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamRumbleDecay );
		effectProps.mRumble.mMaxDist		= table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamRumbleMaxDistance );
		effectProps.mRumble.mFallOff		= table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamRumbleFallOff );
		effectProps.mScreenShake.mMagnitude	= table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamCameraShake );
		effectProps.mScreenShake.mTime		= table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamCameraShakeTime );
		effectProps.mScreenShake.mMaxDistance = table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamCameraShakeMaxDist );

		b32 persistent = (b32)table.fIndexByRowCol<f32>( rowIndex, cEffectTableParamRumblePersistent );

		tPersistentEffectPtr persistentP;
		if( persistent )
			persistentP.fReset( NEW tPersistentEffect( effectProps ) );

		tGameAppBase& app = tGameAppBase::fInstance( ); //might spawn to scene graph root
		tEntity *parent = NULL;

		Audio::tSourcePtr audio;

		if( !Perf_Audio_GameAndAnimEffects && audioEvent.fExists( ) )
		{
			audio.fReset( NEW Audio::tSource( effect.fCStr( ) ) );
		}

		// Fire the event before any use of args
		if( logicEvent && mLogicEventID != ~0 )
		{
			Logic::tEvent event = Logic::tEvent( mLogicEventID, Logic::tEventContextPtr( NEW tEffectLogicEventContext( args, audio, audioEvent, persistentP ) ) );
			ownerEntity->fLogic( )->fHandleLogicEvent( event );
		}

		// Determine owner
		if( attach.fExists( ) )
		{
			if( args.mParentOverride )
				parent = args.mParentOverride->fFirstDescendentWithName( attach );
			else if( ownerEntity )
				parent = ownerEntity->fFirstDescendentWithName( attach );
		}

		if( !parent ) 
		{
			// no parent yet, try some fall backs.
			if( args.mParentOverride )
				parent = args.mParentOverride;
			else if( ownerEntity )
				parent = ownerEntity;
			else
				parent = &app.fSceneGraph( )->fRootEntity( );
		}

		// Compute transform, if transformOverride != NULL, *transformOverride will be equal to eventXform
		Math::tMat3f eventXform = parent->fObjectToWorld( );
		const Math::tMat3f* transformOverride = args.mTransformOverride;

		if( transformOverride )
			eventXform = *transformOverride;
		else if( positionOnce )
			transformOverride = &eventXform;
		//else
		//	transformOverride = NULL; //this is redundant but here for clarity

		Math::tVec3f eventPos = eventXform.fGetTranslation( );

		// The spawnParent is who we spawn as a child of. the parent pointer is for transformation data only (if they're different)
		tEntity *spawnParent = positionOnce ? &app.fSceneGraph( )->fRootEntity( ) : parent;
		if( args.mInsertParent ) 
		{ 
			args.mInsertParent->fSpawn( *spawnParent ); 
			spawnParent = args.mInsertParent; 
		}

		// Actually spawn
		tEntity* ent = NULL;
		if( !args.mDontSpawnEffect )
		{
			if( !args.mSurfaceNormal )
				ent = spawnParent->fSpawnChild( tFilePathPtr( path ) );
			else
			{
				sigassert( args.mInputNormal && args.mXDir );
				transformOverride = NULL;
				ent = spawnParent->fSpawnSurfaceFxSystem( tFilePathPtr( path ), eventPos, *args.mSurfaceNormal, *args.mInputNormal, *args.mXDir );
			}
		}

		if( ent && transformOverride ) 
		{
			ent->fSetLockedToParent( false );
			ent->fMoveTo( eventXform );
		}

		if( audio )
		{
			audio->fMoveTo( eventXform );
			audio->fUpdatePosition( );
			audio->fHandleEvent( audioEvent );
		}

		for( u32 i = 0; i < fInstance( ).gPlayers.fCount( ); ++i )
		{
			const tEffectPlayerPtr& player = fInstance( ).gPlayers[ i ];
			if( persistentP )
			{
				persistentP->mEffect.mOwner.fReset( ownerEntity );
				persistentP->mEffect.mTarget.fReset( parent );
				player->mPersistent.fPushBack( persistentP );			
			}
			else
			{
				tEffectPlayer::tEffectData data = player->fGetEffectsData( ownerEntity );
				tEffectPlayer::fProcessEvent( data, effectProps, eventPos, false, 1.f );
			}
		}

		return ent;
	}	

	void tEffectPlayer::fProcessEvent( tEffectData& playerData, const tEffectProperties& effect, const Math::tVec3f& pos, b32 persistent, f32 scale )
	{
		if( playerData.mCamera )
		{
			if( effect.mFrustumCull && Gameplay_GameEffects_FrustumTest && !playerData.mCamera->fViewport( )->fRenderCamera( ).fGetWorldSpaceFrustum( ).fContains( pos ) )
				return;

			Math::tVec3f delta = playerData.mEffectRefPt - pos;

			f32 shakeFactor;
			f32 rumbleFactor;

			if( playerData.mFullStrength )
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
					playerData.mCamera->fBeginCameraShake( Math::tVec2f( effect.mScreenShake.mMagnitude * shakeFactor ), effect.mScreenShake.mTime * shakeFactor );

				if( rumbleFactor > 0 )
				{
					if( persistent )
						playerData.mGamePad->fRumble( ).fSetExplicitRumble( effect.mRumble.mIntensity * rumbleFactor );
					else
						playerData.mGamePad->fRumble( ).fAddEvent( effect.mRumble, rumbleFactor );
				}
			}
		}
	}

	tStringPtr tGameEffects::fSurfaceLookup( const tStringPtr& tableName, u32 surfaceType )
	{
		const tStringHashDataTable* table = mEffectsTable.fFindTable( tableName );
		if( !table )
		{
			// they must have passed us in a sigml
			return tableName;
		}

		const tStringPtr& name = mSurfaceTypeEnum.fEnumToStr( surfaceType );
		u32 row = table->fRowIndex( name );
		if( row == ~0 )
			row = table->fRowIndex( cDefaultSurfaceType );
		u32 randomizedIndex = tRandom::fObjectiveRand( ).fIntInRange( cSurfaceLookupSigml01, cSurfaceLookupSigmlCount-1 );

		tStringPtr effectsSigmlToSpawn;
		for( s32 i = randomizedIndex; i > -1; --i )
		{
			effectsSigmlToSpawn = table->fIndexByRowCol<tStringPtr>( row, i );
			if( effectsSigmlToSpawn.fLength( ) > 0 )
				break;
		}

		return effectsSigmlToSpawn;
	}

	tEntity* tGameEffects::fPlayEffectForScript( tEntity* ownerEntity, const tStringPtr& effect )
	{
		return fPlayEffect( ownerEntity, effect );
	}

	void tGameEffects::fLoadResourcesAndValidate( const tResourcePtr& res, const tGameEnum& surfaceEnum, u32 logicEventID )
	{
		mEffectsTable.fSet( res );
		mSurfaceTypeEnum = surfaceEnum;
		mLogicEventID = logicEventID;

		if( mEffectsTable.fTableCount( ) )
		{
			// First table, load sigmls
			for( u32 r = 0; r < mEffectsTable.fIndexTable( 0 ).fTable( ).fRowCount( ); ++r )
			{
				tStringPtr path = mEffectsTable.fIndexTable( 0 ).fIndexByRowCol<tStringPtr>( r, cEffectTableParamSigml );

				// dont load the path if it's a key to the surface look up tables
				if( path.fExists( ) && !mEffectsTable.fFindTable( path ) )
					tGameAppBase::fInstance( ).fAddAlwaysLoadedResource( tResourceId::fMake< tSceneGraphFile >( tFilePathPtr( path ) ) );
			}
		}

		// surface tables - first table is effects table above, dont validate it
		for( u32 t = 1; t < mEffectsTable.fTableCount( ); ++t )
			for( u32 r = 0; r < mEffectsTable.fIndexTable( t ).fTable( ).fRowCount( ); ++r )
			{
				const tStringPtr& rowName = mEffectsTable.fIndexTable( t ).fTable( ).fRowName( r );
				for( int s = 0; s < cSurfaceLookupSigmlCount; ++s )
				{
					tStringPtr path = mEffectsTable.fIndexTable( t ).fIndexByRowCol<tStringPtr>( r, s );
					if( path.fExists( ) )
					{
						if( mSurfaceTypeEnum.fStringToEnum( rowName ) >= mSurfaceTypeEnum.fCount( )
							&& rowName != cDefaultSurfaceType )
							log_warning( 0, "Invalid rowname in surface lookup table: " << mEffectsTable.fIndexTable( t ).fTable( ).fName( ) << " row: " << rowName );

						
							tGameAppBase::fInstance( ).fAddAlwaysLoadedResource( tResourceId::fMake< tSceneGraphFile >( tFilePathPtr( path ) ) );
					}
				}
			}
	}


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
