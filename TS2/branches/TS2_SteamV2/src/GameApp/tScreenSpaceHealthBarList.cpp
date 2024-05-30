#include "GameAppPch.hpp"
#include "tScreenSpaceHealthBarList.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreateScreenSpaceHealthBarList( "CanvasCreateScreenSpaceHealthBarList" ); }

	tScreenSpaceHealthBarList::tScreenSpaceHealthBarList( const tResourcePtr& scriptResource, const tUserPtr& user )
		: tScriptedControl( scriptResource )
		, mUser( user )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateScreenSpaceHealthBarList, this );
		sigassert( !mCanvas.fIsNull( ) );
	}
	tScreenSpaceHealthBarList::~tScreenSpaceHealthBarList( )
	{
	}
	void tScreenSpaceHealthBarList::fAddHealthBar( const tUnitLogic& unitLogic )
	{
		mUnits.fPushBack( tEntityPtr( unitLogic.fOwnerEntity( ) ) );
		std::string locKey = unitLogic.fLocKey( );
		tFilePathPtr imgPath = tGameApp::fInstance( ).fUnitWaveIconPath( unitLogic.fUnitID( ), unitLogic.fCountry( ) );
		Sqrat::Function( mCanvas.fScriptObject( ), "AddHealthBar" ).Execute( tGameApp::fInstance( ).fLocString( tStringPtr( locKey ) ), imgPath );
	}
	void tScreenSpaceHealthBarList::fSetColor( const tUnitLogic& unitLogic, const Math::tVec4f& color )
	{
		u32 index = mUnits.fIndexOf( unitLogic.fOwnerEntity( ) );
		if( index != ~0 )
			Sqrat::Function( mCanvas.fScriptObject( ), "SetColor" ).Execute( index, color );
	}
	void tScreenSpaceHealthBarList::fSetFlashAndFill( const tUnitLogic& unitLogic, f32 flash, f32 fill )
	{
		u32 index = mUnits.fIndexOf( unitLogic.fOwnerEntity( ) );
		if( index != ~0 )
			Sqrat::Function( mCanvas.fScriptObject( ), "SetFlashAndFill" ).Execute( index, flash, fill );
	}
	void tScreenSpaceHealthBarList::fSetHealthPercent( const tUnitLogic& unitLogic, f32 percent )
	{
		u32 index = mUnits.fIndexOf( unitLogic.fOwnerEntity( ) );
		if( index != ~0 )
			Sqrat::Function( mCanvas.fScriptObject( ), "SetHealthBarPercent" ).Execute( index, percent );
	}
}}


namespace Sig { namespace Gui
{
	void tScreenSpaceHealthBarList::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tScreenSpaceHealthBarList,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		//classDesc
		//	;
		vm.fNamespace(_SC("Gui")).Bind( _SC("ScreenSpaceHealthBarList"), classDesc );
	}
}}

