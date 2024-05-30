#include "BasePch.hpp"
#include "tUberBreakableLogic.hpp"
#include "tSceneGraph.hpp"
#include "tSceneRefEntity.hpp"
#include "t3dGridEntity.hpp"
#include "Logic/tDamageable.hpp"
#include "tGameEffects.hpp"

namespace Sig
{
	devvar( bool, Debug_UberBreakable_RenderPieces, false );

	namespace 
	{ 
		static const tStringPtr cOnDestroyFX( "destroyFX" ); 

		static tUberBreakableDamagable* fDefaultCreateDamageableInterface( tUberBreakableLogic& uberBreakable )
		{
			return NEW tUberBreakableDamagable( );
		}
	}

	// these get set from game code (i.e., at game init time, before any objects are created)
	tUberBreakableLogic::tCreateDamageableInterface tUberBreakableLogic::gCreateDamageableInterface = &fDefaultCreateDamageableInterface;
	u32 tUberBreakableLogic::cGroundFlags = 0;//GameFlags::cFLAG_GROUND;
	u32 tUberBreakableLogic::cCollisionFlags = 0;//GameFlags::cFLAG_COLLISION;
	u32 tUberBreakableLogic::cDebrisFlags = 0;//GameFlags::cFLAG_COLLISION;
	u32 tUberBreakableLogic::cGameFlags = 0;//GameFlags::cFLAG_HIT_VOLUME;
	u32 tUberBreakableLogic::cOwnerEntityFlags = 0;//GameFlags::cFLAG_PROXY_COLLISION_ROOT;
	b32 tUberBreakableLogic::cCollideWithShapeSpatialSet = true;
	f32 tUberBreakableLogic::cDebrisMaxV = Math::cInfinity;
	b32 tUberBreakableLogic::cAttachPieceLogic = false;
	b32 tUberBreakableLogic::cAcquireProperties = false;
	

	tUberBreakableLogic::tUberBreakableLogic( )
		: mCurrentColorizedGroup( 0 )
		, mDamageableInterface( )
		, mFirstBreak( false )
	{
		mColorizerTimer.fResetElapsedS( Math::cInfinity );
		if( gCreateDamageableInterface )
			mDamageableInterface = gCreateDamageableInterface( *this );
	}
	tUberBreakableLogic::~tUberBreakableLogic( )
	{
		delete mDamageableInterface;
	}
	void tUberBreakableLogic::fOnSpawn( )
	{
		tLogic::fOnSpawn( );

		fOwnerEntity( )->fAddGameTags( cOwnerEntityFlags );

		tGrowableArray<t3dGridEntity*> grids;
		fGatherGridsAndMeshes( *fOwnerEntity( ), grids );

		if( grids.fCount( ) > 0 )
		{
			for( u32 i = 0; i < grids.fCount( ); ++i )
			{
				const Math::tAabbf objSpaceBounds = grids[ i ]->fParentRelativeBox( ).fToAabb( );
				mClumps.fPushBack( tUberBreakableClumpPtr( NEW tUberBreakableClump( *fOwnerEntity( ), objSpaceBounds, grids[ i ]->fCellCounts( ) ) ) );
			}
		}
		else
		{
			// no grid objects found, we default to a single grid with a single cell

			tSceneRefEntity* owner = fOwnerEntity( )->fDynamicCast< tSceneRefEntity >( );
			sigassert( owner );
			const Math::tAabbf objSpaceBounds = owner->fSgResource( )->fCast< tSceneGraphFile >( )->mBounds;
			sigassert( objSpaceBounds.fIsValid( ) );

			mClumps.fPushBack( tUberBreakableClumpPtr( NEW tUberBreakableClump( *fOwnerEntity( ), objSpaceBounds, Math::tVec3u( 1, 1, 1 ) ) ) );
		}

		for( u32 i = 0; i < mClumps.fCount( ); ++i )
			mClumps[ i ]->fComputeSubdivision( mBreakMeshes );

		for( u32 i = 0; i < mClumps.fCount( ); ++i )
			mClumps[ i ]->fLinkClumps( mClumps );

		fConfigureInitialMeshStates( );

		fOnPause( false );
	}
	void tUberBreakableLogic::fOnDelete( )
	{
		for( u32 i = 0; i < mClumps.fCount( ); ++i )
			mClumps[ i ]->fOnDelete( );
		mClumps.fSetCount( 0 );
		tLogic::fOnDelete( );
	}
	void tUberBreakableLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListThinkST );
		}
		else
		{
			fRunListInsert( cRunListThinkST );
		}
	}
	void tUberBreakableLogic::fThinkST( f32 dt )
	{
		fStepDebugRendering( dt );
		for( u32 i = 0; i < mClumps.fCount( ); ++i )
			mClumps[ i ]->fThinkST( dt );
	}
	void tUberBreakableLogic::fOnBreak( )
	{
		if( !mFirstBreak )
		{
			mFirstBreak = true;
			for( u32 i = 0; i < mUnbrokenMeshes.fCount( ); ++i )
				mUnbrokenMeshes[ i ]->fSetDisabled( true );
			for( u32 i = 0; i < mBreakMeshes.fCount( ); ++i )
				mBreakMeshes[ i ]->mMesh->fSetDisabled( false );
			for( u32 i = 0; i < mPermaMeshes.fCount( ); ++i )
				mPermaMeshes[ i ]->fSetDisabled( false );
		}
	}
	f32 tUberBreakableLogic::fQueryPercentageBroken( b32 includePiecesThatWillBreak ) const
	{
		u32 numPiecesBroken = 0, numPiecesTotal = 0;
		for( u32 i = 0; i < mClumps.fCount( ); ++i )
			mClumps[ i ]->fQueryBrokenPieceCounts( numPiecesBroken, numPiecesTotal, includePiecesThatWillBreak );

		return ( f32 )numPiecesBroken / ( f32 )numPiecesTotal;
	}
	void tUberBreakableLogic::fSpawnDestroyFX( )
	{
		if( !mOnDestroyEffect.fExists( ) )
			return;

		tEntity* ent = fOwnerEntity( )->fFirstDescendentWithName( cOnDestroyFX );

		if( !ent )
		{
			log_warning( 0, "No attachment point with name " << cOnDestroyFX );
			ent = fOwnerEntity( );
		}

		tGameEffects::fInstance( ).fPlayEffect( ent, mOnDestroyEffect );
	}
	void tUberBreakableLogic::fSpawnDestroyFX( tUberBreakableClump& clump )
	{
		if( !mOnClumpDestroyEffect.fExists( ) )
			return;

		Math::tMat3f xform = Math::tMat3f::cIdentity;
		xform.fSetTranslation( fOwnerEntity( )->fObjectToWorld( ).fXformPoint( clump.fObjSpaceBounds( ).fComputeCenter( ) ) );

		tEffectArgs args;
		args.mTransformOverride = &xform;
		tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), mOnClumpDestroyEffect, args );
	}
	void tUberBreakableLogic::fSpawnDestroyFX( tUberBreakablePiece& piece )
	{
		if( !mOnPieceDestroyEffect.fExists( ) )
			return;

		Math::tMat3f xform = Math::tMat3f::cIdentity;
		xform.fSetTranslation( piece.fBox( ).fCenter( ) );
		
		tEffectArgs args;
		args.mTransformOverride = &xform;
		tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), mOnPieceDestroyEffect, args );
	}
	void tUberBreakableLogic::fIntersect( const Math::tSpheref& worldSphere, tGrowableArray<tUberBreakablePiece*>& pieces )
	{
		for( u32 i = 0; i < mClumps.fCount( ); ++i )
			mClumps[ i ]->fIntersect( worldSphere, pieces );
	}
	void tUberBreakableLogic::fGatherGridsAndMeshes( tEntity& root, tGrowableArray<t3dGridEntity*>& grids )
	{
		t3dGridEntity* grid = root.fDynamicCast< t3dGridEntity >( );
		tMeshEntity* mesh = root.fDynamicCast< tMeshEntity >( );

		if( grid )
			grids.fPushBack( grid );

		if( mesh )
		{
			switch( mesh->fStateIndex( ) )
			{
			case 0: mBreakMeshes.fPushBack( tBreakMeshRefPtr( new tBreakMeshRef( mesh ) ) ); break; // store the dynamic pieces
			case 1: mPermaMeshes.fPushBack( mesh ); break; // ensure "perma-pieces" (marked as state 1) are enabled, as they will default to disabled
			case 2: mUnbrokenMeshes.fPushBack( mesh ); break; // store the unbroken, cheaper representation to render until the first break
			}
		}

		for( u32 i = 0; i < root.fChildCount( ); ++i )
			fGatherGridsAndMeshes( *root.fChild( i ), grids );
	}
	void tUberBreakableLogic::fConfigureInitialMeshStates( )
	{
		if( mUnbrokenMeshes.fCount( ) > 0 )
		{
#ifdef sig_logging
			if( mUnbrokenMeshes.fCount( ) > 5 )
			{
				tSceneRefEntity * ent = fOwnerEntity( ) ? fOwnerEntity( )->fDynamicCast<tSceneRefEntity>( ) : NULL;
				tFilePathPtr path = ent ? ent->fSgResource( )->fGetPath( ) : tFilePathPtr::cNullPtr;
				log_warning_nospam( 0, "Uber breakable entity ( " << path << " ) has " << mUnbrokenMeshes.fCount( ) << " unbroken meshes" );
			}
#endif

			// enable only unbroken meshes
			for( u32 i = 0; i < mUnbrokenMeshes.fCount( ); ++i )
				mUnbrokenMeshes[ i ]->fSetDisabled( false );

			// which means disable the other kinds
			for( u32 i = 0; i < mBreakMeshes.fCount( ); ++i )
				mBreakMeshes[ i ]->mMesh->fSetDisabled( true );
			for( u32 i = 0; i < mPermaMeshes.fCount( ); ++i )
				mPermaMeshes[ i ]->fSetDisabled( true );
		}
		else
		{

#ifdef sig_logging
			if( ( mBreakMeshes.fCount( ) + mPermaMeshes.fCount( ) ) > 5 )
			{
				tSceneRefEntity * ent = fOwnerEntity( ) ? fOwnerEntity( )->fDynamicCast<tSceneRefEntity>( ) : NULL;
				tFilePathPtr path = ent ? ent->fSgResource( )->fGetPath( ) : tFilePathPtr::cNullPtr;
				log_warning_nospam( 0, "Uber breakable entity ( " << path << " ) has no unbroken meshes and " << mBreakMeshes.fCount( ) << " break meshes and " << mPermaMeshes.fCount( ) << " perma meshes" );
			}
#endif

			// no unbroken meshes, so enable everyone
			for( u32 i = 0; i < mBreakMeshes.fCount( ); ++i )
				mBreakMeshes[ i ]->mMesh->fSetDisabled( false );
			for( u32 i = 0; i < mPermaMeshes.fCount( ); ++i )
				mPermaMeshes[ i ]->fSetDisabled( false );
		}
	}
	void tUberBreakableLogic::fStepDebugRendering( f32 dt )
	{
#ifdef sig_devmenu
		if( !Debug_UberBreakable_RenderPieces )
			return;
		if( mCurrentColorizedGroup < mClumps.fCount( ) )
		{
			if( mColorizerTimer.fGetElapsedS( ) > 5.0f )
			{
				if( mClumps[ mCurrentColorizedGroup ]->fStepDebugRendering( dt ) )
					++mCurrentColorizedGroup;
				if( mCurrentColorizedGroup >= mClumps.fCount( ) )
					mCurrentColorizedGroup = 0;
				mColorizerTimer.fResetElapsedS( );
			}
			mClumps[ mCurrentColorizedGroup ]->fRenderDebug( );
		}
#endif//sig_devmenu
	}

	void tUberBreakableDamagable::fOnBreakableBroken( tUberBreakableLogic& logic ) 
	{ 
		logic.fSpawnDestroyFX( ); 
	}

	void tUberBreakableDamagable::fOnClumpBroken( tUberBreakableLogic& logic, tUberBreakableClump& clump ) 
	{ 
		logic.fSpawnDestroyFX( clump ); 
	}

	void tUberBreakableDamagable::fOnCellBroken( tUberBreakableLogic& logic, tUberBreakablePiece& piece )
	{
		logic.fSpawnDestroyFX( piece ); 
	}
}


namespace Sig
{

	void tUberBreakableLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tUberBreakableLogic, tLogic, Sqrat::NoCopy<tUberBreakableLogic> > classDesc( vm.fSq( ) );

		classDesc
			.Var(_SC("OnDestroyEffect" ),		&tUberBreakableLogic::mOnDestroyEffect)
			.Var(_SC("OnClumpDestroyEffect" ),	&tUberBreakableLogic::mOnClumpDestroyEffect)
			.Var(_SC("OnPieceDestroyEffect" ),	&tUberBreakableLogic::mOnPieceDestroyEffect)
			;

		vm.fRootTable( ).Bind(_SC("UberBreakableLogic"), classDesc);
	}
}
