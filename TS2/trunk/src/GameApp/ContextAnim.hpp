#ifndef __ContextAnim__
#define __ContextAnim__
#include "tLogic.hpp"

namespace Sig
{

	class tContextAnimEventContext : public Logic::tEventContext
	{
		define_dynamic_cast( tContextAnimEventContext, Logic::tEventContext );
	public:
		tStringPtr	mAnimBegin;
		tStringPtr	mAnimLoop;
		tStringPtr	mAnimEnd;
		tStringPtr	mAnimMoveTo;
		tStringPtr	mAnimAlignTo;
		b32			mDisablePhysics;
		b32			mJumping; //expect jump event, or call jump after begin anim, soldier will leave box, which should not cancel context anim
		b32			mPathing;
		b32			mTeleport;
		f32			mTeleportSpeed;
		f32			mJumpVel;
		f32			mHeight; //distance to top of context box

		tContextAnimEventContext( )
			: mAnimBegin( "" )
			, mAnimLoop( "" )
			, mAnimEnd( "" )
			, mAnimMoveTo( "" )
			, mAnimAlignTo( "" )
			, mDisablePhysics( false )
			, mJumping( false )
			, mPathing( false )
			, mTeleport( false )
			, mTeleportSpeed( 1.f )
			, mJumpVel( 1.f )
			, mHeight( 0.f )
		{ }


		static tContextAnimEventContext fConvert( const Logic::tEventContext* obj )
		{
			tContextAnimEventContext* context = obj->fDynamicCast< tContextAnimEventContext >( );
			if( context ) return *context;
			else return tContextAnimEventContext( );
		}

		static void fExportScriptInterface( tScriptVm& vm )
		{
			Sqrat::DerivedClass<tContextAnimEventContext, Logic::tEventContext, Sqrat::DefaultAllocator<tContextAnimEventContext> > classDesc( vm.fSq( ) );
			classDesc
				.Var(_SC("AnimBegin"), &tContextAnimEventContext::mAnimBegin)
				.Var(_SC("AnimLoop"), &tContextAnimEventContext::mAnimLoop)
				.Var(_SC("AnimEnd"), &tContextAnimEventContext::mAnimEnd)
				.Var(_SC("AnimMoveTo"), &tContextAnimEventContext::mAnimMoveTo)
				.Var(_SC("AnimAlignTo"), &tContextAnimEventContext::mAnimAlignTo)
				.Var(_SC("DisablePhysics"), &tContextAnimEventContext::mDisablePhysics)
				.Var(_SC("Jumping"), &tContextAnimEventContext::mJumping)
				.Var(_SC("Pathing"), &tContextAnimEventContext::mPathing)
				.Var(_SC("Teleport"), &tContextAnimEventContext::mTeleport)
				.Var(_SC("JumpVel"), &tContextAnimEventContext::mJumpVel)
				.Var(_SC("Height"), &tContextAnimEventContext::mHeight)
				.StaticFunc(_SC("Convert"), &tContextAnimEventContext::fConvert)
				;

			vm.fRootTable( ).Bind(_SC("ContextAnimEvent"), classDesc);
		}
	};


}

#endif//__ContextAnim__
