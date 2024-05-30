#include "GameAppPch.hpp"
#include "tPowerUpLogic.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tGameSessionStats.hpp"
#include "tSceneGraphCollectTris.hpp"

namespace Sig
{
	namespace
	{
		struct tPowerUpRayCastCallback
		{
			mutable Math::tRayCastHit		mHit;
			mutable tEntity*				mHitEntity;

			explicit tPowerUpRayCastCallback( ) 
				: mHitEntity( 0 )
			{
			}

			inline void fRayCastAgainstSpatial( const Math::tRayf& ray, tSpatialEntity* spatial ) const
			{
				if( spatial->fToSpatialSetObject( )->fQuickRejectByBox( ray ) )
					return;

				Math::tRayCastHit hit;
				spatial->fRayCast( ray, hit );
				if( !hit.fHit( ) )
					return;

				if( hit.mT < mHit.mT )
				{
					mHit = hit;
					mHitEntity = spatial;
				}					
			}
			
			inline void operator( )( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const
			{
				if( i->fQuickRejectByFlags( ) )
					return;

				tSpatialEntity* spatial = static_cast<tSpatialEntity*>( i );
				if( spatial->fHasGameTagsAny( GameFlags::cFLAG_DUMMY ) )
					return;
				if( !spatial->fHasGameTagsAny( GameFlags::cFLAG_GROUND ) )
					return;
				
				fRayCastAgainstSpatial( ray, spatial );
			}
		};
	}

	tPowerUpLogic::tPowerUpLogic( ) : mNextPos( Math::tVec3f::cZeroVector )
									, mLaunchVector( Math::tVec3f::cZeroVector )
									, mFalling( true )
									, mOutsideLevel( false )
									, mSpin( 0.f )
									, mBounce( Math::c3PiOver2 )
									, mRestHeight( 0.f )
									, mPowerUpValue( 0 )
	{
	}

	tPowerUpLogic::~tPowerUpLogic( )
	{
	}
	void tPowerUpLogic::fSetTeam( GameFlags::tTEAM team )
	{
		tUnitLogic::fSetTeam( team );
	}
	void tPowerUpLogic::fOnSpawn( )
	{
		sigassert( !"tPowerUpLogic is no longer planned for use in the ts2." );
		fOnPause( false );

		tUnitLogic::fOnSpawn( );

		fCollectSelectionShapes( );

		mNextPos = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
	}
	void tPowerUpLogic::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListActST );
			fRunListRemove( cRunListMoveST );
			fRunListRemove( cRunListCoRenderMT );
		}
		else
		{
			fRunListInsert( cRunListActST );
			fRunListInsert( cRunListMoveST );
			fRunListInsert( cRunListCoRenderMT );
		}
	}
	Gui::tRadialMenuPtr tPowerUpLogic::fCreateSelectionRadialMenu( tPlayer& player )
	{
		Gui::tRadialMenuPtr radialMenu = Gui::tRadialMenuPtr( NEW Gui::tRadialMenu( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptPowerUpOptions ), player.fUser( ), player.fGameController( ) ) );

		Sqrat::Object params( Sqrat::Table( ).SetValue( "Unit", this ).SetValue( "Player", &player ) );
		Sqrat::Function( radialMenu->fCanvas( ).fScriptObject( ), "DefaultSetup" ).Execute( params );

		return radialMenu;
	}
	b32 tPowerUpLogic::fTryToUse( )
	{
		// TODO: Actually give some kind of buff to the player
		mAlternateDebrisMesh = tFilePathPtr( "Art/_Placeholder/Gameplay/australian_debris.sigml" );
		fExplodeIntoParts( );
		return true;
	}
	namespace { static const tStringPtr cPowerUpName("PowerUp1"); }
	tLocalizedString tPowerUpLogic::fHoverText( ) const
	{
		return tGameApp::fInstance( ).fLocString( cPowerUpName );
	}
	void tPowerUpLogic::fActST( f32 dt )
	{
		tUnitLogic::fActST( dt );

		if( mOutsideLevel )
		{
			fOwnerEntity( )->fDelete( );
			return;
		}
	}

	void tPowerUpLogic::fMoveST( f32 dt )
	{
		mSpin += dt;
		Math::tVec3f zAxis = Math::tVec3f::cZAxis;
		const Math::tQuatf rot( Math::tAxisAnglef( Math::tVec3f::cYAxis, mSpin * Math::cPi ) );
		zAxis = rot.fRotate( zAxis );
		Math::tMat3f xform = fOwnerEntity( )->fObjectToWorld( );
		xform.fOrientZAxis( zAxis );

		if( !mFalling )
		{
			mNextPos.y = mRestHeight + ( sin( mBounce * 4.f ) + 1.f ) * 0.5f;
			mBounce += dt;
		}

		xform.fSetTranslation( mNextPos );
		fOwnerEntity( )->fMoveTo( xform );
	}

	void tPowerUpLogic::fCoRenderMT( f32 dt )
	{
		if( !mFalling ) return;
		
		fComputeNewPosition( dt );
		if( fCheckLevelBounds( ) )
			fRayCast( );
		else
			mOutsideLevel = true;
	}

	void tPowerUpLogic::fComputeNewPosition( f32 dt )
	{
		const Math::tVec3f gravity( 0.f, -9.81f, 0.f );
		mLaunchVector += gravity * dt;
		mNextPos += mLaunchVector * dt;

		fCheckLevelBounds( );
	}

	b32 tPowerUpLogic::fCheckLevelBounds( ) const
	{
		return tGameApp::fInstance( ).fCurrentLevel( )->fLevelBounds( ).fContainsXZ( mNextPos );
	}

	void tPowerUpLogic::fRayCast( )
	{
		const Math::tVec3f currPos = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( );
		const Math::tRayf ray( currPos, mNextPos - currPos );
		tPowerUpRayCastCallback rayCastcb;

		fOwnerEntity( )->fSceneGraph( )->fRayCastAgainstRenderable( ray, rayCastcb );

		if( rayCastcb.mHit.fHit( ) )
		{
			mNextPos = ray.fEvaluate( rayCastcb.mHit.mT );
			mFalling = false;
			mRestHeight = mNextPos.y;
		}
	}

	void tPowerUpLogic::fReactToDamage( const tDamageContext& dc, const tDamageResult& dr )
	{ 
		if( dr.mDestroysYou )
		{

			//if( fTeamPlayer( ) )
			//	fTeamPlayer( )->fStats( ).fAddToPowerPool( (f32)mPowerUpValue );
			//else
			//	log_warning( 0, "No team player for powerup!" );

			fOwnerEntity( )->fDelete( );
		}

		tUnitLogic::fReactToDamage( dc, dr );
	}

}

namespace Sig
{
	void tPowerUpLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tPowerUpLogic, tLogic, Sqrat::NoCopy<tPowerUpLogic> > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("TryToUse"),			&tPowerUpLogic::fTryToUse)
			;
		vm.fRootTable( ).Bind(_SC("PowerUpLogic"), classDesc);
	}
}