#include "BasePch.hpp"
#include "tPhysical.hpp"

namespace Sig { namespace Logic
{
	void tPhysical::fExportScriptInterface( tScriptVm& vm )
	{
#define logic_event_script_export
#	include "PhysicalEvents.hpp"
#undef logic_event_script_export

		Sqrat::Class<tPhysical, Sqrat::NoConstructor > classDesc( vm.fSq( ) );

		vm.fRootTable( ).Bind(_SC("Physical"), classDesc);
	}
}}

namespace Sig { namespace Physics
{
	tStandardPhysics::tStandardPhysics( )
		: mInternalFlags( 0 )
		, mGroundMask( 0 )
		, mV( Math::tVec3f::cZeroVector )
	{
	}
	Math::tMat3f tStandardPhysics::fApplyRefFrameDelta( const Math::tMat3f& baseRefFrame, const Math::tPRSXformf& refFrameDelta, f32 scale )
	{
		Math::tMat3f result = baseRefFrame;
		refFrameDelta.fApplyAsRefFrameDelta( result, scale );
		sigassert( !result.fIsNan( ) );
		return result;
	}
	void tStandardPhysics::fSetTransform( const Math::tMat3f& moveTo )
	{
		mTransform = moveTo;
		mInternalFlags = fSetBits( mInternalFlags, cFlagMoveToDirty );
	}

	namespace 
	{
		static tStandardPhysics* fConvert( const Logic::tPhysical* obj )
		{
			return obj->fDynamicCast< tStandardPhysics >( );
		}
	}

	void tStandardPhysics::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass< tStandardPhysics, Logic::tPhysical, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		classDesc
			.Func( _SC( "SetFlag" ), &tStandardPhysics::fSetFlag )
			.Func( _SC( "ClearFlag" ), &tStandardPhysics::fClearFlag )
			.Func( _SC( "TestFlag" ), &tStandardPhysics::fTestFlag )
			.Func( _SC( "Disable" ), &tStandardPhysics::fDisabled )
			.StaticFunc( _SC("Convert"), &fConvert);
			;

		vm.fNamespace(_SC("Physics")).Bind(_SC("Standard"), classDesc);

		vm.fConstTable( ).Const(_SC("STANDARD_PHYSICS_DISABLE"), ( int ) cFlagDisable );
		vm.fConstTable( ).Const(_SC("STANDARD_PHYSICS_FALLING"), ( int ) cFlagFalling );
	}
}}