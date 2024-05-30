#include "BasePch.hpp"
#include "tLogicPtr.hpp"

// for debug
#include "tSceneRefEntity.hpp"
#include "Logic/tAnimatable.hpp"
#include "Logic/tGoalDriven.hpp"

namespace Sig
{
	tLogicPtr::tLogicPtr( const Sqrat::Object& o )
		: tScriptOrCodeObjectPtr<tLogic>( o )
	{
#ifdef sig_assert
		//if we don't have an OnSpawn or OnDelete then we are some "Logic" that doesn't derive (aka 'extends') from tLogic which is VERY BAD NEWS
		Sqrat::Function fs( o, "OnSpawn" );
		if( fs.IsNull( ) )
		{
			tScriptVm::fDumpCallstack( );
			log_assert( !fs.IsNull( ), "NO 'OnSpawn'!!! This logic does not 'extend' Logic. DO NOT DO THIS. We can't have logics assigned to entities that do not derive from tLogic!" );
		}
		Sqrat::Function fd( o, "OnDelete" );
		if( fd.IsNull( ) )
		{
			tScriptVm::fDumpCallstack( );
			log_assert( !fd.IsNull( ), "NO 'OnDelete'!!! This logic does not 'extend' Logic. DO NOT DO THIS. We can't have logics assigned to entities that do not derive from tLogic!" );
		}
#endif//sig_assert
	}
	const char* tLogicPtr::fDebugTypeName( ) const
	{
		if( fIsCodeOwned( ) )
			return fCodeObject( )->fDebugTypeName( );
		else
		{
			Sqrat::Function f( fScriptObject( ), "DebugTypeName" );
			sigcheckfail( !f.IsNull( ), return "(unknown)" );
			return f.Evaluate<const char*>( );
		}
	}
	void tLogicPtr::fOnSpawn( )
	{
		if( fCodeObject( ) )
			fCodeObject( )->fSetupComponents( );

		if( fIsCodeOwned( ) )
			fCodeObject( )->fOnSpawn( );
		else
		{
			Sqrat::Function f( fScriptObject( ), "OnSpawn" );
			sigcheckfail( !f.IsNull( ), return );
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
			sigcheckfail( !f.IsNull( ), return );
			f.Execute( );
		}
	}
}

