#include "GameAppPch.hpp"
#include "tFocalPrompt.hpp"

namespace Sig { namespace Gui 
{

	namespace { static const tStringPtr cCanvasCreateFocalPrompt( "CanvasCreateFocalPrompt" ); }

	tFocalPrompt::tFocalPrompt( const tResourcePtr& scriptResource, tUser& user )
		: tScriptedControl( scriptResource )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateFocalPrompt, this );
		log_assert( !mCanvas.fIsNull( ), "Canvas couldn't be created from script: " << scriptResource->fGetPath( ) );
		
		Sqrat::Function( mCanvas.fScriptObject( ), "Setup" ).Execute( &user );
	}

	void tFocalPrompt::fShow( const tStringPtr& locID )
	{
		return Sqrat::Function( mCanvas.fScriptObject( ), "Show" ).Execute( locID );
	}

	void tFocalPrompt::fHide( b32 hide )
	{
		return Sqrat::Function( mCanvas.fScriptObject( ), "Hide" ).Execute( hide );
	}

}}

namespace Sig { namespace Gui 
{

	void tFocalPrompt::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tFocalPrompt, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			;
		vm.fNamespace(_SC("Gui")).Bind( _SC("FocalPrompt"), classDesc );
	}

}}