#include "BasePch.hpp"
#include "tLogicPtr.hpp"

// for debug
#include "tSceneRefEntity.hpp"
#include "Logic/tAnimatable.hpp"
#include "Logic/tGoalDriven.hpp"

namespace Sig
{
	const char* tLogicPtr::fDebugTypeName( ) const
	{
		if( fIsCodeOwned( ) )
			return fCodeObject( )->fDebugTypeName( );
		else
		{
			Sqrat::Function f( fScriptObject( ), "DebugTypeName" );
			sigassert( !f.IsNull( ) );
			return f.Evaluate<const char*>( );
		}
	}
	void tLogicPtr::fOnSpawn( )
	{
		if( fIsCodeOwned( ) )
			fCodeObject( )->fOnSpawn( );
		else
		{
			Sqrat::Function f( fScriptObject( ), "OnSpawn" );
			sigassert( !f.IsNull( ) );
			f.Execute( );
		}
	}
	void tLogicPtr::fOnDelete( )
	{
		if( fIsCodeOwned( ) )
			fCodeObject( )->fOnDelete( );
		else
		{
			Sqrat::Function f( fScriptObject( ), "OnDelete" );
			sigassert( !f.IsNull( ) );
			f.Execute( );
		}
	}
	void tLogicPtr::fComputeDebugText( std::string& textOut ) const
	{
#ifdef sig_devmenu
		tLogic* logic = fCodeObject( );
		sigassert( logic && logic->fOwnerEntity( ) );

		std::stringstream ss;
		ss << fDebugTypeName( ) << std::endl;

		if( logic->fOwnerEntity( )->fName( ).fExists( ) )
			ss << "Name: " << logic->fOwnerEntity( )->fNameCStr( ) << std::endl;

		tSceneRefEntity* sceneRef = logic->fOwnerEntity( )->fDynamicCast< tSceneRefEntity >( );
		if( sceneRef )
			ss << "SceneRef: " << sceneRef->fSgResource( )->fGetPath( ).fCStr( ) << std::endl;

		Logic::tAnimatable* animatable = logic->fQueryAnimatable( );
		if( animatable )
			animatable->fAddWorldDebugText( ss );

		Logic::tGoalDriven* goalDriven = logic->fQueryGoalDriven( );
		if( goalDriven )
			goalDriven->fAddWorldDebugText( ss );

		logic->fAddWorldDebugText( ss );

		textOut = ss.str( );
#endif//sig_devmenu
	}
}

