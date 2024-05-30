#include "BasePch.hpp"
#include "tMotionMap.hpp"

namespace Sig
{
	Sqrat::Function tScriptMotionMap::fMapState( const char* stateName )
	{ 
		log_assert( !fScriptObject( ).IsNull( ), "No MotionMap when looking for MotionState: " << stateName );
		Sqrat::Function f = Sqrat::Function( fScriptObject( ), stateName );
		if( f.IsNull( ) )
			log_warning( Log::cFlagAnimation, "The motion state [" << stateName << "] is not present in the MoMap script." );
		return f;
	}
}


#include "tApplication.hpp"
#include "tAnimPackFile.hpp"
namespace Sig
{
	namespace
	{
		static tAnimPackFile* fGetAnimPack( const tMotionMap* momap, const tFilePathPtr& path )
		{
			const tResourcePtr res = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake<tAnimPackFile>( tAnimPackFile::fAnipkPathToAnib( path ) ) );
			if( res->fLoaded( ) )
				return res->fCast<tAnimPackFile>( );
			else
			{
				log_warning( Log::cFlagAnimation, "AnimPack does not exist: " << path );
				return NULL;
			}
		}
	}

	void tMotionMap::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tMotionMap, Sqrat::NoCopy<tMotionMap> > classDesc( vm.fSq( ) );

		classDesc
			.GlobalFunc(_SC("GetAnimPack"),		&fGetAnimPack)
			.Prop(_SC("Stack"),					&tMotionMap::fAnimationStack)
			.Prop(_SC("Logic"),					&tMotionMap::fGetLogicForScript)
			;

		vm.fNamespace(_SC("Anim")).Bind(_SC("MotionMap"), classDesc);
	}
}

