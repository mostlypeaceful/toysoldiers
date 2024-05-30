#include "GameAppPch.hpp"
#include "tPointCaptureUI.hpp"

namespace Sig { namespace Gui
{
	namespace { static const tStringPtr cCanvasCreatePointCaptureUI( "CanvasCreatePointCaptureUI" ); }

	tPointCaptureUI::tPointCaptureUI( const tResourcePtr& scriptResource, const tUserPtr& user )
		: tScriptedControl( scriptResource )
		, mUser( user )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreatePointCaptureUI, this );
		sigassert( !mCanvas.fIsNull( ) );
	}

	void tPointCaptureUI::fShow( b32 show )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Show" ).Execute( show );
	}

	u32 tPointCaptureUI::fCurrentOwner()
	{
		return Sqrat::Function( mCanvas.fScriptObject( ), "CurrentOwner" ).Evaluate< u32 >( );
	}

	void tPointCaptureUI::fSetPercent( u32 team, f32 percent )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetPercent" ).Execute( team, percent );
	}

	void tPointCaptureUI::fCapture( u32 team )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Capture" ).Execute( team );
	}

	void tPointCaptureUI::fLost( u32 toTeam )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Lost" ).Execute( toTeam );
	}

	void tPointCaptureUI::fDisputed( b32 isDisputed )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Disputed" ).Execute( isDisputed );
	}

} }

namespace Sig { namespace Gui
{
	void tPointCaptureUI::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tPointCaptureUI,Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("User"), &tPointCaptureUI::fUser)
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("PointCaptureUI"), classDesc );
	}
} }