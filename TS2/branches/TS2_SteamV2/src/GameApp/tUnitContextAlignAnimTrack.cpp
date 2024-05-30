#include "GameAppPch.hpp"
#include "tUnitContextAlignAnimTrack.hpp"
#include "tAnimatedSkeleton.hpp"

namespace Sig
{

	tUnitContextAlignAnim::tUnitContextAlignAnim( const tUnitContextAlignDesc& desc )
		: tUnitPathAnimTrack( tUnitPathAnimDesc( desc.mBlendIn, desc.mBlendOut, desc.mTimeScale, desc.mBlendScale, desc.mUnitPath, desc.mRotateSpeed, true ) )
	{
	}

	const Math::tVec3f tUnitContextAlignAnim::fGetTarget( )
	{
		return mUnitPath->fContextTarget( );
	}

}


namespace Sig
{
	namespace
	{
		static void fPushAnim( const tUnitContextAlignDesc* desc, tAnimatedSkeleton* stack )
		{
			sigassert( desc );
			stack->fRemoveTracksOfType<tOrientAnimTrack>( );
			stack->fPushTrack( tAnimTrackPtr( NEW tUnitContextAlignAnim( *desc ) ) );
		}
	}
	void tUnitContextAlignDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tUnitContextAlignDesc,Sqrat::DefaultAllocator<tUnitContextAlignDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("BlendIn"), &tUnitContextAlignDesc::mBlendIn)
			.Var(_SC("BlendOut"), &tUnitContextAlignDesc::mBlendOut)
			.Var(_SC("TimeScale"), &tUnitContextAlignDesc::mTimeScale)
			.Var(_SC("BlendScale"), &tUnitContextAlignDesc::mBlendScale)
			.Var(_SC("UnitPath"), &tUnitContextAlignDesc::mUnitPath)
			.Var(_SC("RotateSpeed"), &tUnitContextAlignDesc::mRotateSpeed)
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("UnitContextAlignTrack"), classDesc );
	}
}
