#include "GameAppPch.hpp"
#include "tMiniMap.hpp"
#include "tGameApp.hpp"
#include "tLevelLogic.hpp"
#include "tUnitLogic.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateMiniMap( "CanvasCreateMiniMap" ); }
	namespace { static const tStringPtr cCanvasCreateUnitIcon( "CanvasCreateUnitIcon" ); }

	tMiniMapBase::tMiniMapBase( const tResourcePtr& scriptResource, const tUserPtr& user )
		:tScriptedControl( scriptResource )
		, mUser( user )
	{
		sigassert( mScriptResource->fLoaded( ) );
	}
	tMiniMapBase::~tMiniMapBase( )
	{
	}
	f32 tMiniMapBase::fMiniMapAngle( tUnitLogic* unit ) const
	{
		sigassert( unit );
		const Gfx::tViewportPtr& vp = mUser->fViewport( );
		const Gfx::tCamera& camera = vp->fRenderCamera( );
		Math::tVec3f objDir = unit->fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ) - camera.fGetTripod( ).mEye;
		objDir.fProjectToXZAndNormalize( );
		const f32 angle = Math::fAtan2( objDir.x, objDir.z );
		return angle;
	}

	tMiniMapUnitIcon::tMiniMapUnitIcon( const tResourcePtr& scriptResource, const tUserPtr& user )
		: tMiniMapBase( scriptResource, user )
	{
		
		fCreateControlFromScript( cCanvasCreateUnitIcon, this );
		sigassert( !mCanvas.fIsNull( ) );
	}
	tMiniMapUnitIcon::~tMiniMapUnitIcon( )
	{
	}
	void tMiniMapUnitIcon::fSetUnit( tUnitLogic* unit )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetUnit" ).Execute( unit );
	}
	

	tMiniMap::tMiniMap( const tResourcePtr& scriptResource, const tUserPtr& user )
		: tMiniMapBase( scriptResource, user )
	{
		fCreateControlFromScript( cCanvasCreateMiniMap, this );
		sigassert( !mCanvas.fIsNull( ) );
		//Sqrat::Function( mCanvas.fScriptObject( ), "Position" ).Execute( );
	}
	void tMiniMap::fInitialize( tPlayer* player )
	{
		fAddToyBoxes( player );
		fAddDrivableUnits( player );
	}
	void tMiniMap::fFadeIn( )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "FadeIn" ).Execute( );
	}
	void tMiniMap::fFadeOut( )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "FadeOut" ).Execute( );
	}
	void tMiniMap::fUpdate( )
	{
		const Gfx::tViewportPtr& vp = mUser->fViewport( );
		const Gfx::tCamera& camera = vp->fRenderCamera( );
		Math::tVec3f view = camera.fZAxis( );
		view.y = 0.f;
		view = view.fNormalizeSafe( Math::tVec3f::cZAxis );
		const f32 angle = Math::fAtan2( view.x, view.z );
		Sqrat::Function( mCanvas.fScriptObject( ), "RotateMap" ).Execute( angle );

		//Math::tVec3f cameraPos = camera.fGetTripod( ).mLookAt;
		//cameraPos = fComputeLevelNormalizedPosition( cameraPos );
		//Sqrat::Function( mCanvas.fScriptObject( ), "CameraPos" ).Execute( cameraPos );
	}

	void tMiniMap::fAddToyBoxes( tPlayer* player ) const
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		sigassert( level );

		if( !player )
			return;

		for( u32 i = 0; i < level->fGoalBoxCount( ); ++i )
		{
			tUnitLogic* unit = level->fGoalBox( i )->fLogicDerived< tUnitLogic >( );
			if( !unit )
				continue;
			if( unit->fCountry( ) != player->fCountry( ) ) // only add friendly toyboxes
				continue;

			Gui::tMiniMapUnitIcon* icon = NEW Gui::tMiniMapUnitIcon( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptMiniMap ), player->fUser( ) );
			icon->fSetUnit( unit );
			unit->fAddCanvasObject( Gui::tScriptedControlPtr( icon ) );

			Sqrat::Function( mCanvas.fScriptObject( ), "AddToyBox" ).Execute( icon->fCanvas( ).fScriptObject( ) );
		}
	}

	void tMiniMap::fAddDrivableUnits( tPlayer* player ) const
	{
		tLevelLogic* level = tGameApp::fInstance( ).fCurrentLevel( );
		sigassert( level );

		if( !player )
			return;

		for( u32 i = 0; i < level->fUseableUnitCount( ); ++i )
		{
			tUnitLogic* unit = level->fUseableUnit( i )->fLogicDerived< tUnitLogic >( );
			if( !unit )
				continue;
			if( unit->fCountry( ) != player->fCountry( ) ) // only add friendly drivable units
				continue;

			Gui::tMiniMapUnitIcon* icon = NEW Gui::tMiniMapUnitIcon( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptMiniMap ), player->fUser( ) );
			icon->fSetUnit( unit );
			unit->fAddCanvasObject( Gui::tScriptedControlPtr( icon ) );

			Sqrat::Function( mCanvas.fScriptObject( ), "AddDrivable" ).Execute( icon->fCanvas( ).fScriptObject( ) );
		}
	}

	void tMiniMap::fAddUnit( const Sqrat::Object& unit ) const
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "AddUnit" ).Execute( unit );
	}
}}


namespace Sig { namespace Gui
{
	void tMiniMapBase::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tMiniMapBase,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("User"), &fUser)
			.Func(_SC("MiniMapAngle"), &tMiniMapBase::fMiniMapAngle)
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("MiniMapBase"), classDesc );

		tMiniMap::fExportScriptInterface( vm );
		tMiniMapUnitIcon::fExportScriptInterface( vm );
	}

	void tMiniMapUnitIcon::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tMiniMapUnitIcon, tMiniMapBase, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		vm.fNamespace(_SC("Gui")).Bind( _SC("MiniMapUnitIcon"), classDesc );
	}

	void tMiniMap::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tMiniMap, tMiniMapBase, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		vm.fNamespace(_SC("Gui")).Bind( _SC("MiniMap"), classDesc );
	}
}}

