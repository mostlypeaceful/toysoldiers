#ifndef __tPhysical__
#define __tPhysical__

namespace Sig { namespace Logic { namespace PhysicalEvent
{
#include "PhysicalEvents.hpp"
}}}

namespace Sig { namespace Logic
{
	class tPhysical
	{
		define_dynamic_cast_base( tPhysical );
	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	public:
		virtual ~tPhysical( ) { }
	};
}}

namespace Sig
{
	template<class tDerived>
	inline tDerived* tLogic::fQueryPhysicalDerived( )
	{
		Logic::tPhysical* comp = fQueryPhysical( );
		return comp ? comp->fDynamicCast< tDerived >( ) : 0;
	}
}


namespace Sig { namespace Physics
{
	class tStandardPhysics : public Logic::tPhysical
	{
		define_dynamic_cast( tStandardPhysics, Logic::tPhysical );
	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	protected:
		enum tInternalFlags
		{
			cFlagMoveToDirty	= 1 << 0,
			cFlagStartedFalling	= 1 << 1,
			cFlagFalling		= 1 << 2,
			cFlagJustLanded		= 1 << 3,
			cFlagWantsJump		= 1 << 4,
			cFlagDisable		= 1 << 5, //when the object becomes kinematic
		};
		inline void				fSetFlag( u32 flag ) { mInternalFlags = fSetBits( mInternalFlags, flag ); }
		inline void				fClearFlag( u32 flag ) { mInternalFlags = fClearBits( mInternalFlags, flag ); }
		inline b32				fTestFlag( u32 flag ) { return fTestBits( mInternalFlags, flag ); }
	public:
		tStandardPhysics( );
		virtual ~tStandardPhysics( ) { }

		static Math::tMat3f		fApplyRefFrameDelta( const Math::tMat3f& baseRefFrame, const Math::tPRSXformf& refFrameDelta, f32 scale = 1.f );
		void					fSetTransform( const Math::tMat3f& moveTo ); // call before fPhysicsMT and fCoRenderMT
		const Math::tMat3f&		fTransform( ) const { return mTransform; } // call after fPhysicsMT
		Math::tVec3f			fPosition( ) const { return mTransform.fGetTranslation( ); }

	public: // query current state
		inline b32				fStartedFalling( ) const { return fTestBits( mInternalFlags, cFlagStartedFalling ); }
		inline b32				fFalling( ) const { return fTestBits( mInternalFlags, cFlagFalling ); }
		inline b32				fJustLanded( ) const { return fTestBits( mInternalFlags, cFlagJustLanded ); }
		inline b32				fDisabled( ) const { return fTestBits( mInternalFlags, cFlagDisable ); }
		b32						fWantsJump( ) const { return fTestBits( mInternalFlags, cFlagWantsJump ); }

	public: // configure
		inline void				fDisable( b32 disable ) { if( disable ) fSetFlag( cFlagDisable ); else fClearFlag( cFlagDisable ); }

		void					fSetGroundMask( tEntityTagMask groundMask ) { mGroundMask = groundMask; }
		tEntityTagMask			fGroundMask( ) const { return mGroundMask; }

		void					fSetVelocity( const Math::tVec3f& v ) { sigassert( !v.fIsNan( ) ); mV = v; }
		const Math::tVec3f&		fVelocity( ) const { return mV; }
		virtual Math::tVec3f	fPointVelocity( const Math::tVec3f& worldPoint ) const { return mV; }

	protected:
		u32				mInternalFlags;
		tEntityTagMask	mGroundMask;
		Math::tVec3f	mV;
		Math::tMat3f	mTransform;
	};
}}

#endif//__tPhysical__
