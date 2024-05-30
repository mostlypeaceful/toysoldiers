#include "BasePch.hpp"
#include "tAnimTrackInputs.hpp"

namespace Sig { namespace Anim
{
	namespace
	{
	}

	void tTrackInput::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tTrackInput, Sqrat::DefaultAllocator<tTrackInput> > classDesc( vm.fSq( ) );
		classDesc
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("TrackInput"), classDesc );
	}
	
} }


